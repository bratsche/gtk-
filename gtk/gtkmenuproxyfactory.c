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

#include "gtkmenuproxyfactory.h"

enum {
  PROP_0,
  PROP_MODULE_PATH
};

static GObject* gtk_menu_proxy_factory_constructor   (GType                  type,
                                                      guint                  n_params,
                                                      GObjectConstructParam *params);
static void     gtk_menu_proxy_factory_finalize      (GObject               *object);
static void     gtk_menu_proxy_factory_get_property  (GObject               *object,
                                                      guint                  param_id,
                                                      GValue                *value,
                                                      GParamSpec            *pspec);
static void     gtk_menu_proxy_factory_set_property  (GObject               *object,
                                                      guint                  param_id,
                                                      const GValue          *value,
                                                      GParamSpec            *pspec);
static gboolean gtk_menu_proxy_factory_query_modules (GtkMenuProxyFactory   *factory);

G_DEFINE_TYPE (GtkMenuProxyFactory, gtk_menu_proxy_factory, G_TYPE_OBJECT);

static GtkMenuProxyFactory *factory_singleton = NULL;

static void
gtk_menu_proxy_factory_class_init (GtkMenuProxyFactoryClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->constructor  = gtk_menu_proxy_factory_constructor;
  gobject_class->finalize     = gtk_menu_proxy_factory_finalize;
  gobject_class->get_property = gtk_menu_proxy_factory_get_property;
  gobject_class->set_property = gtk_menu_proxy_factory_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_MODULE_PATH,
                                   g_param_spec_string ("module-path",
                                                        "Module Path",
                                                        "Path the search for modules",
                                                        NULL,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
gtk_menu_proxy_factory_init (GtkMenuProxyFactory *factory)
{
  factory->module_path = g_strdup ("/usr/lib/gtk-2.0/2.10.0/menuproxies");
  factory->modules     = NULL;
}

static GObject*
gtk_menu_proxy_factory_constructor (GType type,
                                    guint n_params,
                                    GObjectConstructParam *params)
{
  GObject             *object;
  GtkMenuProxyFactory *factory;

  if (factory_singleton != NULL)
    {
      object = g_object_ref (factory_singleton);
    }
  else
    {
      g_print ("Creating GtkMenuProxyFactory...\n");

      object = G_OBJECT_CLASS (gtk_menu_proxy_factory_parent_class)->constructor (type,
                                                                                  n_params,
                                                                                  params);

      factory_singleton = GTK_MENU_PROXY_FACTORY (object);
      g_object_add_weak_pointer (object, (gpointer) &factory_singleton);

      factory = GTK_MENU_PROXY_FACTORY (object);

      //if (factory->module_path)
      if (1)
        {
          gtk_menu_proxy_factory_query_modules (factory);
        }
    }

  return object;
}

static void
gtk_menu_proxy_factory_finalize (GObject *object)
{
  GtkMenuProxyFactory *factory = GTK_MENU_PROXY_FACTORY (object);

  if (factory->module_path != NULL)
    g_free (factory->module_path);

  /* Do not unref type modules */
  if (factory->modules != NULL)
    g_list_free (factory->modules);

  G_OBJECT_CLASS (gtk_menu_proxy_factory_parent_class)->finalize (object);
}

static void
gtk_menu_proxy_factory_get_property (GObject               *object,
                                     guint                  param_id,
                                     GValue                *value,
                                     GParamSpec            *pspec)
{
  GtkMenuProxyFactory *factory = GTK_MENU_PROXY_FACTORY (object);

  switch (param_id)
    {
    case PROP_MODULE_PATH:
      g_value_set_string (value, factory->module_path);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
gtk_menu_proxy_factory_set_property (GObject               *object,
                                     guint                  param_id,
                                     const GValue          *value,
                                     GParamSpec            *pspec)
{
  GtkMenuProxyFactory *factory = GTK_MENU_PROXY_FACTORY (object);

  switch (param_id)
    {
    case PROP_MODULE_PATH:
      factory->module_path = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static gboolean
is_valid_module_name (const gchar *name)
{
#if !defined(G_OS_WIN32) && !defined(G_WITH_CYGWIN)
  return g_str_has_prefix (name, "lib") && g_str_has_suffix (name, ".so");
#else
  return g_str_has_suffix (name, ".dll");
#endif
}

#define MODULE_PATH   "/usr/lib/gtk-2.0/2.10.0/menuproxies"

static gboolean
gtk_menu_proxy_factory_query_modules (GtkMenuProxyFactory *factory)
{
  const gchar *name;
  GDir        *dir;
  GError      *error = NULL;

  g_print (" ********* query_modules!!!\n");

  dir = g_dir_open (MODULE_PATH, 0, &error);

  if (!dir)
    {
      g_printerr ("Error while opening module dir: %s\n", error->message);
      g_clear_error (&error);

      return FALSE;
    }

  while ((name = g_dir_read_name (dir)))
    {
      if (is_valid_module_name (name))
        {
          GtkMenuProxyModule *module;
          gchar              *path;

          path = g_build_filename (MODULE_PATH, name, NULL);
          module = gtk_menu_proxy_module_new (path);

          if (!g_type_module_use (G_TYPE_MODULE (module)))
            {
              g_printerr ("Failed to load module: %s\n", path);
              g_object_unref (module);
              g_free (path);

              continue;
            }

          g_free (path);

          g_type_module_unuse (G_TYPE_MODULE (module));

          factory->modules = g_list_prepend (factory->modules, module);
        }
    }

  g_dir_close (dir);

  return TRUE;
}

GtkMenuProxy*
gtk_menu_proxy_factory_get_proxy (void)
{
  GtkMenuProxyFactory *factory;
  GType *proxies = NULL;
  guint  n_proxies = 0;

  factory = g_object_new (GTK_TYPE_MENU_PROXY_FACTORY, NULL);

  proxies = g_type_children (GTK_TYPE_MENU_PROXY, &n_proxies);

  if (n_proxies > 0)
    {
      return g_object_new (proxies[0], NULL);
    }

  return NULL;
}
