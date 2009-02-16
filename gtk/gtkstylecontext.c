/*
 * GTK - The GIMP Toolkit
 * Copyright (C) 2009  Carlos Garnacho Parro <carlosg@gnome.org>
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

#include <gtk/gtkstylecontext.h>
#include <gtk/gtktypebuiltins.h>
#include "gtkwidget.h"

#define GTK_STYLE_CONTEXT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_STYLE_CONTEXT, GtkStyleContextPrivate))
#define GET_CURRENT_CONTEXT(p) ((GHashTable *) (p)->context_stack->data)

typedef struct GtkStyleContextPrivate GtkStyleContextPrivate;
typedef struct GtkStyleContextAnimInfo GtkStyleContextAnimInfo;

struct GtkStyleContextPrivate
{
  GHashTable *composed_context;
  GList *context_stack;
  GList *reverse_context_stack;

  GList *animations;
  GList *regions_stack;
};

struct GtkStyleContextAnimInfo
{
  GtkTimeline *timeline;
  gpointer identifier;
  GtkWidgetState state;
  gboolean target_value;
};


static void gtk_style_context_finalize (GObject *object);

static GtkTimeline * gtk_style_context_create_animation (GtkStyleContext *context,
                                                         GtkWidgetState   state);

static void gtk_style_context_paint_box (GtkStyleContext *context,
                                         cairo_t         *cr,
                                         gint             x,
                                         gint             y,
                                         gint             width,
                                         gint             height);

G_DEFINE_TYPE (GtkStyleContext, gtk_style_context, G_TYPE_OBJECT)

static void
gtk_style_context_class_init (GtkStyleContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtk_style_context_finalize;

  klass->create_animation = gtk_style_context_create_animation;
  klass->paint_box = gtk_style_context_paint_box;

  g_type_class_add_private (klass, sizeof (GtkStyleContextPrivate));
}

static void
free_value (GValue *value)
{
  if (value)
    {
      g_value_unset (value);
      g_free (value);
    }
}

static GHashTable *
create_subcontext (void)
{
  return g_hash_table_new_full (g_str_hash,
                                g_str_equal,
                                (GDestroyNotify) g_free,
                                (GDestroyNotify) free_value);
}

static void
gtk_style_context_init (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  priv->composed_context = g_hash_table_new (g_str_hash, g_str_equal);
  priv->reverse_context_stack = priv->context_stack = g_list_prepend (priv->context_stack, create_subcontext ());
}

static void
gtk_style_context_finalize (GObject *object)
{
  GtkStyleContextPrivate *priv;

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (object);

  g_list_foreach (priv->context_stack, (GFunc) g_hash_table_destroy, NULL);
  g_list_free (priv->context_stack);

  g_hash_table_destroy (priv->composed_context);

  G_OBJECT_CLASS (gtk_style_context_parent_class)->finalize (object);
}

void
gtk_style_context_save (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  /* Create a new subcontext for new changes */
  priv->context_stack = g_list_prepend (priv->context_stack, create_subcontext ());
}

static void
set_context_param (gpointer    key,
                   gpointer    value,
                   GHashTable *context)
{
  g_hash_table_insert (context, key, value);
}

static void
recalculate_composed_context (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;
  GList *elem;

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  g_hash_table_remove_all (priv->composed_context);

  for (elem = priv->reverse_context_stack; elem; elem = elem->prev)
    {
      GHashTable *subcontext;

      subcontext = elem->data;

      g_hash_table_foreach (subcontext, (GHFunc) set_context_param, priv->composed_context);
    }
}

void
gtk_style_context_restore (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  if (priv->context_stack)
    {
      GList *elem;

      elem = priv->context_stack;
      priv->context_stack = g_list_remove_link (priv->context_stack, elem);

      g_hash_table_destroy (elem->data);
      g_list_free_1 (elem);
    }

  if (!priv->context_stack)
    {
      /* Restore an empty subcontext */
      priv->reverse_context_stack = priv->context_stack = g_list_prepend (priv->context_stack, create_subcontext ());
    }

  recalculate_composed_context (context);
}

