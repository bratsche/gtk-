/* GTK - The GIMP Toolkit
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* TODO
 * - move out custom sliders
 * - pop-up entries
 */

#include "config.h"

#include "gtkcolorchooserprivate.h"
#include "gtkcoloreditor.h"
#include "gtkcolorplane.h"
#include "gtkcolorswatch.h"
#include "gtkgrid.h"
#include "gtkscale.h"
#include "gtkaspectframe.h"
#include "gtkdrawingarea.h"
#include "gtkentry.h"
#include "gtkhsv.h"
#include "gtkadjustment.h"
#include "gtkintl.h"

#include <math.h>

struct _GtkColorEditorPrivate
{
  GtkWidget *grid;
  GtkWidget *swatch;
  GtkWidget *entry;
  GtkWidget *h_slider;
  GtkWidget *sv_plane;
  GtkWidget *a_slider;

  GtkAdjustment *h_adj;
  GtkAdjustment *a_adj;
  cairo_surface_t *h_surface;
  cairo_surface_t *a_surface;

  GdkRGBA color;
  gdouble h, s, v;
  guint text_changed : 1;
  guint show_alpha   : 1;
};

enum
{
  PROP_ZERO,
  PROP_COLOR,
  PROP_SHOW_ALPHA
};

static void gtk_color_editor_iface_init (GtkColorChooserInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtkColorEditor, gtk_color_editor, GTK_TYPE_BOX,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_COLOR_CHOOSER,
                                                gtk_color_editor_iface_init))

static guint
scale_round (gdouble value, gdouble scale)
{
  value = floor (value * scale + 0.5);
  value = MAX (value, 0);
  value = MIN (value, scale);
  return (guint)value;
}

static void
update_entry (GtkColorEditor *editor)
{
  gchar *text;

  text = g_strdup_printf ("#%02X%02X%02X",
                          scale_round (editor->priv->color.red, 255),
                          scale_round (editor->priv->color.green, 255),
                          scale_round (editor->priv->color.blue, 255));
  gtk_entry_set_text (GTK_ENTRY (editor->priv->entry), text);
  editor->priv->text_changed = FALSE;
  g_free (text);
}

static void
entry_apply (GtkWidget      *entry,
             GtkColorEditor *editor)
{
  GdkRGBA color;
  gchar *text;

  if (!editor->priv->text_changed)
    return;

  text = gtk_editable_get_chars (GTK_EDITABLE (editor->priv->entry), 0, -1);
  if (gdk_rgba_parse (&color, text))
    {
      color.alpha = editor->priv->color.alpha;
      gtk_color_chooser_set_color (GTK_COLOR_CHOOSER (editor), &color);
    }

  editor->priv->text_changed = FALSE;

  g_free (text);
}

static gboolean
entry_focus_out (GtkWidget      *entry,
                 GdkEventFocus  *event,
                 GtkColorEditor *editor)
{
  entry_apply (entry, editor);
  return FALSE;
}

static void
entry_text_changed (GtkWidget      *entry,
                    GParamSpec     *pspec,
                    GtkColorEditor *editor)
{
  editor->priv->text_changed = TRUE;
}

static void
h_changed (GtkAdjustment  *adj,
           GtkColorEditor *editor)
{
  editor->priv->h = gtk_adjustment_get_value (adj);
  gtk_hsv_to_rgb (editor->priv->h, editor->priv->s, editor->priv->v,
                  &editor->priv->color.red,
                  &editor->priv->color.green,
                  &editor->priv->color.blue);
  gtk_color_plane_set_h (GTK_COLOR_PLANE (editor->priv->sv_plane), editor->priv->h);
  update_entry (editor);
  gtk_color_swatch_set_color (GTK_COLOR_SWATCH (editor->priv->swatch), &editor->priv->color);
  gtk_widget_queue_draw (editor->priv->a_slider);
  if (editor->priv->a_surface)
    {
      cairo_surface_destroy (editor->priv->a_surface);
      editor->priv->a_surface = NULL;
    }
  g_object_notify (G_OBJECT (editor), "color");
}

