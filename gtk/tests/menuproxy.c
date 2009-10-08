#undef GTK_DISABLE_DEPRECATED
#include "../gtk/gtk.h"

static void
proxy_tests (void)
{
  gtk_menu_proxy = (GtkMenuProxyIface *)0x1;

  GtkWidget *widget = g_object_new (GTK_TYPE_MENU_BAR, NULL);

  g_assert (GTK_IS_MENU_BAR (widget));

  g_assert (GTK_MENU_BAR (widget)->proxy == gtk_menu_proxy);
}

static void
null_proxy_test (void)
{
  gtk_menu_proxy = NULL;

  GtkWidget *widget = g_object_new (GTK_TYPE_MENU_BAR, NULL);

  g_assert (GTK_IS_MENU_BAR (widget));

  g_assert (GTK_MENU_BAR (widget)->proxy == gtk_menu_proxy);
}

static void
proxy_type_exists (void)
{
  g_assert (gtk_menu_proxy_get_type () != 0);
}

int
main (int argc, char *argv[])
{
  gtk_test_init (&argc, &argv);

  g_test_add_func ("/proxy/type-exists", proxy_type_exists);
  g_test_add_func ("/proxy", proxy_tests);
  g_test_add_func ("/proxy/null-proxy", null_proxy_test);

  return g_test_run();
}
