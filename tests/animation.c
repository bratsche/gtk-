#include "config.h"
#include <gtk/gtk.h>

static GtkTimeline *timeline = NULL;

static gdouble rotate       = 0.0;
static gdouble translate[2] = { 1., 1. };
static gdouble scale        = 1.0;

static void
frame_cb (GtkTimeline *timeline,
	  gdouble      progress,
	  gpointer     user_data)
{
  GtkWidget *da = (GtkWidget *)user_data;

  translate[0] = 400.0 * progress;
  translate[1] = 400.0 * progress;
  rotate = 180.0 * progress;

  gtk_widget_queue_draw (da);
}

static void
finish_cb (GtkTimeline *timeline,
	   gpointer     user_data)
{
  gtk_timeline_set_direction (timeline,
			      !gtk_timeline_get_direction (timeline));
}

static void
clicked (GtkWidget *widget,
	 gpointer   user_data)
{
  GtkWidget *da = (GtkWidget *)user_data;

  if (timeline == NULL)
    {
      timeline = gtk_timeline_new (1000000);
      gtk_timeline_set_transition_func (timeline,
					gtk_transition_sinusoidal);

      g_signal_connect (timeline,
			"frame",
			G_CALLBACK (frame_cb),
			da);

      g_signal_connect (timeline,
			"finish",
			G_CALLBACK (finish_cb),
			NULL);
    }

  if (gtk_timeline_is_running (timeline))
    {
      gtk_timeline_stop (timeline);
    }
  else
    {
      gtk_timeline_reset (timeline);
      gtk_timeline_start (timeline);
    }
}

static gboolean
draw (GtkWidget *widget,
      cairo_t   *cr,
      gpointer   data)
{
  gdouble radians;
  gint width = 10;

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_width (cr, width);

  radians = rotate * (G_PI / 180);
  cairo_translate (cr, translate[0], translate[1]);
  cairo_scale (cr, scale, scale);
  cairo_rotate (cr, radians);

  cairo_rectangle (cr, 0, 0, 100, 100);
  cairo_stroke_preserve (cr);
  cairo_set_source_rgb (cr, 1, 0, 1);
  cairo_fill (cr);

  return FALSE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *da;
  GtkWidget *button;

  /* FIXME: This is not allowable - what is this supposed to be? */
  /*  gdk_progclass = g_strdup ("XTerm"); */
  gtk_init (&argc, &argv);
  
  window = g_object_connect (g_object_new (gtk_window_get_type (),
					   "type", GTK_WINDOW_TOPLEVEL,
					   "title", "Animate Me!",
					   "resizable", FALSE,
					   "border_width", 10,
					   NULL),
			     "signal::destroy", gtk_main_quit, NULL,
			     NULL);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  da = g_object_connect (g_object_new (gtk_drawing_area_get_type (),
				       "GtkWidget::visible", TRUE,
				       NULL),
			 "signal::draw", draw, NULL,
			 NULL);
  gtk_widget_set_size_request (da, 400, 400);
  gtk_box_pack_start (GTK_BOX (vbox), da, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Start");
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  g_signal_connect (button, "clicked", G_CALLBACK (clicked), da);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
