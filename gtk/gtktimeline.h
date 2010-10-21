#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#ifndef __GTK_TIMELINE_H__
#define __GTK_TIMELINE_H__

#include <glib-object.h>
#include "gtkenums.h"

G_BEGIN_DECLS

#define GTK_TYPE_TIMELINE         (gtk_timeline_get_type ())
#define GTK_TIMELINE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_TIMELINE, GtkTimeline))
#define GTK_TIMELINE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), GTK_TYPE_TIMELINE, GtkTimelineClass))
#define GTK_IS_TIMELINE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_TIMELINE))
#define GTK_IS_TIMELINE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_TYPE_TIMELINE))
#define GTK_TIMELINE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GTK_TYPE_TIMELINE, GtkTimelineClass))

typedef struct _GtkTimeline        GtkTimeline;
typedef struct _GtkTimelinePrivate GtkTimelinePrivate;
typedef struct _GtkTimelineClass   GtkTimelineClass;

typedef gdouble (* GtkTransitionFunc) (gdouble position);

struct _GtkTimeline
{
  GObject parent_instance;

  GtkTimelinePrivate *priv;
};

struct _GtkTimelineClass
{
  GObjectClass parent_class;
};

GType        gtk_timeline_get_type      (void) G_GNUC_CONST;

GtkTimeline *gtk_timeline_new           (guint         length);

guint        gtk_timeline_get_length    (GtkTimeline  *timeline);
void         gtk_timeline_set_length    (GtkTimeline  *timeline,
					 guint         length);
GtkDirection gtk_timeline_get_direction (GtkTimeline  *timeline);
void         gtk_timeline_set_direction (GtkTimeline  *timeline,
					 GtkDirection  direction);

void         gtk_timeline_set_transition_func (GtkTimeline       *timeline,
					       GtkTransitionFunc  func);

void         gtk_timeline_start         (GtkTimeline    *timeline);
void         gtk_timeline_stop          (GtkTimeline    *timeline);
void         gtk_timeline_reset         (GtkTimeline    *timeline);

gboolean     gtk_timeline_is_running    (GtkTimeline    *timeline);

G_END_DECLS

#endif /* __GTK_TIMELINE_H__ */
