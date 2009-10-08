#include "config.h"
#include "gtkintl.h"
#include "gtkmenuproxy.h"

static GtkMenuProxyIface *singleton = NULL;

GType
gtk_menu_proxy_get_type (void)
{
  static GType menu_proxy_type = 0;

  if (!menu_proxy_type)
    {
      menu_proxy_type = g_type_register_static_simple (G_TYPE_INTERFACE, I_("GtkMenuProxy"),
                                                       sizeof (GtkMenuProxyIface),
                                                       NULL, 0, NULL, 0);
    }

  return menu_proxy_type;
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
