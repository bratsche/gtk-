/*
 * Copyright 2010 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of either or both of the following licenses:
 *
 * 1) the GNU Lesser General Public License version 3, as published by the
 * Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authors:
 *    Cody Russell <crussell@canonical.com>
 */

#ifndef __GTK_SCALE_MENU_ITEM_H__
#define __GTK_SCALE_MENU_ITEM_H__

#include <gtk/gtkmenuitem.h>
#include "gtkrange.h"

G_BEGIN_DECLS

#define GTK_TYPE_SCALE_MENU_ITEM         (gtk_scale_menu_item_get_type ())
#define GTK_SCALE_MENU_ITEM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_SCALE_MENU_ITEM, GtkScaleMenuItem))
#define GTK_SCALE_MENU_ITEM_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c), GTK_TYPE_SCALE_MENU_ITEM, GtkScaleMenuItemClass))
#define GTK_IS_SCALE_MENU_ITEM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_SCALE_MENU_ITEM))
#define GTK_IS_SCALE_MENU_ITEM_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c), GTK_TYPE_SCALE_MENU_ITEM))
#define GTK_SCALE_MENU_ITEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GTK_TYPE_SCALE_MENU_ITEM, GtkScaleMenuItemClass))

/*
typedef enum
{
  GTK_SCALE_MENU_ITEM_STYLE_NONE,
  GTK_SCALE_MENU_ITEM_STYLE_IMAGE,
  GTK_SCALE_MENU_ITEM_STYLE_LABEL
} GtkScaleMenuItemStyle;
*/

typedef struct _GtkScaleMenuItem        GtkScaleMenuItem;
typedef struct _GtkScaleMenuItemClass   GtkScaleMenuItemClass;
typedef struct _GtkScaleMenuItemPrivate GtkScaleMenuItemPrivate;

struct _GtkScaleMenuItem
{
  GtkMenuItem parent_instance;

  GtkScaleMenuItemPrivate *priv;
};

struct _GtkScaleMenuItemClass
{
  GtkMenuItemClass parent_class;
};


GType	   gtk_scale_menu_item_get_type            (void) G_GNUC_CONST;
GtkWidget *gtk_scale_menu_item_new                 (const gchar      *label,
                                                    //GtkRangeStyle     size,
                                                    GtkAdjustment    *adjustment);
GtkWidget *gtk_scale_menu_item_new_with_range      (const gchar      *label,
                                                    //GtkRangeStyle     size,
                                                    gdouble           value,
                                                    gdouble           min,
                                                    gdouble           max,
                                                    gdouble           step);
GtkWidget *gtk_scale_menu_item_get_scale           (GtkScaleMenuItem *menuitem);

GtkScaleMenuItemStyle  gtk_scale_menu_item_get_style           (GtkScaleMenuItem      *menuitem);
void                   gtk_scale_menu_item_set_style           (GtkScaleMenuItem      *menuitem,
                                                                GtkScaleMenuItemStyle  style);
GtkWidget             *gtk_scale_menu_item_get_primary_image   (GtkScaleMenuItem      *menuitem);
GtkWidget             *gtk_scale_menu_item_get_secondary_image (GtkScaleMenuItem      *menuitem);
G_CONST_RETURN gchar  *gtk_scale_menu_item_get_primary_label   (GtkScaleMenuItem      *menuitem);
G_CONST_RETURN gchar  *gtk_scale_menu_item_get_secondary_label (GtkScaleMenuItem      *menuitem);
void                   gtk_scale_menu_item_set_primary_label   (GtkScaleMenuItem      *menuitem,
                                                                const gchar           *label);
void                   gtk_scale_menu_item_set_secondary_label (GtkScaleMenuItem      *menuitem,
                                                                const gchar           *label);

G_END_DECLS

#endif /* __GTK_SCALE_MENU_ITEM_H__ */
