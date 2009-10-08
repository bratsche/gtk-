#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#ifndef __GTK_MENU_PROXY_H__
#define __GTK_MENU_PROXY_H__

#include <gtk/gtkmenushell.h>
#include <gtk/gtktypeutils.h>

G_BEGIN_DECLS

#define GTK_TYPE_MENU_PROXY          (gtk_menu_proxy_get_type ())
#define GTK_MENU_PROXY(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_MENU_PROXY, GtkMenuProxy))
#define GTK_MENU_PROXY_CLASS(k)      (G_TYPE_CHECK_CLASS_CAST ((k), GTK_TYPE_MENU_PROXY, GtkMenuProxyIface))
#define GTK_IS_MENU_PROXY(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_MENU_PROXY))
#define GTK_MENU_PROXY_GET_IFACE(o)  (G_TYPE_INSTANCE_GET_INTERFACE ((o), GTK_TYPE_MENU_PROXY, GtkMenuProxyIface))

typedef struct _GtkMenuProxy      GtkMenuProxy; /* Dummy typedef */
typedef struct _GtkMenuProxyIface GtkMenuProxyIface;

struct _GtkMenuProxyIface
{
  GTypeInterface g_iface;
};

GType              gtk_menu_proxy_get_type      (void) G_GNUC_CONST;
GtkMenuProxyIface *gtk_menu_proxy_get           (void);
void               gtk_menu_proxy_register_type (GType type);

G_END_DECLS

#endif /* __GTK_BUILDABLE_H__ */