void
gtk_style_context_set_param (GtkStyleContext *context,
                             const gchar     *param,
                             const GValue    *value)
{
  GtkStyleContextPrivate *priv;
  GValue *copy = NULL;
  gchar *str;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));
  g_return_if_fail (param != NULL);

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  str = g_strdup (param);

  if (value)
    {
      copy = g_new0 (GValue, 1);
      g_value_init (copy, G_VALUE_TYPE (value));
      g_value_copy (value, copy);
    }

  /* Insert in current context, replacing key */
  g_hash_table_replace (GET_CURRENT_CONTEXT (priv), str, copy);

  /* And in composed context */
  g_hash_table_insert (priv->composed_context, str, copy);
}

gboolean
gtk_style_context_get_param (GtkStyleContext *context,
                             const gchar     *param,
                             GValue          *value)
{
  GtkStyleContextPrivate *priv;
  GValue *val;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), FALSE);
  g_return_val_if_fail (param != NULL, FALSE);

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  val = g_hash_table_lookup (priv->composed_context, param);

  if (!val)
    return FALSE;

  if (value)
    {
      g_value_init (value, G_VALUE_TYPE (val));
      g_value_copy (val, value);
    }

  return TRUE;
}

gboolean
gtk_style_context_param_exists (GtkStyleContext *context,
                                const gchar     *param)
{
  return gtk_style_context_get_param (context, param, NULL);
}

void
gtk_style_context_unset_param (GtkStyleContext *context,
                               const gchar     *param)
{
  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));
  g_return_if_fail (param != NULL);

  gtk_style_context_set_param (context, param, NULL);
}

void
gtk_style_context_set_param_int (GtkStyleContext *context,
                                 const gchar     *param,
                                 gint             param_value)
{
  GValue value = { 0 };

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));
  g_return_if_fail (param != NULL);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, param_value);

  gtk_style_context_set_param (context, param, &value);
  g_value_unset (&value);
}

gint
gtk_style_context_get_param_int (GtkStyleContext *context,
                                 const gchar     *param)
{
  GValue value = { 0 };
  gint retval;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), 0);
  g_return_val_if_fail (param != NULL, 0);

  if (!gtk_style_context_get_param (context, param, &value))
    return 0;

  retval = g_value_get_int (&value);
  g_value_unset (&value);

  return retval;
}

void
gtk_style_context_set_param_flag (GtkStyleContext *context,
                                  const gchar     *param)
{
  GValue value = { 0 };

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));
  g_return_if_fail (param != NULL);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, TRUE);

  gtk_style_context_set_param (context, param, &value);
  g_value_unset (&value);
}

void
gtk_style_context_set_gtype (GtkStyleContext *context,
                             GType            type)
{
  GValue value = { 0 };

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  g_value_init (&value, G_TYPE_GTYPE);
  g_value_set_gtype (&value, type);

  gtk_style_context_set_param (context, GTK_STYLE_CONTEXT_PARAMETER_WIDGET_GTYPE, &value);
  g_value_unset (&value);
}

GType
gtk_style_context_get_gtype (GtkStyleContext *context)
{
  GValue value = { 0 };
  GType type;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), G_TYPE_INVALID);

  if (!gtk_style_context_get_param (context, GTK_STYLE_CONTEXT_PARAMETER_WIDGET_GTYPE, &value))
    return G_TYPE_INVALID;

  type = g_value_get_gtype (&value);
  g_value_unset (&value);

  return type;
}

void
gtk_style_context_set_clip_area (GtkStyleContext    *context,
                                 const GdkRectangle *clip_area)
{
  GValue value = { 0 };

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  g_value_init (&value, GDK_TYPE_RECTANGLE);
  g_value_set_boxed (&value, clip_area);

  gtk_style_context_set_param (context, GTK_STYLE_CONTEXT_PARAMETER_EXPOSE_CLIP_AREA, &value);
  g_value_unset (&value);
}

