/*
 * GTK - The GIMP Toolkit
 * Copyright (C) 2009  Carlos Garnacho Parro <carlosg@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "gtkstylecontext.h"

#define GTK_STYLE_CONTEXT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_STYLE_CONTEXT, GtkStyleContextPrivate))
#define GET_CURRENT_CONTEXT(p) ((GHashTable *) (p)->context_stack->data)

typedef struct GtkStyleContextPrivate GtkStyleContextPrivate;

struct GtkStyleContextPrivate
{
  GHashTable *composed_context;
  GList *context_stack;
  GList *reverse_context_stack;
};


static void gtk_style_context_finalize (GObject *object);


G_DEFINE_TYPE (GtkStyleContext, gtk_style_context, G_TYPE_OBJECT)

static void
gtk_style_context_class_init (GtkStyleContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtk_style_context_finalize;

  g_type_class_add_private (klass, sizeof (GtkStyleContextPrivate));
}

static GHashTable *
create_subcontext (void)
{
  return g_hash_table_new_full (g_str_hash,
                                g_str_equal,
                                (GDestroyNotify) g_free,
                                NULL);
}

static void
gtk_style_context_init (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  priv->composed_context = g_hash_table_new (g_str_hash, g_str_equal);
  priv->reverse_context_stack = priv->context_stack = g_list_prepend (priv->context_stack, create_subcontext ());
}

static void
gtk_style_context_finalize (GObject *object)
{
  GtkStyleContextPrivate *priv;

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (object);

  g_list_foreach (priv->context_stack, (GFunc) g_hash_table_destroy, NULL);
  g_list_free (priv->context_stack);

  g_hash_table_destroy (priv->composed_context);

  G_OBJECT_CLASS (gtk_style_context_parent_class)->finalize (object);
}

void
gtk_style_context_save (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  /* Create a new subcontext for new changes */
  priv->context_stack = g_list_prepend (priv->context_stack, create_subcontext ());
}

static void
set_context_param (gpointer    key,
                   gpointer    value,
                   GHashTable *context)
{
  g_hash_table_insert (context, key, value);
}

static void
recalculate_composed_context (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;
  GList *elem;

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  g_hash_table_remove_all (priv->composed_context);

  for (elem = priv->reverse_context_stack; elem; elem = elem->prev)
    {
      GHashTable *subcontext;

      subcontext = elem->data;

      g_hash_table_foreach (subcontext, (GHFunc) set_context_param, priv->composed_context);
    }
}

void
gtk_style_context_restore (GtkStyleContext *context)
{
  GtkStyleContextPrivate *priv;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  if (priv->context_stack)
    {
      GList *elem;

      elem = priv->context_stack;
      priv->context_stack = g_list_remove_link (priv->context_stack, elem);

      g_hash_table_destroy (elem->data);
      g_list_free_1 (elem);
    }

  if (!priv->context_stack)
    {
      /* Restore an empty subcontext */
      priv->reverse_context_stack = priv->context_stack = g_list_prepend (priv->context_stack, create_subcontext ());
    }

  recalculate_composed_context (context);
}

void
gtk_style_context_add_param (GtkStyleContext *context,
                             const gchar     *param)
{
  GtkStyleContextPrivate *priv;
  gchar *str;

  g_return_if_fail (GTK_IS_STYLE_CONTEXT (context));
  g_return_if_fail (param != NULL);

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  str = g_strdup (param);

  /* Insert in current context */
  g_hash_table_insert (GET_CURRENT_CONTEXT (priv), str, GINT_TO_POINTER (TRUE));

  /* And in composed context */
  g_hash_table_insert (priv->composed_context, str, GINT_TO_POINTER (TRUE));
}

gboolean
gtk_style_context_param_exists (GtkStyleContext *context,
                                const gchar     *param)
{
  GtkStyleContextPrivate *priv;

  g_return_val_if_fail (GTK_IS_STYLE_CONTEXT (context), FALSE);
  g_return_val_if_fail (param != NULL, FALSE);

  priv = GTK_STYLE_CONTEXT_GET_PRIVATE (context);

  return (g_hash_table_lookup (priv->composed_context, param) != NULL);
}

#define __GTK_STYLE_CONTEXT_C__
#include "gtkaliasdef.c"
