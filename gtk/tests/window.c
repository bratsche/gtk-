#include <gtk/gtk.h>

static gboolean interactive = FALSE;

static gboolean
stop_main (gpointer data)
{
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

static gboolean
on_draw (GtkWidget *widget, cairo_t *cr)
{
  gint i, j;

  for (i = 0; 20 * i < gtk_widget_get_allocated_width (widget); i++)
    {
      for (j = 0; 20 * j < gtk_widget_get_allocated_height (widget); j++)
        {
          if ((i + j) % 2 == 1)
            cairo_set_source_rgb (cr, 1., 1., 1.);
          else
            cairo_set_source_rgb (cr, 0., 0., 0.);

          cairo_rectangle (cr, 20. * i, 20. *j, 20., 20.);
          cairo_fill (cr);
        }
    }

  return FALSE;
}

static gboolean
on_keypress (GtkWidget *widget)
{
  gtk_main_quit ();

  return TRUE;
}

static void
test_default_size (void)
{
  GtkWidget *window;
  GtkWidget *box;
  gint w, h;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "draw", G_CALLBACK (on_draw), NULL);
  if (interactive)
    g_signal_connect (window, "key-press-event", G_CALLBACK (on_keypress), NULL);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (window), box);

  gtk_window_get_default_size (GTK_WINDOW (window), &w, &h);
  g_assert_cmpint (w, ==, -1);
  g_assert_cmpint (h, ==, -1);

  gtk_window_set_default_size (GTK_WINDOW (window), 300, 300);

  gtk_window_get_default_size (GTK_WINDOW (window), &w, &h);
  g_assert_cmpint (w, ==, 300);
  g_assert_cmpint (h, ==, 300);

  gtk_window_get_size (GTK_WINDOW (window), &w, &h);
  g_assert_cmpint (w, ==, 300);
  g_assert_cmpint (h, ==, 300);

  gtk_widget_show_all (window);

  if (!interactive)
    g_timeout_add (200, stop_main, NULL);
  gtk_main ();

  gtk_window_get_size (GTK_WINDOW (window), &w, &h);
  g_assert_cmpint (w, ==, 300);
  g_assert_cmpint (h, ==, 300);

  g_assert_cmpint (gtk_widget_get_allocated_width (window), ==, 300);
  g_assert_cmpint (gtk_widget_get_allocated_height (window), ==, 300);

  g_assert_cmpint (gtk_widget_get_allocated_width (box), ==, 300);
  g_assert_cmpint (gtk_widget_get_allocated_height (box), ==, 300);

  gtk_widget_destroy (window);
}

static void
test_resize (void)
{
  GtkWidget *window;
  GtkWidget *box;
  gint w, h;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "draw", G_CALLBACK (on_draw), NULL);
  if (interactive)
    g_signal_connect (window, "key-press-event", G_CALLBACK (on_keypress), NULL);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (window), box);

  gtk_window_resize (GTK_WINDOW (window), 400, 200);

  gtk_widget_show_all (window);

  if (!interactive)
    g_timeout_add (200, stop_main, NULL);
  gtk_main ();

  gtk_window_get_size (GTK_WINDOW (window), &w, &h);
  g_assert_cmpint (w, ==, 400);
  g_assert_cmpint (h, ==, 200);

  gtk_window_resize (GTK_WINDOW (window), 200, 400);

  if (!interactive)
    g_timeout_add (200, stop_main, NULL);
  gtk_main ();

  gtk_window_get_size (GTK_WINDOW (window), &w, &h);
  g_assert_cmpint (w, ==, 200);
  g_assert_cmpint (h, ==, 400);

  gtk_widget_destroy (window);
}

int
main (int argc, char *argv[])
{
  gint i;

  gtk_test_init (&argc, &argv);

  for (i = 0; i < argc; i++)
    {
      if (g_strcmp0 (argv[i], "--interactive") == 0)
        interactive = TRUE;
    }

  g_test_add_func ("/window/default-size", test_default_size);
  g_test_add_func ("/window/resize", test_resize);

  return g_test_run ();
}