static void
sv_changed (GtkColorPlane  *plane,
            GtkColorEditor *editor)
{
  editor->priv->s = gtk_color_plane_get_s (plane);
  editor->priv->v = gtk_color_plane_get_v (plane);
  gtk_hsv_to_rgb (editor->priv->h, editor->priv->s, editor->priv->v,
                  &editor->priv->color.red,
                  &editor->priv->color.green,
                  &editor->priv->color.blue);
  update_entry (editor);
  gtk_color_swatch_set_color (GTK_COLOR_SWATCH (editor->priv->swatch), &editor->priv->color);
  gtk_widget_queue_draw (editor->priv->a_slider);
  if (editor->priv->a_surface)
    {
      cairo_surface_destroy (editor->priv->a_surface);
      editor->priv->a_surface = NULL;
    }
  g_object_notify (G_OBJECT (editor), "color");
}

static void
a_changed (GtkAdjustment  *adj,
           GtkColorEditor *editor)
{
  editor->priv->color.alpha = gtk_adjustment_get_value (adj);
  gtk_color_swatch_set_color (GTK_COLOR_SWATCH (editor->priv->swatch), &editor->priv->color);
  g_object_notify (G_OBJECT (editor), "color");
}

static cairo_pattern_t *
get_checkered_pattern (void)
{
  /* need to respect pixman's stride being a multiple of 4 */
  static unsigned char data[8] = { 0xFF, 0x00, 0x00, 0x00,
                                   0x00, 0xFF, 0x00, 0x00 };
  static cairo_surface_t *checkered = NULL;
  cairo_pattern_t *pattern;

  if (checkered == NULL)
    checkered = cairo_image_surface_create_for_data (data,
                                                     CAIRO_FORMAT_A8,
                                                     2, 2, 4);

  pattern = cairo_pattern_create_for_surface (checkered);
  cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
  cairo_pattern_set_filter (pattern, CAIRO_FILTER_NEAREST);

  return pattern;
}

static cairo_surface_t *
create_h_surface (GtkWidget *widget)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  gint width, height, stride;
  cairo_surface_t *tmp;
  guint red, green, blue;
  guint32 *data, *p;
  gdouble h;
  gdouble r, g, b;
  gdouble f;
  gint x, y;

  if (!gtk_widget_get_realized (widget))
    return NULL;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               width, height);

  if (width == 1 || height == 1)
    return surface;

  stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, width);

  data = g_malloc (height * stride);

  f = 1.0 / (height - 1);
  for (y = 0; y < height; y++)
    {
      h = CLAMP (y * f, 0.0, 1.0);
      p = data + y * (stride / 4);
      for (x = 0; x < width; x++)
        {
          gtk_hsv_to_rgb (h, 1, 1, &r, &g, &b);
          red = CLAMP (r * 255, 0, 255);
          green = CLAMP (g * 255, 0, 255);
          blue = CLAMP (b * 255, 0, 255);
          p[x] = (red << 16) | (green << 8) | blue;
        }
    }

  tmp = cairo_image_surface_create_for_data ((guchar *)data, CAIRO_FORMAT_RGB24,
                                             width, height, stride);
  cr = cairo_create (surface);

  cairo_set_source_surface (cr, tmp, 0, 0);
  cairo_paint (cr);

  cairo_destroy (cr);
  cairo_surface_destroy (tmp);
  g_free (data);

  return surface;
}

static gboolean
h_draw (GtkWidget *widget,
        cairo_t   *cr,
        GtkColorEditor *editor)
{
  cairo_surface_t *surface;
  gint width, height;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  if (!editor->priv->h_surface)
    editor->priv->h_surface = create_h_surface (widget);
  surface = editor->priv->h_surface;

  cairo_save (cr);

  cairo_rectangle (cr, 1, 1, width - 2, height - 2);
  cairo_clip (cr);
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);

  cairo_restore (cr);

  return FALSE;
}

static cairo_surface_t *
create_a_surface (GtkWidget *widget,
                  GdkRGBA   *color)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_pattern_t *pattern;
  cairo_matrix_t matrix;
  gint width, height;

  if (!gtk_widget_get_realized (widget))
    return NULL;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               width, height);

  if (width == 1 || height == 1)
    return surface;

  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 0.33, 0.33, 0.33);
  cairo_paint (cr);
  cairo_set_source_rgb (cr, 0.66, 0.66, 0.66);

  pattern = get_checkered_pattern ();
  cairo_matrix_init_scale (&matrix, 0.125, 0.125);
  cairo_pattern_set_matrix (pattern, &matrix);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);

  pattern = cairo_pattern_create_linear (0, 0, width, 0);
  cairo_pattern_add_color_stop_rgba (pattern, 0, color->red, color->green, color->blue, 0);
  cairo_pattern_add_color_stop_rgba (pattern, width, color->red, color->green, color->blue, 1);
  cairo_set_source (cr, pattern);
  cairo_paint (cr);
  cairo_pattern_destroy (pattern);

  cairo_destroy (cr);

  return surface;
}

