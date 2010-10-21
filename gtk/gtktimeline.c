#include "config.h"
#include <gdk/gdk.h>
#include "gtktimeline.h"

struct _GtkTimelinePrivate
{
  guint             length;
  guint             id;
  guint64           start_time;

  GtkDirection      direction;
  GtkTransitionFunc transition;
};

enum {
  PROP_0,
  PROP_LENGTH
};

enum {
  START,
  FINISH,
  FRAME,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

#define GTK_TIMELINE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_TIMELINE, GtkTimelinePrivate))

G_DEFINE_TYPE (GtkTimeline, gtk_timeline, G_TYPE_OBJECT)

static void
gtk_timeline_finalize (GObject *object)
{
  GtkTimelinePrivate *priv = GTK_TIMELINE (object)->priv;

  gdk_threads_periodic_remove (priv->id);
  priv->id = 0;
}

static void
gtk_timeline_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
  GtkTimeline *timeline = GTK_TIMELINE (object);

  switch (prop_id)
    {
    case PROP_LENGTH:
      gtk_timeline_set_length (timeline, g_value_get_uint (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_timeline_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
  GtkTimelinePrivate *priv = GTK_TIMELINE (object)->priv;

  switch (prop_id)
    {
    case PROP_LENGTH:
      g_value_set_uint (value, priv->length);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_timeline_class_init (GtkTimelineClass *c)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (c);

  gobject_class->get_property = gtk_timeline_get_property;
  gobject_class->set_property = gtk_timeline_set_property;
  gobject_class->finalize     = gtk_timeline_finalize;

  g_object_class_install_property (gobject_class,
				   PROP_LENGTH,
				   g_param_spec_uint ("length",
						      "Length",
						      "Length",
						      0, G_MAXUINT,
						      0,
						      G_PARAM_READWRITE));

  signals[FRAME] = g_signal_new ("frame",
				 G_TYPE_FROM_CLASS (gobject_class),
				 G_SIGNAL_RUN_LAST,
				 0,
				 NULL, NULL,
				 g_cclosure_marshal_VOID__DOUBLE,
				 G_TYPE_NONE, 1,
				 G_TYPE_DOUBLE);

  signals[START] = g_signal_new ("start",
				 G_TYPE_FROM_CLASS (gobject_class),
				 G_SIGNAL_RUN_LAST,
				 0,
				 NULL, NULL,
				 g_cclosure_marshal_VOID__VOID,
				 G_TYPE_NONE, 0);

  signals[FINISH] = g_signal_new ("finish",
				  G_TYPE_FROM_CLASS (gobject_class),
				  G_SIGNAL_RUN_LAST,
				  0,
				  NULL, NULL,
				  g_cclosure_marshal_VOID__VOID,
				  G_TYPE_NONE, 0);

  g_type_class_add_private (c, sizeof (GtkTimelinePrivate));
}

static void
gtk_timeline_init (GtkTimeline *timeline)
{
  GtkTimelinePrivate *priv = GTK_TIMELINE_GET_PRIVATE (timeline);

  timeline->priv = priv;

  priv->id = 0;
  priv->length  = 0.0;
}

static void
gtk_timeline_tick (GPeriodic *periodic,
		   guint64    timestamp,
		   gpointer   user_data)
{
  GtkTimeline *timeline = GTK_TIMELINE (user_data);
  GtkTimelinePrivate *priv = timeline->priv;
  gdouble progress;
  gdouble goal;

  goal = priv->direction == GTK_DIRECTION_REVERSE ? 0.0 : 1.0;

  progress = CLAMP (((gdouble)(timestamp - priv->start_time)) / priv->length, 0., 1.);

  if (priv->direction == GTK_DIRECTION_REVERSE)
    progress = 1.0 - progress;

  if (priv->transition)
    progress = priv->transition (progress);

  g_signal_emit (timeline, signals[FRAME], 0, progress);

  if (progress == goal)
    {
      gtk_timeline_stop (timeline);
      g_signal_emit (timeline, signals[FINISH], 0);
    }
}

void
gtk_timeline_start (GtkTimeline *timeline)
{
  GtkTimelinePrivate *priv;

  g_return_if_fail (GTK_IS_TIMELINE (timeline));
  g_return_if_fail (timeline->priv->id == 0);

  priv = timeline->priv;

  priv->id = gdk_threads_periodic_add (gtk_timeline_tick, timeline, NULL);
}

void
gtk_timeline_stop (GtkTimeline *timeline)
{
  GtkTimelinePrivate *priv;

  g_return_if_fail (GTK_IS_TIMELINE (timeline));
  g_return_if_fail (timeline->priv->id != 0);

  priv = timeline->priv;

  gdk_threads_periodic_remove (priv->id);
  priv->id = 0;
}

void
gtk_timeline_reset (GtkTimeline *timeline)
{
  GtkTimelinePrivate *priv;
  GDateTime *dt;
  GTimeVal timeval;

  g_return_if_fail (GTK_IS_TIMELINE (timeline));
  g_return_if_fail (timeline->priv->id == 0);

  priv = timeline->priv;

  dt = g_date_time_new_now_local ();
  g_date_time_to_timeval (dt, &timeval);

  priv->start_time = (timeval.tv_sec * 1000000 + timeval.tv_usec);

  g_date_time_unref (dt);
}

GtkTimeline *
gtk_timeline_new (guint length)
{
  GtkTimeline *timeline = g_object_new (GTK_TYPE_TIMELINE,
					"length", length,
					NULL);

  gtk_timeline_reset (timeline);

  return timeline;
}

gboolean
gtk_timeline_is_running (GtkTimeline *timeline)
{
  g_return_val_if_fail (GTK_IS_TIMELINE (timeline), FALSE);

  return timeline->priv->id != 0;
}

void
gtk_timeline_set_length (GtkTimeline *timeline,
			 guint        length)
{
  GtkTimelinePrivate *priv;

  g_return_if_fail (GTK_IS_TIMELINE (timeline));

  priv = timeline->priv;

  if (length != priv->length)
    {
      priv->length = length;
      g_object_notify (G_OBJECT (timeline), "length");
    }
}

guint
gtk_timeline_get_length (GtkTimeline *timeline)
{
  g_return_val_if_fail (GTK_IS_TIMELINE (timeline), 0);

  return timeline->priv->length;
}

void
gtk_timeline_set_direction (GtkTimeline  *timeline,
			    GtkDirection  direction)
{
  g_return_if_fail (GTK_IS_TIMELINE (timeline));
  g_return_if_fail (timeline->priv->id == 0);

  timeline->priv->direction = direction;
}

GtkDirection
gtk_timeline_get_direction (GtkTimeline *timeline)
{
  g_return_val_if_fail (GTK_IS_TIMELINE (timeline), GTK_DIRECTION_FORWARD);

  return timeline->priv->direction;
}

void
gtk_timeline_set_transition_func (GtkTimeline       *timeline,
				  GtkTransitionFunc  func)
{
  g_return_if_fail (GTK_IS_TIMELINE (timeline));
  g_return_if_fail (timeline->priv->id == 0);

  timeline->priv->transition = func;
}