gboolean
gtk_style_context_get_clip_area (GtkStyleContext *context,
                                 GdkRectangle    *rectangle)
{
  GValue value = { 0 };
  GdkRectangle *rect;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), FALSE);

  if (!gtk_style_context_get_param (context, GTK_STYLE_CONTEXT_PARAMETER_EXPOSE_CLIP_AREA, &value))
    return FALSE;

  rect = g_value_get_boxed (&value);

  if (rectangle)
    *rectangle = *rect;

  g_value_unset (&value);

  return TRUE;
}

void
gtk_style_context_set_role (GtkStyleContext     *context,
                            GtkStyleContextRole  role)
{
  GValue value = { 0 };

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  g_value_init (&value, GTK_TYPE_PLACING_CONTEXT);
  g_value_set_enum (&value, role);

  gtk_style_context_set_param (context, GTK_STYLE_CONTEXT_PARAMETER_EXPOSE_CONTENT_ROLE, &value);
  g_value_unset (&value);
}

GtkStyleContextRole
gtk_style_context_get_role (GtkStyleContext *context)
{
  GValue value = { 0 };
  GtkStyleContextRole role;

  role = GTK_STYLE_CONTEXT_ROLE_INVALID;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), role);

  if (!gtk_style_context_get_param (context, GTK_STYLE_CONTEXT_PARAMETER_EXPOSE_CONTENT_ROLE, &value))
    return role;

  role = g_value_get_enum (&value);
  g_value_unset (&value);

  return role;
}

void
gtk_style_context_set_state (GtkStyleContext *context,
                             GtkWidgetState   state)
{
  GValue value = { 0 };

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  g_value_init (&value, GTK_TYPE_WIDGET_STATE);
  g_value_set_flags (&value, state);

  gtk_style_context_set_param (context, GTK_STYLE_CONTEXT_PARAMETER_WIDGET_STATE, &value);
  g_value_unset (&value);
}

GtkWidgetState
gtk_style_context_get_state (GtkStyleContext *context)
{
  GValue value = { 0 };
  GtkWidgetState state;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), 0);

  if (!gtk_style_context_get_param (context, GTK_STYLE_CONTEXT_PARAMETER_WIDGET_STATE, &value))
    return 0;

  state = g_value_get_flags (&value);
  g_value_unset (&value);

  return state;
}

void
gtk_style_context_set_state_flags (GtkStyleContext *context,
                                  GtkWidgetState   state)
{
  GtkWidgetState saved_state;

  saved_state = gtk_style_context_get_state (context);
  saved_state |= state;
  gtk_style_context_set_state (context, saved_state);
}

void
gtk_style_context_unset_state_flags (GtkStyleContext *context,
                                     GtkWidgetState   state)
{
  GtkWidgetState saved_state;

  saved_state = gtk_style_context_get_state (context);
  saved_state &= ~(state);
  gtk_style_context_set_state (context, saved_state);
}

void
gtk_style_context_set_placing_context (GtkStyleContext   *context,
                                       GtkPlacingContext  placing)
{
  GValue value = { 0 };

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  g_value_init (&value, GTK_TYPE_PLACING_CONTEXT);
  g_value_set_flags (&value, placing);

  gtk_style_context_set_param (context, GTK_STYLE_CONTEXT_PARAMETER_WIDGET_PLACING, &value);
  g_value_unset (&value);
}

GtkPlacingContext
gtk_style_context_get_placing_context (GtkStyleContext *context)
{
  GValue value = { 0 };
  GtkPlacingContext placing;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), 0);

  if (!gtk_style_context_get_param (context, GTK_STYLE_CONTEXT_PARAMETER_WIDGET_PLACING, &value))
    return 0;

  placing = g_value_get_flags (&value);
  g_value_unset (&value);

  return placing;
}

void
gtk_style_context_set_color (GtkStyleContext *context,
                             const GdkColor  *color)
{
  GValue value = { 0 };

  /* FIXME: Obviously there's not going to be just one color */

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));
  g_return_if_fail (color != NULL);

  g_value_init (&value, GDK_TYPE_COLOR);
  g_value_set_boxed (&value, color);

  gtk_style_context_set_param (context, "color", &value);
  g_value_unset (&value);
}