static gboolean
a_draw (GtkWidget *widget,
        cairo_t   *cr,
        GtkColorEditor *editor)
{
  cairo_surface_t *surface;
  gint width, height;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  if (!editor->priv->a_surface)
    editor->priv->a_surface = create_a_surface (widget, &editor->priv->color);
  surface = editor->priv->a_surface;

  cairo_save (cr);

  cairo_rectangle (cr, 1, 1, width - 2, height - 2);
  cairo_clip (cr);
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);

  cairo_restore (cr);

  return FALSE;
}

static void
gtk_color_editor_init (GtkColorEditor *editor)
{
  GtkWidget *grid;
  GtkAdjustment *adj;

  editor->priv = G_TYPE_INSTANCE_GET_PRIVATE (editor,
                                              GTK_TYPE_COLOR_EDITOR,
                                              GtkColorEditorPrivate);
  editor->priv->show_alpha = TRUE;

  gtk_widget_push_composite_child ();

  editor->priv->grid = grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);

  editor->priv->swatch = gtk_color_swatch_new ();
  gtk_widget_set_sensitive (editor->priv->swatch, FALSE);
  gtk_color_swatch_set_corner_radii (GTK_COLOR_SWATCH (editor->priv->swatch), 2, 2, 2, 2);

  editor->priv->entry = gtk_entry_new ();
  g_signal_connect (editor->priv->entry, "activate",
                    G_CALLBACK (entry_apply), editor);
  g_signal_connect (editor->priv->entry, "notify::text",
                    G_CALLBACK (entry_text_changed), editor);
  g_signal_connect (editor->priv->entry, "focus-out-event",
                    G_CALLBACK (entry_focus_out), editor);

  adj = gtk_adjustment_new (0, 0, 1, 0.01, 0.1, 0);
  g_signal_connect (adj, "value-changed", G_CALLBACK (h_changed), editor);
  editor->priv->h_slider = gtk_scale_new (GTK_ORIENTATION_VERTICAL, adj);
  editor->priv->h_adj = adj;
  g_signal_connect (editor->priv->h_slider, "draw", G_CALLBACK (h_draw), editor);

  gtk_scale_set_draw_value (GTK_SCALE (editor->priv->h_slider), FALSE);
  editor->priv->sv_plane = gtk_color_plane_new ();
  gtk_widget_set_size_request (editor->priv->sv_plane, 300, 300);
  gtk_widget_set_hexpand (editor->priv->sv_plane, TRUE);
  gtk_widget_set_vexpand (editor->priv->sv_plane, TRUE);

  g_signal_connect (editor->priv->sv_plane, "changed", G_CALLBACK (sv_changed), editor);

  adj = gtk_adjustment_new (1, 0, 1, 0.01, 0.1, 0);
  editor->priv->a_slider = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adj);
  g_signal_connect (adj, "value-changed", G_CALLBACK (a_changed), editor);
  gtk_scale_set_draw_value (GTK_SCALE (editor->priv->a_slider), FALSE);
  editor->priv->a_adj = adj;
  g_signal_connect (editor->priv->a_slider, "draw", G_CALLBACK (a_draw), editor);

  gtk_grid_attach (GTK_GRID (grid), editor->priv->swatch,   1, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), editor->priv->entry,    2, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), editor->priv->h_slider, 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), editor->priv->sv_plane, 1, 1, 2, 1);
  gtk_grid_attach (GTK_GRID (grid), editor->priv->a_slider, 1, 2, 2, 1);
  gtk_widget_show_all (grid);

  gtk_container_add (GTK_CONTAINER (editor), grid);
  gtk_widget_pop_composite_child ();
}

