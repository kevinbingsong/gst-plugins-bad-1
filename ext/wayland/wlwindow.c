/* GStreamer Wayland video sink
 *
 * Copyright (C) 2011 Intel Corporation
 * Copyright (C) 2011 Sreerenj Balachandran <sreerenj.balachandran@intel.com>
 * Copyright (C) 2014 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "wlwindow.h"

G_DEFINE_TYPE (GstWlWindow, gst_wl_window, G_TYPE_OBJECT);

static void gst_wl_window_finalize (GObject * gobject);

static void
handle_ping (void *data, struct wl_shell_surface *shell_surface,
    uint32_t serial)
{
  wl_shell_surface_pong (shell_surface, serial);
}

static void
handle_configure (void *data, struct wl_shell_surface *shell_surface,
    uint32_t edges, int32_t width, int32_t height)
{
}

static void
handle_popup_done (void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
  handle_ping,
  handle_configure,
  handle_popup_done
};

static void
gst_wl_window_class_init (GstWlWindowClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = gst_wl_window_finalize;
}

static void
gst_wl_window_init (GstWlWindow * self)
{
}

static void
gst_wl_window_finalize (GObject * gobject)
{
  GstWlWindow *self = GST_WL_WINDOW (gobject);

  if (self->shell_surface) {
    wl_shell_surface_destroy (self->shell_surface);
  }

  if (self->own_surface) {
    wl_surface_destroy (self->surface);
  }

  g_clear_object (&self->display);

  G_OBJECT_CLASS (gst_wl_window_parent_class)->finalize (gobject);
}

GstWlWindow *
gst_wl_window_new_toplevel (GstWlDisplay * display, gint width, gint height)
{
  GstWlWindow *window;

  window = g_object_new (GST_TYPE_WL_WINDOW, NULL);
  window->display = g_object_ref (display);
  window->width = width;
  window->height = height;

  window->surface = wl_compositor_create_surface (display->compositor);
  window->own_surface = TRUE;

  window->shell_surface = wl_shell_get_shell_surface (display->shell,
      window->surface);

  if (window->shell_surface) {
    wl_shell_surface_add_listener (window->shell_surface,
        &shell_surface_listener, window);
    wl_shell_surface_set_toplevel (window->shell_surface);
  } else {
    GST_ERROR ("Unable to get wl_shell_surface");

    g_object_unref (window);
    return NULL;
  }

  return window;
}

GstWlWindow *
gst_wl_window_new_from_surface (GstWlDisplay * display,
    struct wl_surface * surface, gint width, gint height)
{
  GstWlWindow *window;

  g_return_val_if_fail (surface != NULL, NULL);

  window = g_object_new (GST_TYPE_WL_WINDOW, NULL);
  window->display = g_object_ref (display);
  window->width = width;
  window->height = height;

  window->surface = surface;
  window->own_surface = FALSE;

  return window;
}

GstWlDisplay *
gst_wl_window_get_display (GstWlWindow * window)
{
  g_return_val_if_fail (window != NULL, NULL);

  return g_object_ref (window->display);
}

struct wl_surface *
gst_wl_window_get_wl_surface (GstWlWindow * window)
{
  g_return_val_if_fail (window != NULL, NULL);

  return window->surface;
}

void
gst_wl_window_get_size (GstWlWindow * window, gint * w, gint * h)
{
  g_return_if_fail (window != NULL);

  if (w)
    *w = window->width;

  if (h)
    *h = window->height;
}

void
gst_wl_window_set_size (GstWlWindow * window, gint w, gint h)
{
  g_return_if_fail (window != NULL);

  window->width = w;
  window->height = h;
}
