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
#include "gtkmenuproxymodule.h"
#include "gtkmodules.h"

enum {
  PROP_0,
  PROP_NAME
};

static GObject  *gtk_menu_proxy_module_constructor   (GType                  type,
                                                      guint                  n_params,
                                                      GObjectConstructParam *params);
static void      gtk_menu_proxy_module_finalize      (GObject               *object);
static void      gtk_menu_proxy_module_get_property  (GObject               *object,
                                                      guint                  param_id,
                                                      GValue                *value,
                                                      GParamSpec            *pspec);
static void      gtk_menu_proxy_module_set_property  (GObject               *object,
                                                      guint                  param_id,
                                                      const GValue          *value,
                                                      GParamSpec            *pspec);
static gboolean  gtk_menu_proxy_module_load_module   (GTypeModule           *gmodule);
static void      gtk_menu_proxy_module_unload_module (GTypeModule           *gmodule);


G_DEFINE_TYPE (GtkMenuProxyModule, gtk_menu_proxy_module, G_TYPE_TYPE_MODULE);

static GtkMenuProxyModule *proxy_module_singleton = NULL;

static void
gtk_menu_proxy_module_class_init (GtkMenuProxyModuleClass *class)
{
  GObjectClass     *object_class      = G_OBJECT_CLASS (class);
  GTypeModuleClass *type_module_class = G_TYPE_MODULE_CLASS (class);

  object_class->constructor  = gtk_menu_proxy_module_constructor;
  object_class->finalize     = gtk_menu_proxy_module_finalize;
  object_class->get_property = gtk_menu_proxy_module_get_property;
  object_class->set_property = gtk_menu_proxy_module_set_property;

  type_module_class->load    = gtk_menu_proxy_module_load_module;
  type_module_class->unload  = gtk_menu_proxy_module_unload_module;

  g_object_class_install_property (object_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "Name",
                                                        "The name of the module",
                                                        NULL,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static GObject *
gtk_menu_proxy_module_constructor (GType                  type,
                                   guint                  n_params,
                                   GObjectConstructParam *params)
{
  GObject            *object;

  if (proxy_module_singleton != NULL)
    {
      object = g_object_ref (proxy_module_singleton);
    }
  else
    {
      object = G_OBJECT_CLASS (gtk_menu_proxy_module_parent_class)->constructor (type,
                                                                                 n_params,
                                                                                 params);

      proxy_module_singleton = GTK_MENU_PROXY_MODULE (object);
      g_object_add_weak_pointer (object, (gpointer) &proxy_module_singleton);
    }

  return object;
}

static void
gtk_menu_proxy_module_init (GtkMenuProxyModule *module)
{
  module->name = NULL;
  module->library  = NULL;
  module->load     = NULL;
  module->unload   = NULL;
}

static void
gtk_menu_proxy_module_finalize (GObject *object)
{
  GtkMenuProxyModule *module = GTK_MENU_PROXY_MODULE (object);

  g_free (module->name);

  G_OBJECT_CLASS (gtk_menu_proxy_module_parent_class)->finalize (object);
}

static void
gtk_menu_proxy_module_get_property (GObject    *object,
                                    guint       param_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GtkMenuProxyModule *module = GTK_MENU_PROXY_MODULE (object);

  switch (param_id)
    {
    case PROP_NAME:
      g_value_set_string (value, module->name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
gtk_menu_proxy_module_set_property (GObject      *object,
                                    guint         param_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  GtkMenuProxyModule *module = GTK_MENU_PROXY_MODULE (object);

  switch (param_id)
    {
    case PROP_NAME:
      g_free (module->name);
      module->name = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static gboolean
gtk_menu_proxy_module_load_module (GTypeModule *gmodule)
{
  GtkMenuProxyModule *module = GTK_MENU_PROXY_MODULE (gmodule);

  if (!module->name)
    {
      g_warning ("Module path not set");
      return FALSE;
    }

  module->library = g_module_open (module->name, 0);

  if (!module->library)
    {
      g_printerr ("%s\n", g_module_error ());
      return FALSE;
    }

  /* Make sure that the loaded library contains the required methods */
  if (!g_module_symbol (module->library,
                        "gtk_menu_proxy_module_load",
                        (gpointer *) &module->load) ||
      !g_module_symbol (module->library,
                        "gtk_menu_proxy_module_unload",
                        (gpointer *) &module->unload))
    {
      g_printerr ("%s\n", g_module_error ());
      g_module_close (module->library);

      return FALSE;
    }

  /* Initialize the loaded module */
  module->load (module);

  return TRUE;
}

static void
gtk_menu_proxy_module_unload_module (GTypeModule *gmodule)
{
  GtkMenuProxyModule *module = GTK_MENU_PROXY_MODULE (gmodule);

  module->unload (module);

  g_module_close (module->library);
  module->library = NULL;

  module->load   = NULL;
  module->unload = NULL;
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

GtkMenuProxyModule *
gtk_menu_proxy_module_get (void)
{
  GtkMenuProxyModule *module = NULL;
  const gchar *module_name;

  module_name = g_getenv ("GTK_MENUPROXY");

  if (module_name != NULL)
    {
      if (is_valid_module_name (module_name))
        {
          gchar *path = _gtk_find_module (module_name, "menuproxies");

          module = g_object_new (GTK_TYPE_MENU_PROXY_MODULE,
                                 "name", path,
                                 NULL);

          if (!g_type_module_use (G_TYPE_MODULE (module)))
            {
              g_warning ("Failed to load type module: %s\n", path);

              g_object_unref (module);
              g_free (path);

              return NULL;
            }

          g_free (path);
          g_type_module_unuse (G_TYPE_MODULE (module));

          proxy_module_singleton = module;
        }
    }

  return module;
}
