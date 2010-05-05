/*
 * Copyright (C) 2009 Canonical, Ltd.
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
 *
 * Authors: Cody Russell <bratsche@gnome.org>
 */

#include "config.h"
#include "gtkintl.h"
#include "gtkmarshalers.h"
#include "gtkmenuproxy.h"
#include "gtkmenuproxymodule.h"
#include "gtkmodules.h"

enum {
  INSERTED,
  LAST_SIGNAL
};

static guint               menu_proxy_signals[LAST_SIGNAL] = { 0 };

static void gtk_menu_proxy_real_insert (GtkMenuProxy *proxy,
                                        GtkWidget    *child,
                                        guint         position);



/* --------------------------------------------------------- */

G_DEFINE_TYPE (GtkMenuProxy, gtk_menu_proxy, G_TYPE_OBJECT)

static void
gtk_menu_proxy_init (GtkMenuProxy *proxy)
{
}

static void
gtk_menu_proxy_class_init (GtkMenuProxyClass *class)
{
  gtk_menu_proxy_parent_class = g_type_class_peek_parent (class);

  menu_proxy_signals[INSERTED] =
    g_signal_new (I_("inserted"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkMenuProxyClass, inserted),
                  NULL, NULL,
                  _gtk_marshal_VOID__OBJECT_UINT,
                  G_TYPE_NONE, 2,
                  GTK_TYPE_WIDGET, G_TYPE_UINT);

  class->insert = gtk_menu_proxy_real_insert;
}

GtkMenuProxy *
gtk_menu_proxy_get (void)
{
  GtkMenuProxyModule *module = NULL;
  GType *proxy_types = NULL;
  guint  n_proxies = 0;

  module = gtk_menu_proxy_module_get ();

  if (!module)
    {
      g_warning (" ** Failed to open menu proxy module!");
      return NULL;
    }

  proxy_types = g_type_children (GTK_TYPE_MENU_PROXY,
                                 &n_proxies);

  g_print ("n_proxies == %d\n", n_proxies);

  if (n_proxies > 1)
    {
      g_warning ("There are %d child types of GtkMenuProxy, should be 0 or 1.\n",
                 n_proxies);
    }

  if (proxy_types != NULL)
    {
      g_print (" ===========> type name is: %s\n", g_type_name (proxy_types[0]));
      //g_print ("is a GtkMenuProxy: %d\n", g_type_is_a (proxy_types[0], gtk_menu_proxy_get_type ()));
      return NULL;
      //return g_object_new (proxy_types[0], NULL);
    }
  else
    {
      return NULL;
    }
}

static void
gtk_menu_proxy_real_insert (GtkMenuProxy *proxy,
                            GtkWidget    *child,
                            guint         position)
{
}

void
gtk_menu_proxy_insert (GtkMenuProxy *proxy,
                       GtkWidget    *child,
                       guint         position)
{
  g_return_if_fail (GTK_IS_MENU_PROXY (proxy));

  GTK_MENU_PROXY_GET_CLASS (proxy)->insert (proxy,
                                            child,
                                            position);

  //g_signal_emit_by_name (proxy, "inserted", child, position);
}