static void
gtk_color_editor_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GtkColorEditor *ce = GTK_COLOR_EDITOR (object);
  GtkColorChooser *cc = GTK_COLOR_CHOOSER (object);

  switch (prop_id)
    {
    case PROP_COLOR:
      {
        GdkRGBA color;
        gtk_color_chooser_get_color (cc, &color);
        g_value_set_boxed (value, &color);
      }
      break;
    case PROP_SHOW_ALPHA:
      g_value_set_boolean (value, gtk_widget_get_visible (ce->priv->a_slider));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_color_editor_set_show_alpha (GtkColorEditor *editor,
                                 gboolean        show_alpha)
{
  if (editor->priv->show_alpha != show_alpha)
    {
      editor->priv->show_alpha = show_alpha;

      gtk_widget_set_visible (editor->priv->a_slider, show_alpha);

      gtk_color_swatch_set_show_alpha (GTK_COLOR_SWATCH (editor->priv->swatch), show_alpha);
    }
}

static void
gtk_color_editor_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GtkColorEditor *ce = GTK_COLOR_EDITOR (object);
  GtkColorChooser *cc = GTK_COLOR_CHOOSER (object);

  switch (prop_id)
    {
    case PROP_COLOR:
      gtk_color_chooser_set_color (cc, g_value_get_boxed (value));
      break;
    case PROP_SHOW_ALPHA:
      gtk_color_editor_set_show_alpha (ce, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_color_editor_finalize (GObject *object)
{
  GtkColorEditor *editor = GTK_COLOR_EDITOR (object);

  if (editor->priv->h_surface)
    cairo_surface_destroy (editor->priv->h_surface);
  if (editor->priv->a_surface)
    cairo_surface_destroy (editor->priv->a_surface);

  G_OBJECT_CLASS (gtk_color_editor_parent_class)->finalize (object);
}

static void
gtk_color_editor_class_init (GtkColorEditorClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = gtk_color_editor_finalize;
  object_class->get_property = gtk_color_editor_get_property;
  object_class->set_property = gtk_color_editor_set_property;

  g_object_class_override_property (object_class, PROP_COLOR, "color");
  g_object_class_override_property (object_class, PROP_SHOW_ALPHA, "show-alpha");

  g_type_class_add_private (class, sizeof (GtkColorEditorPrivate));
}

static void
gtk_color_editor_get_color (GtkColorChooser *chooser,
                            GdkRGBA         *color)
{
  GtkColorEditor *editor = GTK_COLOR_EDITOR (chooser);

  color->red = editor->priv->color.red;
  color->green = editor->priv->color.green;
  color->blue = editor->priv->color.blue;
  color->alpha = editor->priv->color.alpha;
}

static void
gtk_color_editor_set_color (GtkColorChooser *chooser,
                            const GdkRGBA   *color)
{
  GtkColorEditor *editor = GTK_COLOR_EDITOR (chooser);
  gdouble h, s, v;

  editor->priv->color.red = color->red;
  editor->priv->color.green = color->green;
  editor->priv->color.blue = color->blue;
  editor->priv->color.alpha = color->alpha;
  gtk_rgb_to_hsv (color->red, color->green, color->blue, &h, &s, &v);
  editor->priv->h = h;
  editor->priv->s = s;
  editor->priv->v = v;
  gtk_color_plane_set_h (GTK_COLOR_PLANE (editor->priv->sv_plane), h);
  gtk_color_plane_set_s (GTK_COLOR_PLANE (editor->priv->sv_plane), s);
  gtk_color_plane_set_v (GTK_COLOR_PLANE (editor->priv->sv_plane), v);
  gtk_color_swatch_set_color (GTK_COLOR_SWATCH (editor->priv->swatch), color);

  gtk_adjustment_set_value (editor->priv->h_adj, h);
  gtk_adjustment_set_value (editor->priv->a_adj, color->alpha);
  update_entry (editor);

  if (editor->priv->a_surface)
    {
      cairo_surface_destroy (editor->priv->a_surface);
      editor->priv->a_surface = NULL;
    }
  gtk_widget_queue_draw (GTK_WIDGET (editor));

  g_object_notify (G_OBJECT (editor), "color");
}

static void
gtk_color_editor_iface_init (GtkColorChooserInterface *iface)
{
  iface->get_color = gtk_color_editor_get_color;
  iface->set_color = gtk_color_editor_set_color;
}

GtkWidget *
gtk_color_editor_new (void)
{
  return (GtkWidget *) g_object_new (GTK_TYPE_COLOR_EDITOR, NULL);
}