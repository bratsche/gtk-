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

typedef struct GtkStyleContext GtkStyleContext;
typedef struct GtkStyleContextClass GtkStyleContextClass;

struct GtkStyleContext
{
  GObject parent_instance;
};

struct GtkStyleContextClass
{
  GObjectClass parent_class;
};


GType     gtk_style_context_get_type (void) G_GNUC_CONST;

void      gtk_style_context_save         (GtkStyleContext *context);
void      gtk_style_context_restore      (GtkStyleContext *context);

void      gtk_style_context_add_param    (GtkStyleContext *context,
                                          const gchar     *param);
gboolean  gtk_style_context_param_exists (GtkStyleContext *context,
                                          const gchar     *param);

G_END_DECLS

#endif /* __GTK_STYLE_CONTEXT_H__ */
