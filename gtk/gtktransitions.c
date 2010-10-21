#include <math.h>

#include "gtktransitions.h"

gdouble
gtk_transition_sinusoidal (gdouble position)
{
  return sinf ((position * G_PI) / 2);
}
