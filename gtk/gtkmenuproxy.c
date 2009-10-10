#include "config.h"
#include "gtkintl.h"
#include "gtkmarshal.h"
#include "gtkmenuproxy.h"

static void gtk_menu_proxy_base_init (gpointer g_class);

static GtkMenuProxyIface *singleton = NULL;

GType
gtk_menu_proxy_get_type (void)
{
  static GType menu_proxy_type = 0;

  if (!menu_proxy_type)
    {
      const GTypeInfo menu_proxy_info =
        {
          sizeof (GtkMenuProxyIface), /* class_size */
          gtk_menu_proxy_base_init,   /* base_init */
          NULL,                       /* base_finalize */
          NULL,
          NULL,                       /* class_finalize */
          NULL,                       /* class_data */
          0,
          0,
          NULL
        };

      menu_proxy_type = g_type_register_static (G_TYPE_INTERFACE, I_("GtkMenuProxy"),
                                                &menu_proxy_info, 0);
    }

  return menu_proxy_type;
}

static void
gtk_menu_proxy_base_init (gpointer g_class)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      initialized = TRUE;

      g_signal_new (I_("inserted"),
                    GTK_TYPE_MENU_PROXY,
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkMenuProxyIface, inserted),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER,
                    G_TYPE_NONE, 1,
                    GTK_TYPE_WIDGET);
    }
}

GtkMenuProxyIface *
gtk_menu_proxy_get (void)
{
  return singleton;
}

void
gtk_menu_proxy_register_type (GType type)
{
  singleton = g_object_new (type, NULL);
}

void
gtk_menu_proxy_insert (GtkMenuProxy *proxy,
                       GtkWidget    *child,
                       guint         position)
{
  g_return_if_fail (GTK_IS_MENU_PROXY (proxy));

  (* GTK_MENU_PROXY_GET_IFACE (proxy)->insert) (proxy, child, position);

  g_signal_emit_by_name (proxy, "inserted", child);
}
