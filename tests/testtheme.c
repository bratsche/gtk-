#include <gtk/gtk.h>

#define BORDER 5
#define N_COLS 3
#define N_ROWS 3

static gboolean
on_expose_event (GtkWidget      *widget,
                 GdkEventExpose *event,
                 gpointer        user_data)
{
  GtkStyleContext *context;
  gint x, y, width, height;
  gint r, c;
  cairo_t *cr;

  cr = gdk_cairo_create (widget->window);
  context = gtk_widget_get_style_context (widget);

  /* Set radius for box borders */
  gtk_style_context_set_param_int (context, "radius", 15);

  width = (widget->allocation.width - 2 * BORDER) / N_COLS;
  height = (widget->allocation.height - 2 * BORDER) / N_ROWS;
  x = y = BORDER;

  for (r = 0; r < N_ROWS; r++)
    {
      GtkPlacingContext placing = 0;

      /* Specify where will the row connect with others */
      if (r > 0)
        placing |= GTK_PLACING_CONNECTS_UP;

      if (r < N_ROWS - 1)
        placing |= GTK_PLACING_CONNECTS_DOWN;

      gtk_style_context_set_placing_context (context, placing);

      for (c = 0; c < N_COLS; c++)
        {
          /* Save context so we can restore later the previous placing */
          gtk_style_context_save (context);

          placing = gtk_style_context_get_placing_context (context);

          /* Specify where will the column connect with others */
          if (c > 0)
            placing |= GTK_PLACING_CONNECTS_LEFT;

          if (c < N_COLS - 1)
            placing |= GTK_PLACING_CONNECTS_RIGHT;

          gtk_style_context_set_placing_context (context, placing);

          if ((r + c) % 2)
            {
              GdkColor color = { 0, 65535, 65535, 65535 };
              gtk_style_context_set_bg_color (context, 0, &color);
            }

          /* Paint box, GtkStyleContext will use the placing
           * info to know which are the rounded corners in
           * the box group.
           */
          gtk_depict_box (context, cr, x, y, width, height);

          x += width;

          /* Restore context to reset placing to
           * the previous value and unset color
           */
          gtk_style_context_restore (context);
        }

      y += height;
      x = BORDER;
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
  event_box = gtk_event_box_new ();

  gtk_container_add (GTK_CONTAINER (window), event_box);

  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (event_box, "expose-event",
                    G_CALLBACK (on_expose_event), NULL);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
