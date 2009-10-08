#undef GTK_DISABLE_DEPRECATED
#include "../gtk/gtk.h"

static GtkMenuProxyIface *parent_iface;

typedef struct _TestProxy      TestProxy;
typedef struct _TestProxyClass TestProxyClass;

struct _TestProxy
{
  GObject parent_object;
};

struct _TestProxyClass
{
  GObjectClass parent_class;
};

static void test_proxy_interface_init (GtkMenuProxyIface *iface);

G_DEFINE_TYPE_WITH_CODE (TestProxy, test_proxy,  g_object_get_type (),
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_MENU_PROXY,
                                                test_proxy_interface_init))

static void
test_proxy_init (TestProxy *proxy)
{
}

static void
test_proxy_class_init (TestProxyClass *class)
{
}

static void
test_proxy_interface_init (GtkMenuProxyIface *iface)
{
  parent_iface = g_type_interface_peek_parent (iface);
}

static void
non_null_proxy_test (void)
{
  gtk_menu_proxy_register_type (test_proxy_get_type ());

  GtkWidget *widget = g_object_new (GTK_TYPE_MENU_BAR, NULL);
  g_object_ref_sink (widget);

  g_assert (GTK_IS_MENU_BAR (widget));

  g_assert (GTK_MENU_BAR (widget)->proxy != NULL);

  g_object_unref (widget);
}

static void
null_proxy_test (void)
{
  GtkWidget *widget = g_object_new (GTK_TYPE_MENU_BAR, NULL);
  g_object_ref_sink (widget);

  g_assert (GTK_IS_MENU_BAR (widget));

  g_assert (GTK_MENU_BAR (widget)->proxy == NULL);

  g_object_unref (widget);
}

static void
proxy_type_exists (void)
{
  g_assert (gtk_menu_proxy_get_type () != 0);
}

static void
can_instantiate_test (void)
{
  GtkMenuProxyIface *proxy = g_object_new (test_proxy_get_type (), NULL);
  g_object_ref_sink (proxy);

  g_assert (proxy != NULL);

  G_TYPE_CHECK_INSTANCE_TYPE ((proxy), test_proxy_get_type ());

  g_object_unref (proxy);
}

int
main (int argc, char *argv[])
{
  gtk_test_init (&argc, &argv);

  g_test_add_func ("/proxy/type-exists", proxy_type_exists);
  g_test_add_func ("/proxy/can-instantiate", can_instantiate_test);
  g_test_add_func ("/proxy/null-proxy", null_proxy_test);
  g_test_add_func ("/proxy/non-null-proxy", non_null_proxy_test);

  return g_test_run();
}
