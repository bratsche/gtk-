#include <gtk/gtk.h>

#define N_TABS 4
#define TABS_HEIGHT 20

static gint active_tab = -1;

static void
unset_prev_tab (GtkWidget       *widget,
                GtkStyleContext *context)
{
  if (active_tab < 0)
    return;

  gtk_style_context_modify_state (context,
                                  widget,
                                  GINT_TO_POINTER (active_tab),
                                  GTK_WIDGET_STATE_PRELIGHT,
                                  FALSE);
  active_tab = -1;
}

static gboolean
on_leave_notify_event (GtkWidget        *widget,
                       GdkEventCrossing *event,
                       gpointer          user_data)
{
  GtkStyleContext *context;

  context = gtk_widget_get_style_context (widget);
  unset_prev_tab (widget, context);

  return TRUE;
}

static gboolean
on_motion_notify_event (GtkWidget      *widget,
                        GdkEventMotion *event,
                        gpointer        user_data)
{
  GtkStyleContext *context;
  gint tabs_width, n_tab;

  context = gtk_widget_get_style_context (widget);


  /* Ignore motion below "tabs" */
  if (event->y > TABS_HEIGHT)
    {
      unset_prev_tab (widget, context);
      return FALSE;
    }

  /* get "tab" below pointer */
  tabs_width = widget->allocation.width / N_TABS;

  n_tab = (gint) event->x / tabs_width;

  if (n_tab != active_tab)
    {
      unset_prev_tab (widget, context);

      gtk_style_context_modify_state (context,
                                      widget,
                                      GINT_TO_POINTER (n_tab),
                                      GTK_WIDGET_STATE_PRELIGHT,
                                      TRUE);
      active_tab = n_tab;
    }

  return TRUE;
}

static gboolean
on_expose_event (GtkWidget      *widget,
                 GdkEventExpose *event,
                 gpointer        user_data)
{
  GtkStyleContext *context;
  GtkPlacingContext placing;
  cairo_t *cr;
  gint tab_width, tab_x, i;

  cr = gdk_cairo_create (widget->window);
  context = gtk_widget_get_style_context (widget);
  tab_width = widget->allocation.width / N_TABS;
  tab_x = 0;

  /* Set radius for box borders */
  gtk_style_context_set_param_int (context, "radius", 15);

  gtk_style_context_save (context);
  gtk_style_context_set_placing_context (context, GTK_PLACING_CONNECTS_UP);

  gtk_depict_box (context, cr,
                  widget->allocation.x,
                  widget->allocation.y + TABS_HEIGHT,
                  widget->allocation.width,
                  widget->allocation.height - TABS_HEIGHT);

  gtk_style_context_restore (context);

  gtk_style_context_set_placing_context (context, GTK_PLACING_CONNECTS_DOWN);

  for (i = 0; i < N_TABS; i++)
    {
      gtk_style_context_save (context);
      gtk_style_context_push_activatable_region (context, GINT_TO_POINTER (i));

      /* Make tabs connect with the others sensibly */
      placing = gtk_style_context_get_placing_context (context);

      if (i > 0)
        placing |= GTK_PLACING_CONNECTS_LEFT;
      if (i < N_TABS - 1)
        placing |= GTK_PLACING_CONNECTS_RIGHT;

      gtk_style_context_set_placing_context (context, placing);

      /* Set active if pointer is on this "tab" */
      if (i == active_tab)
        gtk_style_context_set_state_flags (context, GTK_WIDGET_STATE_PRELIGHT);

      gtk_depict_box (context, cr,
                      tab_x,
                      widget->allocation.y,
                      tab_width,
                      TABS_HEIGHT);

      tab_x += tab_width;

      gtk_style_context_pop_activatable_region (context);
      gtk_style_context_restore (context);
    }

  cairo_destroy (cr);

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *event_box;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request (window, 300, 300);
  /* gtk_container_set_border_width (GTK_CONTAINER (window), 12); */

  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (window), event_box);

  gtk_widget_add_events (event_box, GDK_POINTER_MOTION_MASK);

  g_signal_connect (event_box, "leave-notify-event",
                    G_CALLBACK (on_leave_notify_event), NULL);
  g_signal_connect (event_box, "motion-notify-event",
                    G_CALLBACK (on_motion_notify_event), NULL);
  g_signal_connect (event_box, "expose-event",
                    G_CALLBACK (on_expose_event), NULL);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
