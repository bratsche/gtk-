#include <gtk/gtk.h>

static gboolean
on_enter_notify_event (GtkWidget        *widget,
                       GdkEventCrossing *event,
                       gpointer          user_data)
{
  GtkStyleContext *context;

  context = gtk_widget_get_style_context (widget);

  gtk_style_context_set_state_flags (context, GTK_WIDGET_STATE_PRELIGHT);
  gtk_style_context_modify_state (context,
                                  widget,
                                  widget,
                                  GTK_WIDGET_STATE_PRELIGHT,
                                  TRUE);
  return TRUE;
}

static gboolean
on_leave_notify_event (GtkWidget        *widget,
                       GdkEventCrossing *event,
                       gpointer          user_data)
{
  GtkStyleContext *context;

  context = gtk_widget_get_style_context (widget);

  gtk_style_context_unset_state_flags (context, GTK_WIDGET_STATE_PRELIGHT);
  gtk_style_context_modify_state (context,
                                  widget,
                                  widget,
                                  GTK_WIDGET_STATE_PRELIGHT,
                                  FALSE);
  return TRUE;
}

static gboolean
on_expose_event (GtkWidget      *widget,
                 GdkEventExpose *event,
                 gpointer        user_data)
{
  GtkStyleContext *context;
  cairo_t *cr;

  cr = gdk_cairo_create (widget->window);
  context = gtk_widget_get_style_context (widget);

  gtk_style_context_push_activatable_region (context, widget);

  /* Set radius for box borders */
  gtk_style_context_set_param_int (context, "radius", 15);

  gtk_depict_box (context, cr,
                  widget->allocation.x,
                  widget->allocation.y,
                  widget->allocation.width,
                  widget->allocation.height);

  gtk_style_context_pop_activatable_region (context);

  cairo_destroy (cr);

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *event_box;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  /* gtk_container_set_border_width (GTK_CONTAINER (window), 12); */

  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (window), event_box);

  g_signal_connect (event_box, "enter-notify-event",
                    G_CALLBACK (on_enter_notify_event), NULL);
  g_signal_connect (event_box, "leave-notify-event",
                    G_CALLBACK (on_leave_notify_event), NULL);
  g_signal_connect (event_box, "expose-event",
                    G_CALLBACK (on_expose_event), NULL);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