gboolean
gtk_style_context_get_color (GtkStyleContext *context,
                             GdkColor        *color)
{
  GValue value = { 0 };
  GdkColor *c;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), FALSE);

  if (!gtk_style_context_get_param (context, "color", &value))
    return FALSE;

  c = g_value_get_boxed (&value);

  if (color)
    *color = *c;

  g_value_unset (&value);

  return TRUE;
}

static GtkTimeline *
gtk_style_context_create_animation (GtkStyleContext *context,
                                    GtkWidgetState   state)
{
  GtkTimeline *timeline;

  timeline = gtk_timeline_new (2000);

  return timeline;
}

/* Paint methods default implementations */
static void
gtk_style_context_paint_box (GtkStyleContext *context,
                             cairo_t         *cr,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height)
{
  GtkPlacingContext placing;
  GdkColor color;
  gint radius;
  gdouble progress;

  radius = gtk_style_context_get_param_int (context, "radius");
  placing = gtk_style_context_get_placing_context (context);

  if (gtk_style_context_get_region_progress (context, NULL, GTK_WIDGET_STATE_PRELIGHT, &progress))
    cairo_set_source_rgb (cr, progress, progress, progress);
  else
    {
      GtkWidgetState state;

      state = gtk_style_context_get_state (context);

      if (gtk_style_context_get_color (context, &color))
        gdk_cairo_set_source_color (cr, &color);
      else if (state & GTK_WIDGET_STATE_PRELIGHT)
        cairo_set_source_rgb (cr, 1., 1., 1.);
      else
        cairo_set_source_rgb (cr, 0., 0., 0.);
    }

  if (radius == 0)
    cairo_rectangle (cr,
                     (gdouble) x,
                     (gdouble) y,
                     (gdouble) width,
                     (gdouble) height);
  else
    {
      cairo_move_to (cr, x + radius, y);

      /* top line and top-right corner */
      if (placing & GTK_PLACING_CONNECTS_UP ||
          placing & GTK_PLACING_CONNECTS_RIGHT)
        cairo_line_to (cr, x + width, y);
      else
        {
          cairo_line_to (cr, x + width - radius, y);
          cairo_curve_to (cr, x + width, y, x + width, y, x + width, y + radius);
        }

      /* right line and bottom-right corner */
      if (placing & GTK_PLACING_CONNECTS_DOWN ||
          placing & GTK_PLACING_CONNECTS_RIGHT)
        cairo_line_to (cr, x + width, y + height);
      else
        {
          cairo_line_to (cr, x + width, y + height - radius);
          cairo_curve_to (cr, x + width, y + height, x + width, y + height, x + width - radius, y + height);
        }

      /* bottom line and bottom-left corner */
      if (placing & GTK_PLACING_CONNECTS_DOWN ||
          placing & GTK_PLACING_CONNECTS_LEFT)
        cairo_line_to (cr, x, y + height);
      else
        {
          cairo_line_to (cr, x + radius, y + height);
          cairo_curve_to (cr, x, y + height, x, y + height, x, y + height - radius);
        }

      /* left line and top-left corner */
      if (placing & GTK_PLACING_CONNECTS_UP ||
          placing & GTK_PLACING_CONNECTS_LEFT)
        cairo_line_to (cr, x, y);
      else
        {
          cairo_line_to (cr, x, y + radius);
          cairo_curve_to (cr, x, y, x, y, x + radius, y);
        }

      cairo_close_path (cr);
    }

  if (gtk_style_context_param_exists (context, "flat"))
    cairo_fill (cr);
  else
    {
      cairo_fill_preserve (cr);

      cairo_set_source_rgb (cr, 0, 0, 0);
      cairo_stroke (cr);
    }
}

/* Animation functions */
void
gtk_style_context_push_region (GtkStyleContext *context,
                               gpointer         identifier)
{
  GtkStyleContextPrivate *priv;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  priv->regions_stack = g_list_prepend (priv->regions_stack, identifier);
}

void
gtk_style_context_pop_region (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);
  priv->regions_stack = g_list_delete_link (priv->regions_stack, priv->regions_stack);
}

