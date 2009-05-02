#include "config.h"
#include <gtk/gtk.h>

void
hello (void)
{
  g_print ("hello world\n");
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *button;

  gtk_init (&argc, &argv);

  window = g_object_connect (g_object_new (gtk_window_get_type (),
					     "user_data", NULL,
					     "type", GTK_WINDOW_TOPLEVEL,
					     "title", "hello world",
					     "allow_grow", FALSE,
					     "allow_shrink", FALSE,
					     "border_width", 10,
					     NULL),
			     "signal::destroy", gtk_main_quit, NULL,
			     NULL);
  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  label = gtk_label_new ("There's a lady who's sure\n"
                         "All that glitters is gold\n"
                         "And she's buying a stairway to Heaven.");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  button = g_object_connect (g_object_new (gtk_button_get_type (),
					     "GtkButton::label", "Stairway",
					     "GtkWidget::visible", TRUE,
					     NULL),
			     "signal::clicked", hello, NULL,
			     NULL);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
