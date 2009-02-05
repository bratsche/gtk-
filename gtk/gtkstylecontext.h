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

#ifndef __GTK_STYLE_CONTEXT_H__
#define __GTK_STYLE_CONTEXT_H__

#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

#define GTK_TYPE_STYLE_CONTEXT          (gtk_style_context_get_type ())
#define GTK_STYLE_CONTEXT(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_STYLE_CONTEXT, GtkStyleContext))
#define GTK_STYLE_CONTEXT_CLASS(c)      (G_TYPE_CHECK_CLASS_CAST ((c),    GTK_TYPE_STYLE_CONTEXT, GtkStyleContextClass))
#define GTK_IS_STYLE_CONTEXT(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_STYLE_CONTEXT))
#define GTK_IS_STYLE_CONTEXT_CLASS(c)   (G_TYPE_CHECK_CLASS_TYPE ((c),    GTK_TYPE_STYLE_CONTEXT))
#define GTK_STYLE_CONTEXT_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o),  GTK_TYPE_STYLE_CONTEXT, GtkStyleContextClass))

/* Usual context parameters */
#define GTK_STYLE_CONTEXT_PARAMETER_WIDGET_GTYPE        "widget::gtype"
#define GTK_STYLE_CONTEXT_PARAMETER_WIDGET_STATE        "widget::state"
#define GTK_STYLE_CONTEXT_PARAMETER_WIDGET_PLACING      "widget::placing"
#define GTK_STYLE_CONTEXT_PARAMETER_EXPOSE_CLIP_AREA    "expose::clip-area"
#define GTK_STYLE_CONTEXT_PARAMETER_EXPOSE_CONTENT_ROLE "expose::content-role"

typedef struct GtkStyleContext GtkStyleContext;
typedef struct GtkStyleContextClass GtkStyleContextClass;

struct GtkStyleContext
{
  GObject parent_instance;
};

struct GtkStyleContextClass
{
  GObjectClass parent_class;

  void (* paint_box) (GtkStyleContext *context,
                      cairo_t         *cr,
                      gint             x,
                      gint             y,
                      gint             width,
                      gint             height);
};


GType     gtk_style_context_get_type (void) G_GNUC_CONST;

/* Context saving/restoring */
void      gtk_style_context_save         (GtkStyleContext *context);
void      gtk_style_context_restore      (GtkStyleContext *context);

/* Generic parameter functions */
void      gtk_style_context_set_param    (GtkStyleContext *context,
                                          const gchar     *param,
                                          const GValue    *value);
gboolean  gtk_style_context_get_param    (GtkStyleContext *context,
                                          const gchar     *param,
                                          GValue          *value);
gboolean  gtk_style_context_param_exists (GtkStyleContext *context,
                                          const gchar     *param);
void      gtk_style_context_unset_param  (GtkStyleContext *context,
                                          const gchar     *param);

/* Helpers for different types */
void      gtk_style_context_set_param_int  (GtkStyleContext *context,
                                            const gchar     *param,
                                            gint             param_value);
gint      gtk_style_context_get_param_int  (GtkStyleContext *context,
                                            const gchar     *param);

void      gtk_style_context_set_param_flag (GtkStyleContext *context,
                                            const gchar     *param);

/* Specific option helpers */
void      gtk_style_context_set_gtype           (GtkStyleContext     *context,
                                                 GType                type);
GType     gtk_style_context_get_gtype           (GtkStyleContext     *context);

void      gtk_style_context_set_clip_area       (GtkStyleContext     *context,
                                                 const GdkRectangle  *clip_area);
gboolean  gtk_style_context_get_clip_area       (GtkStyleContext     *context,
                                                 GdkRectangle        *rectangle);
void      gtk_style_context_set_role            (GtkStyleContext     *context,
                                                 GtkStyleContextRole  role);
GtkStyleContextRole
          gtk_style_context_get_role            (GtkStyleContext     *context);

void      gtk_style_context_set_state           (GtkStyleContext     *context,
                                                 GtkWidgetState       state);
GtkWidgetState
          gtk_style_context_get_state           (GtkStyleContext     *context);

void      gtk_style_context_set_placing_context (GtkStyleContext     *context,
                                                 GtkPlacingContext    placing);
GtkPlacingContext
          gtk_style_context_get_placing_context (GtkStyleContext     *context);

void      gtk_style_context_set_color           (GtkStyleContext     *context,
                                                 const GdkColor      *color);
gboolean  gtk_style_context_get_color           (GtkStyleContext     *context,
                                                 GdkColor            *color);

/* Paint functions */
void      gtk_depict_box                        (GtkStyleContext *context,
                                                 cairo_t         *cr,
                                                 gint             x,
                                                 gint             y,
                                                 gint             width,
                                                 gint             height);

G_END_DECLS

#endif /* __GTK_STYLE_CONTEXT_H__ */