static GtkStyleContextAnimInfo *
_gtk_timeline_context_get_anim_info (GtkStyleContext *context,
                                     gpointer         identifier,
                                     GtkWidgetState   state)
{
  GtkStyleContextAnimInfo *anim_info;
  GtkStyleContextPrivate *priv;
  GList *anims;

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  for (anims = priv->animations; anims; anims = anims->next)
    {
      anim_info = anims->data;

      if (anim_info->state == state &&
          anim_info->identifier == identifier)
        return anim_info;
    }

  return NULL;
}

static void
timeline_frame_cb (GtkTimeline *timeline,
                   gdouble      progress,
                   gpointer     user_data)
{
  GtkWidget *widget;

  g_print (">>> %f\n", progress);
  widget = GTK_WIDGET (user_data);

  gtk_widget_queue_draw (widget);
}

static void
timeline_finished_cb (GtkTimeline *timeline,
                      gpointer     user_data)
{
  GtkStyleContextAnimInfo *anim_info;
  GtkStyleContextPrivate *priv;
  GtkStyleContext *context;
  GList *anims;

  context = GTK_STYLE_CONTEXT (user_data);
  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  for (anims = priv->animations; anims; anims = anims->next)
    {
      anim_info = anims->data;

      if (anim_info->timeline == timeline)
        {
          priv->animations = g_list_delete_link (priv->animations, anims);

          g_object_unref (timeline);
          g_slice_free (GtkStyleContextAnimInfo, anim_info);
          break;
        }
    }
}

void
gtk_style_context_modify_state (GtkStyleContext *context,
                                GtkWidget       *widget,
                                gpointer         identifier,
                                GtkWidgetState   state,
                                gboolean         target_value)
{
  GtkTimeline *timeline;
  GtkStyleContextAnimInfo *anim_info;
  GtkStyleContextPrivate *priv;

  /* First check if there is any animation already running */
  anim_info = _gtk_timeline_context_get_anim_info (context, identifier, state);

  g_print ("anim_info? %p\n", anim_info);

  if (anim_info)
    {
      /* Reverse the animation if target values are the opposite */
      if (anim_info->target_value != target_value)
        {
          gtk_timeline_set_direction (anim_info->timeline, GTK_TIMELINE_DIRECTION_BACKWARD);
          anim_info->target_value = target_value;
        }

      return;
    }

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);
  timeline = GTK_STYLE_CONTEXT_GET_CLASS (context)->create_animation (context, state);

  if (!timeline)
    return;

  anim_info = g_slice_new (GtkStyleContextAnimInfo);
  anim_info->identifier = identifier;
  anim_info->state = state;
  anim_info->target_value = target_value;
  anim_info->timeline = timeline;

  if (target_value == FALSE)
    {
      g_print ("reversing animation\n");
      gtk_timeline_set_direction (timeline, GTK_TIMELINE_DIRECTION_BACKWARD);
      gtk_timeline_rewind (timeline);
    }

  gtk_timeline_start (timeline);
  g_signal_connect (timeline, "frame",
                    G_CALLBACK (timeline_frame_cb), widget);
  g_signal_connect (timeline, "finished",
                    G_CALLBACK (timeline_finished_cb), context);

  priv->animations = g_list_prepend (priv->animations, anim_info);
}

gboolean
gtk_style_context_get_region_progress (GtkStyleContext *context,
                                       gpointer         identifier,
                                       GtkWidgetState   state,
                                       gdouble         *progress)
{
  GtkStyleContextAnimInfo *anim_info;

  anim_info = _gtk_timeline_context_get_anim_info (context, identifier, state);

  if (anim_info)
    {
      if (progress)
        *progress = gtk_timeline_get_progress (anim_info->timeline);

      return TRUE;
    }

  return FALSE;
}

/* Paint functions */
void
gtk_depict_box (GtkStyleContext *context,
                cairo_t         *cr,
                gint             x,
                gint             y,
                gint             width,
                gint             height)
{
  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));
  g_return_if_fail (cr != NULL);

  GTK_STYLE_CONTEXT_GET_CLASS (context)->paint_box (context, cr, x, y, width, height);
}


#define __GTK_STYLE_CONTEXT_C__
#include "gtkaliasdef.c"
