/* 
 * Copyright (C) 2003-2007 the GStreamer project
 *      Julien Moutte <julien@moutte.net>
 *      Ronald Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2005-2008 Tim-Philipp Müller <tim centricular net>
 * Copyright (C) 2009 Sebastian Dröge <sebastian.droege@collabora.co.uk>
 * Copyright (C) 2009  Andoni Morales Alastruey <ylatuya@gmail.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Totem project hereby grant permission for non-gpl compatible GStreamer
 * plugins to be used and distributed together with GStreamer and Totem. This
 * permission is above and beyond the permissions granted by the GPL license
 * Totem is covered by.
 *
 */


#include <gst/gst.h>

/* GStreamer Interfaces */
#include <gst/interfaces/navigation.h>
#include <gst/interfaces/colorbalance.h>
/* for detecting sources of errors */
#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>
#include <gst/audio/gstbaseaudiosink.h>
/* for pretty multichannel strings */
#include <gst/audio/multichannel.h>


/* for missing decoder/demuxer detection */
#include <gst/pbutils/pbutils.h>

/* for the cover metadata info */
#include <gst/tag/tag.h>


/* system */
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* gtk+/gnome */
#ifdef WIN32
#include <gdk/gdkwin32.h>
#else
#include <gdk/gdkx.h>
#endif
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib/gi18n.h>

#include "gst-helpers.h"
#include "bacon-video-widget.h"
#include "baconvideowidget-marshal.h"
#include "video-utils.h"

/* clutter */
#include <clutter-gst/clutter-gst.h>
#include <mx/mx.h>
#include "longomatch-aspect-frame.h"

#define LOGO_SIZE 400

#define is_error(e, d, c) \
  (e->domain == GST_##d##_ERROR && \
   e->code == GST_##d##_ERROR_##c)

/* Signals */
enum
{
  SIGNAL_ERROR,
  SIGNAL_EOS,
  SIGNAL_SEGMENT_DONE,
  SIGNAL_REDIRECT,
  SIGNAL_TITLE_CHANGE,
  SIGNAL_CHANNELS_CHANGE,
  SIGNAL_TICK,
  SIGNAL_GOT_METADATA,
  SIGNAL_BUFFERING,
  SIGNAL_MISSING_PLUGINS,
  SIGNAL_STATE_CHANGE,
  SIGNAL_GOT_DURATION,
  SIGNAL_READY_TO_SEEK,
  LAST_SIGNAL
};

/* Properties */
enum
{
  PROP_0,
  PROP_LOGO_MODE,
  PROP_EXPAND_LOGO,
  PROP_POSITION,
  PROP_CURRENT_TIME,
  PROP_STREAM_LENGTH,
  PROP_PLAYING,
  PROP_SEEKABLE,
  PROP_SHOW_CURSOR,
  PROP_MEDIADEV,
  PROP_VOLUME
};

struct BaconVideoWidgetPrivate
{
  char *mrl;

  GstElement *play;
  GstColorBalance *balance;

  guint update_id;

  GdkPixbuf *logo_pixbuf;

  gboolean media_has_video;
  gboolean media_has_audio;
  gint seekable;                /* -1 = don't know, FALSE = no */
  gint64 stream_length;
  gint64 current_time_nanos;
  gint64 current_time;
  gfloat current_position;
  gboolean is_live;

  GstTagList *tagcache;
  GstTagList *audiotags;
  GstTagList *videotags;

  gboolean got_redirect;

  GdkWindow *video_window;
  GdkCursor *cursor;

  /* Clutter */
  ClutterActor *stage;
  ClutterActor *texture;
  ClutterActor *frame;

  ClutterActor *drawings_frame;
  ClutterActor *drawings;

  ClutterActor *logo_frame;
  ClutterActor *logo;

  /* Other stuff */
  gboolean logo_mode;
  gboolean expand_logo;
  gboolean cursor_shown;
  gboolean fullscreen_mode;
  gboolean auto_resize;
  gboolean uses_fakesink;

  gint video_width;             /* Movie width */
  gint video_height;            /* Movie height */
  gboolean window_resized;      /* Whether the window has already been resized for this media */

  gint movie_par_n;             /* Movie pixel aspect ratio numerator */
  gint movie_par_d;             /* Movie pixel aspect ratio denominator */
  gint video_width_pixels;      /* Scaled movie width */
  gint video_height_pixels;     /* Scaled movie height */
  gint video_fps_n;
  gint video_fps_d;

  GstMessageType ignore_messages_mask;

  BvwUseType use_type;

  GstBus *bus;
  gulong sig_bus_sync;
  gulong sig_bus_async;

  gint eos_id;

  /* state we want to be in, as opposed to actual pipeline state
   * which may change asynchronously or during buffering */
  GstState target_state;
  gboolean buffering;
};

static void bacon_video_widget_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void bacon_video_widget_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);

static void bacon_video_widget_finalize (GObject * object);
static void bvw_process_pending_tag_messages (BaconVideoWidget * bvw);
static void bvw_stop_play_pipeline (BaconVideoWidget * bvw);
static GError *bvw_error_from_gst_error (BaconVideoWidget * bvw,
    GstMessage * m);


static GtkWidgetClass *parent_class = NULL;

static int bvw_signals[LAST_SIGNAL] = { 0 };

GST_DEBUG_CATEGORY (_longomatch_gst_debug_cat);
#define GST_CAT_DEFAULT _longomatch_gst_debug_cat


typedef gchar *(*MsgToStrFunc) (GstMessage * msg);

static void
bvw_error_msg (BaconVideoWidget * bvw, GstMessage * msg)
{
  GError *err = NULL;
  gchar *dbg = NULL;

  GST_DEBUG_BIN_TO_DOT_FILE (GST_BIN_CAST (bvw->priv->play),
      GST_DEBUG_GRAPH_SHOW_ALL ^
      GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS, "totem-error");

  gst_message_parse_error (msg, &err, &dbg);
  if (err) {
    GST_ERROR ("message = %s", GST_STR_NULL (err->message));
    GST_ERROR ("domain  = %d (%s)", err->domain,
        GST_STR_NULL (g_quark_to_string (err->domain)));
    GST_ERROR ("code    = %d", err->code);
    GST_ERROR ("debug   = %s", GST_STR_NULL (dbg));
    GST_ERROR ("source  = %" GST_PTR_FORMAT, msg->src);
    GST_ERROR ("uri     = %s", GST_STR_NULL (bvw->priv->mrl));

    g_message ("Error: %s\n%s\n", GST_STR_NULL (err->message),
        GST_STR_NULL (dbg));

    g_error_free (err);
  }
  g_free (dbg);
}

static void
set_display_pixel_aspect_ratio (GdkScreen * screen, GValue * value)
{
  static const gint par[][2] = {
    {1, 1},                     /* regular screen */
    {16, 15},                   /* PAL TV */
    {11, 10},                   /* 525 line Rec.601 video */
    {54, 59},                   /* 625 line Rec.601 video */
    {64, 45},                   /* 1280x1024 on 16:9 display */
    {5, 3},                     /* 1280x1024 on 4:3 display */
    {4, 3}                      /*  800x600 on 16:9 display */
  };
  guint i;
  gint par_index;
  gdouble ratio;
  gdouble delta;

#define DELTA(idx) (ABS (ratio - ((gdouble) par[idx][0] / par[idx][1])))

  /* first calculate the "real" ratio based on the X values;
   *    * which is the "physical" w/h divided by the w/h in pixels of the display */
  ratio =
      (gdouble) (gdk_screen_get_width_mm (screen) *
      gdk_screen_get_height (screen))
      / (gdk_screen_get_height_mm (screen) * gdk_screen_get_width (screen));

  GST_DEBUG ("calculated pixel aspect ratio: %f", ratio);
  /* now find the one from par[][2] with the lowest delta to the real one */
  delta = DELTA (0);
  par_index = 0;

  for (i = 1; i < sizeof (par) / (sizeof (gint) * 2); ++i) {
    gdouble this_delta = DELTA (i);

    if (this_delta < delta) {
      par_index = i;
      delta = this_delta;
    }
  }

  GST_DEBUG ("Decided on index %d (%d/%d)", par_index,
      par[par_index][0], par[par_index][1]);
  gst_value_set_fraction (value, par[par_index][0], par[par_index][1]);
}

static void
get_media_size (BaconVideoWidget * bvw, gint * width, gint * height)
{
  if (bvw->priv->logo_mode) {
    const GdkPixbuf *pixbuf;

    pixbuf = bvw->priv->logo_pixbuf;
    if (pixbuf) {
      *width = gdk_pixbuf_get_width (pixbuf);
      *height = gdk_pixbuf_get_height (pixbuf);
      if (*width == *height) {
        /* The icons will be square, so lie so we get a 16:9
         * ratio */
        *width = (int) ((float) *height / 9. * 16.);
      }
    } else {
      *width = 0;
      *height = 0;
    }
  } else {
    if (bvw->priv->media_has_video) {
      GValue disp_par = { 0, };
      guint movie_par_n, movie_par_d, disp_par_n, disp_par_d, num, den;

      /* Create and init the fraction value */
      g_value_init (&disp_par, GST_TYPE_FRACTION);

      /* Square pixel is our default */
      gst_value_set_fraction (&disp_par, 1, 1);

      /* Now try getting display's pixel aspect ratio */
      if (gtk_widget_get_realized (GTK_WIDGET (bvw)))
        set_display_pixel_aspect_ratio (gtk_widget_get_screen (GTK_WIDGET
                (bvw)), &disp_par);

      disp_par_n = gst_value_get_fraction_numerator (&disp_par);
      disp_par_d = gst_value_get_fraction_denominator (&disp_par);

      GST_DEBUG ("display PAR is %d/%d", disp_par_n, disp_par_d);

      /* Use the movie pixel aspect ratio if any */
      movie_par_n = bvw->priv->movie_par_n;
      movie_par_d = bvw->priv->movie_par_d;

      GST_DEBUG ("movie PAR is %d/%d", movie_par_n, movie_par_d);

      if (bvw->priv->video_width == 0 || bvw->priv->video_height == 0) {
        GST_DEBUG ("width and/or height 0, assuming 1/1 ratio");
        num = 1;
        den = 1;
      } else if (!gst_video_calculate_display_ratio (&num, &den,
              bvw->priv->video_width, bvw->priv->video_height,
              movie_par_n, movie_par_d, disp_par_n, disp_par_d)) {
        GST_WARNING ("overflow calculating display aspect ratio!");
        num = 1;                /* FIXME: what values to use here? */
        den = 1;
      }

      GST_DEBUG ("calculated scaling ratio %d/%d for video %dx%d", num, den,
          bvw->priv->video_width, bvw->priv->video_height);

      /* now find a width x height that respects this display ratio.
       * prefer those that have one of w/h the same as the incoming video
       * using wd / hd = num / den */

      /* start with same height, because of interlaced video */
      /* check hd / den is an integer scale factor, and scale wd with the PAR */
      if (bvw->priv->video_height % den == 0) {
        GST_DEBUG ("keeping video height");
        bvw->priv->video_width_pixels =
            (guint) gst_util_uint64_scale (bvw->priv->video_height, num, den);
        bvw->priv->video_height_pixels = bvw->priv->video_height;
      } else if (bvw->priv->video_width % num == 0) {
        GST_DEBUG ("keeping video width");
        bvw->priv->video_width_pixels = bvw->priv->video_width;
        bvw->priv->video_height_pixels =
            (guint) gst_util_uint64_scale (bvw->priv->video_width, den, num);
      } else {
        GST_DEBUG ("approximating while keeping video height");
        bvw->priv->video_width_pixels =
            (guint) gst_util_uint64_scale (bvw->priv->video_height, num, den);
        bvw->priv->video_height_pixels = bvw->priv->video_height;
      }
      GST_DEBUG ("scaling to %dx%d", bvw->priv->video_width_pixels,
          bvw->priv->video_height_pixels);

      *width = bvw->priv->video_width_pixels;
      *height = bvw->priv->video_height_pixels;

      /* Free the PAR fraction */
      g_value_unset (&disp_par);
    } else {
      *width = 0;
      *height = 0;
    }
  }
}

static void
bacon_video_widget_realize (GtkWidget * widget)
{
  GtkWidget *toplevel;

  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  gtk_widget_set_realized (widget, TRUE);

  /* setup the toplevel, ready to be resized */
  toplevel = gtk_widget_get_toplevel (widget);
  if (gtk_widget_is_toplevel (toplevel) &&
      gtk_widget_get_parent (widget) != toplevel)
    gtk_window_set_geometry_hints (GTK_WINDOW (toplevel), widget, NULL, 0);
}

static gboolean
draw_pixbuf (BaconVideoWidget * bvw, const GdkPixbuf * pixbuf,
    ClutterActor * actor)
{
  gboolean ret;
  GError *err = NULL;

  if (pixbuf == NULL) {
    return FALSE;
  }

  ret = clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE (actor),
      gdk_pixbuf_get_pixels (pixbuf),
      gdk_pixbuf_get_has_alpha (pixbuf),
      gdk_pixbuf_get_width (pixbuf),
      gdk_pixbuf_get_height (pixbuf),
      gdk_pixbuf_get_rowstride (pixbuf),
      gdk_pixbuf_get_has_alpha (pixbuf) ? 4 : 3, CLUTTER_TEXTURE_NONE, &err);
  clutter_actor_set_size (actor, gdk_pixbuf_get_width (pixbuf),
      gdk_pixbuf_get_height (pixbuf));
  if (ret == FALSE) {
    g_message ("clutter_texture_set_from_rgb_data failed %s", err->message);
    g_error_free (err);
  }

  return ret;
}

static void
set_current_actor (BaconVideoWidget * bvw)
{
  gboolean draw_logo;

  if (bvw->priv->stage == NULL)
    return;

  /* If there's only audio and no visualisation, draw the logo as well.
   * If we have a cover image to display, we display it regardless of whether we're
   * doing visualisations. */
  draw_logo = bvw->priv->media_has_audio && !bvw->priv->media_has_video;

  if (bvw->priv->logo_mode || draw_logo) {
    const GdkPixbuf *pixbuf;

    pixbuf = bvw->priv->logo_pixbuf;
    if (draw_pixbuf (bvw, pixbuf, bvw->priv->logo)) {
      clutter_actor_show (CLUTTER_ACTOR (bvw->priv->logo_frame));
      clutter_actor_hide (CLUTTER_ACTOR (bvw->priv->frame));
      return;
    }
  }
  clutter_actor_show (CLUTTER_ACTOR (bvw->priv->frame));
  clutter_actor_hide (CLUTTER_ACTOR (bvw->priv->logo_frame));
}

static void
bacon_video_widget_class_init (BaconVideoWidgetClass * klass)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  g_type_class_add_private (object_class, sizeof (BaconVideoWidgetPrivate));

  /* GtkWidget */
  //widget_class->size_request = bacon_video_widget_size_request;
  widget_class->realize = bacon_video_widget_realize;

  /* GObject */
  object_class->set_property = bacon_video_widget_set_property;
  object_class->get_property = bacon_video_widget_get_property;
  object_class->finalize = bacon_video_widget_finalize;

  /* Properties */
  g_object_class_install_property (object_class, PROP_LOGO_MODE,
      g_param_spec_boolean ("logo_mode", NULL, NULL, FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_EXPAND_LOGO,
      g_param_spec_boolean ("expand_logo", NULL,
          NULL, TRUE, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_POSITION,
      g_param_spec_int ("position", NULL, NULL,
          0, G_MAXINT, 0, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_STREAM_LENGTH,
      g_param_spec_int64 ("stream_length", NULL,
          NULL, 0, G_MAXINT64, 0, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_PLAYING,
      g_param_spec_boolean ("playing", NULL, NULL, FALSE, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_SEEKABLE,
      g_param_spec_boolean ("seekable", NULL, NULL, FALSE, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_VOLUME,
      g_param_spec_int ("volume", NULL, NULL, 0, 100, 0, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_SHOW_CURSOR,
      g_param_spec_boolean ("showcursor", NULL,
          NULL, FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_MEDIADEV,
      g_param_spec_string ("mediadev", NULL, NULL, FALSE, G_PARAM_READWRITE));


  /* Signals */
  bvw_signals[SIGNAL_ERROR] =
      g_signal_new ("error",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, error),
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  bvw_signals[SIGNAL_EOS] =
      g_signal_new ("eos",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, eos),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  bvw_signals[SIGNAL_SEGMENT_DONE] =
      g_signal_new ("segment_done",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, segment_done),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  bvw_signals[SIGNAL_READY_TO_SEEK] =
      g_signal_new ("ready_to_seek",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, ready_to_seek),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  bvw_signals[SIGNAL_GOT_DURATION] =
      g_signal_new ("got_duration",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, got_duration),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  bvw_signals[SIGNAL_GOT_METADATA] =
      g_signal_new ("got-metadata",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, got_metadata),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  bvw_signals[SIGNAL_REDIRECT] =
      g_signal_new ("got-redirect",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, got_redirect),
      NULL, NULL, g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);

  bvw_signals[SIGNAL_TITLE_CHANGE] =
      g_signal_new ("title-change",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, title_change),
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  bvw_signals[SIGNAL_CHANNELS_CHANGE] =
      g_signal_new ("channels-change",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, channels_change),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  bvw_signals[SIGNAL_TICK] =
      g_signal_new ("tick",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, tick),
      NULL, NULL,
      baconvideowidget_marshal_VOID__INT64_INT64_FLOAT_BOOLEAN,
      G_TYPE_NONE, 4, G_TYPE_INT64, G_TYPE_INT64, G_TYPE_FLOAT, G_TYPE_BOOLEAN);

  bvw_signals[SIGNAL_BUFFERING] =
      g_signal_new ("buffering",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, buffering),
      NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

  bvw_signals[SIGNAL_STATE_CHANGE] =
      g_signal_new ("state_change",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (BaconVideoWidgetClass, state_change),
      NULL, NULL,
      g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
bacon_video_widget_init (BaconVideoWidget * bvw)
{
  BaconVideoWidgetPrivate *priv;

  bvw->priv = priv =
      G_TYPE_INSTANCE_GET_PRIVATE (bvw, BACON_TYPE_VIDEO_WIDGET,
      BaconVideoWidgetPrivate);

  priv->update_id = 0;
  priv->tagcache = NULL;
  priv->audiotags = NULL;
  priv->videotags = NULL;
}

static gboolean bvw_query_timeout (BaconVideoWidget * bvw);
static void parse_stream_info (BaconVideoWidget * bvw);

static void
bvw_update_stream_info (BaconVideoWidget * bvw)
{
  parse_stream_info (bvw);

  g_signal_emit (bvw, bvw_signals[SIGNAL_GOT_METADATA], 0, NULL);
  g_signal_emit (bvw, bvw_signals[SIGNAL_CHANNELS_CHANGE], 0);
}

static void
bvw_handle_application_message (BaconVideoWidget * bvw, GstMessage * msg)
{
  const gchar *msg_name;

  msg_name = gst_structure_get_name (msg->structure);
  g_return_if_fail (msg_name != NULL);

  GST_DEBUG ("Handling application message: %" GST_PTR_FORMAT, msg->structure);

  if (strcmp (msg_name, "stream-changed") == 0) {
    bvw_update_stream_info (bvw);
  } else if (strcmp (msg_name, "video-size") == 0) {
    int w, h;

    g_signal_emit (bvw, bvw_signals[SIGNAL_GOT_METADATA], 0, NULL);

    /* This is necessary for the pixel-aspect-ratio of the
     * display to be taken into account. */
    get_media_size (bvw, &w, &h);
    clutter_actor_set_size (bvw->priv->texture, w, h);
    clutter_actor_set_size (bvw->priv->drawings, w, h);

    bvw->priv->window_resized = TRUE;
    set_current_actor (bvw);
  } else {
    g_message ("Unhandled application message %s", msg_name);
  }
}

static void
bvw_handle_element_message (BaconVideoWidget * bvw, GstMessage * msg)
{
  const gchar *type_name = NULL;
  gchar *src_name;

  src_name = gst_object_get_name (msg->src);
  if (msg->structure)
    type_name = gst_structure_get_name (msg->structure);

  GST_DEBUG ("from %s: %" GST_PTR_FORMAT, src_name, msg->structure);

  if (type_name == NULL)
    goto unhandled;

  if (strcmp (type_name, "redirect") == 0) {
    const gchar *new_location;

    new_location = gst_structure_get_string (msg->structure, "new-location");
    GST_DEBUG ("Got redirect to '%s'", GST_STR_NULL (new_location));

    if (new_location && *new_location) {
      g_signal_emit (bvw, bvw_signals[SIGNAL_REDIRECT], 0, new_location);
      goto done;
    }
  } else if (strcmp (type_name, "progress") == 0) {
    /* this is similar to buffering messages, but shouldn't affect pipeline
     * state; qtdemux emits those when headers are after movie data and
     * it is in streaming mode and has to receive all the movie data first */
    if (!bvw->priv->buffering) {
      gint percent = 0;

      if (gst_structure_get_int (msg->structure, "percent", &percent))
        g_signal_emit (bvw, bvw_signals[SIGNAL_BUFFERING], 0, percent);
    }
    goto done;
  }

unhandled:
  GST_WARNING ("Unhandled element message %s from %s: %" GST_PTR_FORMAT,
      GST_STR_NULL (type_name), GST_STR_NULL (src_name), msg);

done:
  g_free (src_name);
}

/* This is a hack to avoid doing poll_for_state_change() indirectly
 * from the bus message callback (via EOS => totem => close => wait for ready)
 * and deadlocking there. We need something like a
 * gst_bus_set_auto_flushing(bus, FALSE) ... */
static gboolean
bvw_signal_eos_delayed (gpointer user_data)
{
  BaconVideoWidget *bvw = BACON_VIDEO_WIDGET (user_data);
  g_signal_emit (bvw, bvw_signals[SIGNAL_EOS], 0, NULL);
  bvw->priv->eos_id = 0;
  return FALSE;
}

static void
bvw_reconfigure_tick_timeout (BaconVideoWidget * bvw, guint msecs)
{
  if (bvw->priv->update_id != 0) {
    GST_INFO ("removing tick timeout");
    g_source_remove (bvw->priv->update_id);
    bvw->priv->update_id = 0;
  }
  if (msecs > 0) {
    GST_INFO ("adding tick timeout (at %ums)", msecs);
    bvw->priv->update_id =
        g_timeout_add (msecs, (GSourceFunc) bvw_query_timeout, bvw);
  }
}

static void
bvw_bus_message_cb (GstBus * bus, GstMessage * message, gpointer data)
{
  BaconVideoWidget *bvw = (BaconVideoWidget *) data;
  GstMessageType msg_type;

  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));

  msg_type = GST_MESSAGE_TYPE (message);

  /* somebody else is handling the message, probably in poll_for_state_change */
  if (bvw->priv->ignore_messages_mask & msg_type) {
    GST_LOG ("Ignoring %s message from element %" GST_PTR_FORMAT
        " as requested: %" GST_PTR_FORMAT,
        GST_MESSAGE_TYPE_NAME (message), message->src, message);
    return;
  }

  if (msg_type != GST_MESSAGE_STATE_CHANGED) {
    gchar *src_name = gst_object_get_name (message->src);
    GST_LOG ("Handling %s message from element %s",
        gst_message_type_get_name (msg_type), src_name);
    g_free (src_name);
  }

  switch (msg_type) {
    case GST_MESSAGE_ERROR:
    {
      bvw_error_msg (bvw, message);

      GError *error;

      error = bvw_error_from_gst_error (bvw, message);

      bvw->priv->target_state = GST_STATE_NULL;
      if (bvw->priv->play)
        gst_element_set_state (bvw->priv->play, GST_STATE_NULL);

      bvw->priv->buffering = FALSE;

      g_signal_emit (bvw, bvw_signals[SIGNAL_ERROR], 0,
          error->message, TRUE, FALSE);

      g_error_free (error);
      break;
    }
    case GST_MESSAGE_WARNING:
    {
      GST_WARNING ("Warning message: %" GST_PTR_FORMAT, message);
      break;
    }
    case GST_MESSAGE_TAG:
    {
      GstTagList *tag_list, *result;
      GstElementFactory *f;

      gst_message_parse_tag (message, &tag_list);

      GST_DEBUG ("Tags: %" GST_PTR_FORMAT, tag_list);

      /* all tags (replace previous tags, title/artist/etc. might change
       * in the middle of a stream, e.g. with radio streams) */
      result = gst_tag_list_merge (bvw->priv->tagcache, tag_list,
          GST_TAG_MERGE_REPLACE);
      if (bvw->priv->tagcache)
        gst_tag_list_free (bvw->priv->tagcache);
      bvw->priv->tagcache = result;

      /* media-type-specific tags */
      if (GST_IS_ELEMENT (message->src) &&
          (f = gst_element_get_factory (GST_ELEMENT (message->src)))) {
        const gchar *klass = gst_element_factory_get_klass (f);
        GstTagList **cache = NULL;

        if (g_strrstr (klass, "Video")) {
          cache = &bvw->priv->videotags;
        } else if (g_strrstr (klass, "Audio")) {
          cache = &bvw->priv->audiotags;
        }

        if (cache) {
          result = gst_tag_list_merge (*cache, tag_list, GST_TAG_MERGE_REPLACE);
          if (*cache)
            gst_tag_list_free (*cache);
          *cache = result;
        }
      }

      /* clean up */
      gst_tag_list_free (tag_list);
      break;
    }
    case GST_MESSAGE_EOS:
      GST_DEBUG ("EOS message");
      /* update slider one last time */
      bvw_query_timeout (bvw);
      if (bvw->priv->eos_id == 0)
        bvw->priv->eos_id = g_idle_add (bvw_signal_eos_delayed, bvw);
      break;
    case GST_MESSAGE_BUFFERING:
    {
      gint percent = 0;

      /* FIXME: use gst_message_parse_buffering() once core 0.10.11 is out */
      gst_structure_get_int (message->structure, "buffer-percent", &percent);
      g_signal_emit (bvw, bvw_signals[SIGNAL_BUFFERING], 0, percent);

      if (percent >= 100) {
        /* a 100% message means buffering is done */
        bvw->priv->buffering = FALSE;
        /* if the desired state is playing, go back */
        if (bvw->priv->target_state == GST_STATE_PLAYING) {
          GST_DEBUG ("Buffering done, setting pipeline back to PLAYING");
          gst_element_set_state (bvw->priv->play, GST_STATE_PLAYING);
        } else {
          GST_DEBUG ("Buffering done, keeping pipeline PAUSED");
        }
      } else if (bvw->priv->buffering == FALSE &&
          bvw->priv->target_state == GST_STATE_PLAYING) {
        GstState cur_state;

        gst_element_get_state (bvw->priv->play, &cur_state, NULL, 0);
        if (cur_state == GST_STATE_PLAYING) {
          GST_DEBUG ("Buffering ... temporarily pausing playback");
          gst_element_set_state (bvw->priv->play, GST_STATE_PAUSED);
        } else {
          GST_DEBUG ("Buffering ... prerolling, not doing anything");
        }
        bvw->priv->buffering = TRUE;
      } else {
        GST_LOG ("Buffering ... %d", percent);
      }
      break;
    }
    case GST_MESSAGE_APPLICATION:
    {
      bvw_handle_application_message (bvw, message);
      break;
    }
    case GST_MESSAGE_STATE_CHANGED:
    {
      GstState old_state, new_state;
      gchar *src_name;

      gst_message_parse_state_changed (message, &old_state, &new_state, NULL);

      if (old_state == new_state)
        break;

      /* we only care about playbin (pipeline) state changes */
      if (GST_MESSAGE_SRC (message) != GST_OBJECT (bvw->priv->play))
        break;

      src_name = gst_object_get_name (message->src);
      GST_DEBUG ("%s changed state from %s to %s", src_name,
          gst_element_state_get_name (old_state),
          gst_element_state_get_name (new_state));
      g_free (src_name);

      /* now do stuff */
      if (new_state <= GST_STATE_PAUSED) {
        bvw_query_timeout (bvw);
        bvw_reconfigure_tick_timeout (bvw, 0);
        g_signal_emit (bvw, bvw_signals[SIGNAL_STATE_CHANGE], 0, FALSE);

      } else if (new_state == GST_STATE_PAUSED) {
        bvw_reconfigure_tick_timeout (bvw, 500);
        g_signal_emit (bvw, bvw_signals[SIGNAL_STATE_CHANGE], 0, FALSE);

      } else if (new_state > GST_STATE_PAUSED) {
        bvw_reconfigure_tick_timeout (bvw, 200);
        g_signal_emit (bvw, bvw_signals[SIGNAL_STATE_CHANGE], 0, TRUE);
      }


      if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED) {
        GST_DEBUG_BIN_TO_DOT_FILE (GST_BIN_CAST (bvw->priv->play),
            GST_DEBUG_GRAPH_SHOW_ALL ^
            GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS, "totem-prerolled");
        bvw->priv->stream_length = 0;
        if (bacon_video_widget_get_stream_length (bvw) == 0) {
          GST_DEBUG ("Failed to query duration in PAUSED state?!");
        }
        break;
        bvw_update_stream_info (bvw);
        g_signal_emit (bvw, bvw_signals[SIGNAL_READY_TO_SEEK], 0, FALSE);

      } else if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_READY) {
        bvw->priv->media_has_video = FALSE;
        bvw->priv->media_has_audio = FALSE;

        /* clean metadata cache */
        if (bvw->priv->tagcache) {
          gst_tag_list_free (bvw->priv->tagcache);
          bvw->priv->tagcache = NULL;
        }
        if (bvw->priv->audiotags) {
          gst_tag_list_free (bvw->priv->audiotags);
          bvw->priv->audiotags = NULL;
        }
        if (bvw->priv->videotags) {
          gst_tag_list_free (bvw->priv->videotags);
          bvw->priv->videotags = NULL;
        }

        bvw->priv->video_width = 0;
        bvw->priv->video_height = 0;
      }
      break;
    }
    case GST_MESSAGE_ELEMENT:
    {
      bvw_handle_element_message (bvw, message);
      break;
    }

    case GST_MESSAGE_DURATION:
    {
      /* force _get_stream_length() to do new duration query */
      /*bvw->priv->stream_length = 0;
         if (bacon_video_widget_get_stream_length (bvw) == 0)
         {
         GST_DEBUG ("Failed to query duration after DURATION message?!");
         }
         break; */
    }

    case GST_MESSAGE_CLOCK_PROVIDE:
    case GST_MESSAGE_CLOCK_LOST:
    case GST_MESSAGE_NEW_CLOCK:
    case GST_MESSAGE_STATE_DIRTY:
      break;

    default:
      GST_LOG ("Unhandled message: %" GST_PTR_FORMAT, message);
      break;
  }
}

static void
got_time_tick (GstElement * play, gint64 time_nanos, BaconVideoWidget * bvw)
{
  gboolean seekable;

  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));

  bvw->priv->current_time = (gint64) time_nanos / GST_MSECOND;

  if (bvw->priv->stream_length == 0) {
    bvw->priv->current_position = 0;
  } else {
    bvw->priv->current_position =
        (gdouble) bvw->priv->current_time / bvw->priv->stream_length;
  }

  if (bvw->priv->stream_length == 0) {
    seekable = bacon_video_widget_is_seekable (bvw);
  } else {
    if (bvw->priv->seekable == -1)
      g_object_notify (G_OBJECT (bvw), "seekable");
    seekable = TRUE;
  }

  bvw->priv->is_live = (bvw->priv->stream_length == 0);

/*
  GST_INFO ("%" GST_TIME_FORMAT ",%" GST_TIME_FORMAT " %s",
      GST_TIME_ARGS (bvw->priv->current_time),
      GST_TIME_ARGS (bvw->priv->stream_length),
      (seekable) ? "TRUE" : "FALSE"); 
*/

  g_signal_emit (bvw, bvw_signals[SIGNAL_TICK], 0,
      bvw->priv->current_time, bvw->priv->stream_length,
      bvw->priv->current_position, seekable);
}


static void
playbin_source_notify_cb (GObject * play, GParamSpec * p,
    BaconVideoWidget * bvw)
{
  /* CHECKME: do we really need these taglist frees here (tpm)? */
  if (bvw->priv->tagcache) {
    gst_tag_list_free (bvw->priv->tagcache);
    bvw->priv->tagcache = NULL;
  }
  if (bvw->priv->audiotags) {
    gst_tag_list_free (bvw->priv->audiotags);
    bvw->priv->audiotags = NULL;
  }
  if (bvw->priv->videotags) {
    gst_tag_list_free (bvw->priv->videotags);
    bvw->priv->videotags = NULL;
  }
}

static gboolean
bvw_query_timeout (BaconVideoWidget * bvw)
{
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 prev_len = -1;
  gint64 pos = -1, len = -1;

  /* check length/pos of stream */
  prev_len = bvw->priv->stream_length;
  if (gst_element_query_duration (bvw->priv->play, &fmt, &len)) {
    if (len != -1 && fmt == GST_FORMAT_TIME) {
      bvw->priv->stream_length = len / GST_MSECOND;
      if (bvw->priv->stream_length != prev_len) {
        g_signal_emit (bvw, bvw_signals[SIGNAL_GOT_METADATA], 0, NULL);
      }
    }
  } else {
    GST_INFO ("could not get duration");
  }

  if (gst_element_query_position (bvw->priv->play, &fmt, &pos)) {
    if (pos != -1 && fmt == GST_FORMAT_TIME) {
      got_time_tick (GST_ELEMENT (bvw->priv->play), pos, bvw);
    }
  } else {
    GST_INFO ("could not get position");
  }

  return TRUE;
}

static void
caps_set (GObject * obj, GParamSpec * pspec, BaconVideoWidget * bvw)
{
  GstPad *pad = GST_PAD (obj);
  GstStructure *s;
  GstCaps *caps;

  if (!(caps = gst_pad_get_negotiated_caps (pad)))
    return;

  /* Get video decoder caps */
  s = gst_caps_get_structure (caps, 0);
  if (s) {
    const GValue *movie_par;

    /* We need at least width/height and framerate */
    if (!(gst_structure_get_fraction (s, "framerate", &bvw->priv->video_fps_n,
                &bvw->priv->video_fps_d) &&
            gst_structure_get_int (s, "width", &bvw->priv->video_width) &&
            gst_structure_get_int (s, "height", &bvw->priv->video_height)))
      return;

    /* Get the movie PAR if available */
    movie_par = gst_structure_get_value (s, "pixel-aspect-ratio");
    if (movie_par) {
      bvw->priv->movie_par_n = gst_value_get_fraction_numerator (movie_par);
      bvw->priv->movie_par_d = gst_value_get_fraction_denominator (movie_par);
    } else {
      /* Square pixels */
      bvw->priv->movie_par_n = 1;
      bvw->priv->movie_par_d = 1;
    }
  }

  gst_caps_unref (caps);
}

static void
parse_stream_info (BaconVideoWidget * bvw)
{
  GstPad *videopad = NULL;
  gint n_audio, n_video;

  g_object_get (G_OBJECT (bvw->priv->play), "n-audio", &n_audio,
      "n-video", &n_video, NULL);

  bvw->priv->media_has_video = FALSE;
  if (n_video > 0) {
    gint i;

    bvw->priv->media_has_video = TRUE;

    for (i = 0; i < n_video && videopad == NULL; i++)
      g_signal_emit_by_name (bvw->priv->play, "get-video-pad", i, &videopad);
  }

  bvw->priv->media_has_audio = FALSE;
  if (n_audio > 0) {
    bvw->priv->media_has_audio = TRUE;
    if (!bvw->priv->media_has_video) {
      gint flags;

      g_object_get (bvw->priv->play, "flags", &flags, NULL);
      flags &= ~GST_PLAY_FLAG_VIS;
      g_object_set (bvw->priv->play, "flags", flags, NULL);
    }
  }

  if (videopad) {
    GstCaps *caps;

    if ((caps = gst_pad_get_negotiated_caps (videopad))) {
      caps_set (G_OBJECT (videopad), NULL, bvw);
      gst_caps_unref (caps);
    }
    g_signal_connect (videopad, "notify::caps", G_CALLBACK (caps_set), bvw);
    gst_object_unref (videopad);
  }

  set_current_actor (bvw);
}

static void
playbin_stream_changed_cb (GstElement * obj, gpointer data)
{
  BaconVideoWidget *bvw = BACON_VIDEO_WIDGET (data);
  GstMessage *msg;

  /* we're being called from the streaming thread, so don't do anything here */
  GST_LOG ("streams have changed");
  msg = gst_message_new_application (GST_OBJECT (bvw->priv->play),
      gst_structure_new ("stream-changed", NULL));
  gst_element_post_message (bvw->priv->play, msg);
}

static void
bacon_video_widget_finalize (GObject * object)
{
  BaconVideoWidget *bvw = (BaconVideoWidget *) object;

  GST_INFO ("finalizing");

  if (bvw->priv->bus) {
    /* make bus drop all messages to make sure none of our callbacks is ever
     * called again (main loop might be run again to display error dialog) */
    gst_bus_set_flushing (bvw->priv->bus, TRUE);

    if (bvw->priv->sig_bus_sync)
      g_signal_handler_disconnect (bvw->priv->bus, bvw->priv->sig_bus_sync);

    if (bvw->priv->sig_bus_async)
      g_signal_handler_disconnect (bvw->priv->bus, bvw->priv->sig_bus_async);

    gst_object_unref (bvw->priv->bus);
    bvw->priv->bus = NULL;
  }

  g_free (bvw->priv->mrl);
  bvw->priv->mrl = NULL;

  if (bvw->priv->play != NULL && GST_IS_ELEMENT (bvw->priv->play)) {
    gst_element_set_state (bvw->priv->play, GST_STATE_NULL);
    gst_object_unref (bvw->priv->play);
    bvw->priv->play = NULL;
  }

  if (bvw->priv->update_id) {
    g_source_remove (bvw->priv->update_id);
    bvw->priv->update_id = 0;
  }

  if (bvw->priv->tagcache) {
    gst_tag_list_free (bvw->priv->tagcache);
    bvw->priv->tagcache = NULL;
  }
  if (bvw->priv->audiotags) {
    gst_tag_list_free (bvw->priv->audiotags);
    bvw->priv->audiotags = NULL;
  }
  if (bvw->priv->videotags) {
    gst_tag_list_free (bvw->priv->videotags);
    bvw->priv->videotags = NULL;
  }

  if (bvw->priv->cursor != NULL) {
    gdk_cursor_unref (bvw->priv->cursor);
    bvw->priv->cursor = NULL;
  }

  if (bvw->priv->eos_id != 0)
    g_source_remove (bvw->priv->eos_id);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bacon_video_widget_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  BaconVideoWidget *bvw;

  bvw = BACON_VIDEO_WIDGET (object);

  switch (property_id) {
    case PROP_LOGO_MODE:
      bacon_video_widget_set_logo_mode (bvw, g_value_get_boolean (value));
      break;
    case PROP_EXPAND_LOGO:
      bvw->priv->expand_logo = g_value_get_boolean (value);
      break;
    case PROP_VOLUME:
      bacon_video_widget_set_volume (bvw, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
bacon_video_widget_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  BaconVideoWidget *bvw;

  bvw = BACON_VIDEO_WIDGET (object);

  switch (property_id) {
    case PROP_LOGO_MODE:
      g_value_set_boolean (value, bacon_video_widget_get_logo_mode (bvw));
      break;
    case PROP_EXPAND_LOGO:
      g_value_set_boolean (value, bvw->priv->expand_logo);
      break;
    case PROP_POSITION:
      g_value_set_int64 (value, bacon_video_widget_get_position (bvw));
      break;
    case PROP_STREAM_LENGTH:
      g_value_set_int64 (value, bacon_video_widget_get_stream_length (bvw));
      break;
    case PROP_PLAYING:
      g_value_set_boolean (value, bacon_video_widget_is_playing (bvw));
      break;
    case PROP_SEEKABLE:
      g_value_set_boolean (value, bacon_video_widget_is_seekable (bvw));
      break;
    case PROP_VOLUME:
      g_value_set_int (value, bacon_video_widget_get_volume (bvw));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

/* ============================================================= */
/*                                                               */
/*                       Public Methods                          */
/*                                                               */
/* ============================================================= */


/**
 * bacon_video_widget_get_backend_name:
 * @bvw: a #BaconVideoWidget
 *
 * Returns the name string for @bvw. For the GStreamer backend, it is the output
 * of gst_version_string(). *
 * Return value: the backend's name; free with g_free()
 **/
char *
bacon_video_widget_get_backend_name (BaconVideoWidget * bvw)
{
  return gst_version_string ();
}


/* =========================================== */
/*                                             */
/*               Play/Pause, Stop              */
/*                                             */
/* =========================================== */

static GError *
bvw_error_from_gst_error (BaconVideoWidget * bvw, GstMessage * err_msg)
{
  const gchar *src_typename;
  GError *ret = NULL;
  GError *e = NULL;

  GST_LOG ("resolving error message %" GST_PTR_FORMAT, err_msg);

  src_typename = (err_msg->src) ? G_OBJECT_TYPE_NAME (err_msg->src) : NULL;

  gst_message_parse_error (err_msg, &e, NULL);

  if (is_error (e, RESOURCE, NOT_FOUND) || is_error (e, RESOURCE, OPEN_READ)) {
#if 0
    if (strchr (mrl, ':') &&
        (g_str_has_prefix (mrl, "dvd") ||
            g_str_has_prefix (mrl, "cd") || g_str_has_prefix (mrl, "vcd"))) {
      ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_INVALID_DEVICE,
          e->message);
    } else {
#endif
      if (e->code == GST_RESOURCE_ERROR_NOT_FOUND) {
        if (GST_IS_BASE_AUDIO_SINK (err_msg->src)) {
          ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_AUDIO_PLUGIN,
              _("The requested audio output was not found. "
                  "Please select another audio output in the Multimedia "
                  "Systems Selector."));
        } else {
          ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_FILE_NOT_FOUND,
              _("Location not found."));
        }
      } else {
        ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_FILE_PERMISSION,
            _("Could not open location; "
                "you might not have permission to open the file."));
      }
#if 0
    }
#endif
  } else if (is_error (e, RESOURCE, BUSY)) {
    if (GST_IS_VIDEO_SINK (err_msg->src)) {
      /* a somewhat evil check, but hey.. */
      ret = g_error_new_literal (BVW_ERROR,
          BVW_ERROR_VIDEO_PLUGIN,
          _("The video output is in use by another application. "
              "Please close other video applications, or select "
              "another video output in the Multimedia Systems Selector."));
    } else if (GST_IS_BASE_AUDIO_SINK (err_msg->src)) {
      ret = g_error_new_literal (BVW_ERROR,
          BVW_ERROR_AUDIO_BUSY,
          _("The audio output is in use by another application. "
              "Please select another audio output in the Multimedia Systems Selector. "
              "You may want to consider using a sound server."));
    }
  } else if (e->domain == GST_RESOURCE_ERROR) {
    ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_FILE_GENERIC, e->message);
  } else if (is_error (e, CORE, MISSING_PLUGIN) ||
      is_error (e, STREAM, CODEC_NOT_FOUND)) {
    GST_LOG ("no missing plugin messages, posting generic error");
    ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_CODEC_NOT_HANDLED,
        e->message);
  } else if (is_error (e, STREAM, WRONG_TYPE) ||
      is_error (e, STREAM, NOT_IMPLEMENTED)) {
    if (src_typename) {
      ret = g_error_new (BVW_ERROR, BVW_ERROR_CODEC_NOT_HANDLED, "%s: %s",
          src_typename, e->message);
    } else {
      ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_CODEC_NOT_HANDLED,
          e->message);
    }
  } else if (is_error (e, STREAM, FAILED) &&
      src_typename && strncmp (src_typename, "GstTypeFind", 11) == 0) {
    ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_READ_ERROR,
        _("Cannot play this file over the network. "
            "Try downloading it to disk first."));
  } else {
    /* generic error, no code; take message */
    ret = g_error_new_literal (BVW_ERROR, BVW_ERROR_GENERIC, e->message);
  }
  g_error_free (e);

  return ret;
}

static gboolean
poll_for_state_change_full (BaconVideoWidget * bvw, GstElement * element,
    GstState state, GstMessage ** err_msg, gint64 timeout)
{
  GstBus *bus;
  GstMessageType events, saved_events;

  g_assert (err_msg != NULL);

  bus = gst_element_get_bus (element);

  events = GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS;

  saved_events = bvw->priv->ignore_messages_mask;

  if (element != NULL && element == bvw->priv->play) {
    /* we do want the main handler to process state changed messages for
     * playbin as well, otherwise it won't hook up the timeout etc. */
    bvw->priv->ignore_messages_mask |= (events ^ GST_MESSAGE_STATE_CHANGED);
  } else {
    bvw->priv->ignore_messages_mask |= events;
  }

  while (TRUE) {
    GstMessage *message;
    GstElement *src;

    message = gst_bus_poll (bus, events, timeout);

    if (!message)
      goto timed_out;

    src = (GstElement *) GST_MESSAGE_SRC (message);

    switch (GST_MESSAGE_TYPE (message)) {
      case GST_MESSAGE_STATE_CHANGED:
      {
        GstState old, new, pending;

        if (src == element) {
          gst_message_parse_state_changed (message, &old, &new, &pending);
          if (new == state) {
            gst_message_unref (message);
            goto success;
          }
        }
        break;
      }
      case GST_MESSAGE_ERROR:
      {
        bvw_error_msg (bvw, message);
        *err_msg = message;
        message = NULL;
        goto error;
        break;
      }
      case GST_MESSAGE_EOS:
      {
        GError *e = NULL;

        gst_message_unref (message);
        e = g_error_new_literal (BVW_ERROR, BVW_ERROR_FILE_GENERIC,
            _("Media file could not be played."));
        *err_msg =
            gst_message_new_error (GST_OBJECT (bvw->priv->play), e, NULL);
        g_error_free (e);
        goto error;
        break;
      }
      default:
        g_assert_not_reached ();
        break;
    }

    gst_message_unref (message);
  }

  g_assert_not_reached ();

success:
  /* state change succeeded */
  GST_DEBUG ("state change to %s succeeded",
      gst_element_state_get_name (state));
  bvw->priv->ignore_messages_mask = saved_events;
  return TRUE;

timed_out:
  /* it's taking a long time to open -- just tell totem it was ok, this allows
   * the user to stop the loading process with the normal stop button */
  GST_DEBUG ("state change to %s timed out, returning success and handling "
      "errors asynchronously", gst_element_state_get_name (state));
  bvw->priv->ignore_messages_mask = saved_events;
  return TRUE;

error:
  GST_DEBUG ("error while waiting for state change to %s: %" GST_PTR_FORMAT,
      gst_element_state_get_name (state), *err_msg);
  /* already set *err_msg */
  bvw->priv->ignore_messages_mask = saved_events;
  return FALSE;
}

/**
 * bacon_video_widget_open:
 * @bvw: a #BaconVideoWidget
 * @mrl: an MRL
 * @error: a #GError, or %NULL
 *
 * Opens the given @mrl in @bvw for playing.
 *
 * If there was a filesystem error, a %ERROR_GENERIC error will be returned. Otherwise,
 * more specific #BvwError errors will be returned.
 *
 * On success, the MRL is loaded and waiting to be played with bacon_video_widget_play().
 *
 * Return value: %TRUE on success, %FALSE otherwise
 **/

gboolean
bacon_video_widget_open (BaconVideoWidget * bvw,
    const gchar * mrl, GError ** error)
{

  GstMessage *err_msg = NULL;
  GFile *file;
  gboolean ret;
  char *path;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (mrl != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (bvw->priv->play != NULL, FALSE);


  /* So we aren't closed yet... */
  if (bvw->priv->mrl) {
    bacon_video_widget_close (bvw);
  }

  GST_DEBUG ("mrl = %s", GST_STR_NULL (mrl));

  /* this allows non-URI type of files in the thumbnailer and so on */
  file = g_file_new_for_commandline_arg (mrl);


  /* Only use the URI when FUSE isn't available for a file */
  path = g_file_get_path (file);
  if (path) {
    bvw->priv->mrl = g_filename_to_uri (path, NULL, NULL);
    g_free (path);
  } else {
    bvw->priv->mrl = g_strdup (mrl);
  }

  g_object_unref (file);

  if (g_str_has_prefix (mrl, "icy:") != FALSE) {
    /* Handle "icy://" URLs from QuickTime */
    g_free (bvw->priv->mrl);
    bvw->priv->mrl = g_strdup_printf ("http:%s", mrl + 4);
  } else if (g_str_has_prefix (mrl, "icyx:") != FALSE) {
    /* Handle "icyx://" URLs from Orban/Coding Technologies AAC/aacPlus Player */
    g_free (bvw->priv->mrl);
    bvw->priv->mrl = g_strdup_printf ("http:%s", mrl + 5);
  }

  bvw->priv->got_redirect = FALSE;
  bvw->priv->media_has_video = FALSE;
  bvw->priv->media_has_audio = FALSE;
  bvw->priv->stream_length = 0;
  bvw->priv->ignore_messages_mask = 0;

  g_object_set (bvw->priv->play, "uri", bvw->priv->mrl, NULL);

  bvw->priv->seekable = -1;
  bvw->priv->target_state = GST_STATE_PAUSED;

  gst_element_set_state (bvw->priv->play, GST_STATE_PAUSED);

  if (bvw->priv->use_type == BVW_USE_TYPE_PLAYER) {
    GST_INFO ("normal playback, handling all errors asynchroneously");
    ret = TRUE;
  } else {
    /* used as thumbnailer or metadata extractor for properties dialog. In
     * this case, wait for any state change to really finish and process any
     * pending tag messages, so that the information is available right away */
    GST_INFO ("waiting for state changed to PAUSED to complete");
    ret = poll_for_state_change_full (bvw, bvw->priv->play,
        GST_STATE_PAUSED, &err_msg, -1);

    bvw_process_pending_tag_messages (bvw);
    bacon_video_widget_get_stream_length (bvw);
    GST_INFO ("stream length = %u", bvw->priv->stream_length);

    /* even in case of an error (e.g. no decoders installed) we might still
     * have useful metadata (like codec types, duration, etc.) */
    g_signal_emit (bvw, bvw_signals[SIGNAL_GOT_METADATA], 0, NULL);
  }

  if (ret) {
    g_signal_emit (bvw, bvw_signals[SIGNAL_CHANNELS_CHANGE], 0);
  } else {
    GST_INFO ("Error on open: %" GST_PTR_FORMAT, err_msg);
    bvw->priv->ignore_messages_mask |= GST_MESSAGE_ERROR;
    bvw_stop_play_pipeline (bvw);
    g_free (bvw->priv->mrl);
    bvw->priv->mrl = NULL;
  }

  /* When opening a new media we want to redraw ourselves */
  gtk_widget_queue_draw (GTK_WIDGET (bvw));

  if (err_msg != NULL) {
    if (error) {
      *error = bvw_error_from_gst_error (bvw, err_msg);

    } else {
      GST_WARNING ("Got error, but caller is not collecting error details!");
    }
    gst_message_unref (err_msg);
  }


  return ret;
}

/**
 * bacon_video_widget_play:
 * @bvw: a #BaconVideoWidget
 * @error: a #GError, or %NULL
 *
 * Plays the currently-loaded video in @bvw.
 *
 * Errors from the GStreamer backend will be returned asynchronously via the
 * #BaconVideoWidget::error signal, even if this function returns %TRUE.
 *
 * Return value: %TRUE on success, %FALSE otherwise
 **/
gboolean
bacon_video_widget_play (BaconVideoWidget * bvw)
{

  GstState cur_state;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);
  g_return_val_if_fail (bvw->priv->mrl != NULL, FALSE);

  bvw->priv->target_state = GST_STATE_PLAYING;

  /* no need to actually go into PLAYING in capture/metadata mode (esp.
   * not with sinks that don't sync to the clock), we'll get everything
   * we need by prerolling the pipeline, and that is done in _open() */
  if (bvw->priv->use_type == BVW_USE_TYPE_CAPTURE ||
      bvw->priv->use_type == BVW_USE_TYPE_METADATA) {
    return TRUE;
  }

  /* just lie and do nothing in this case */
  gst_element_get_state (bvw->priv->play, &cur_state, NULL, 0);

  GST_INFO ("play");
  gst_element_set_state (bvw->priv->play, GST_STATE_PLAYING);

  /* will handle all errors asynchroneously */
  return TRUE;
}

/**
 * bacon_video_widget_can_direct_seek:
 * @bvw: a #BaconVideoWidget
 *
 * Determines whether direct seeking is possible for the current stream.
 *
 * Return value: %TRUE if direct seeking is possible, %FALSE otherwise
 **/
gboolean
bacon_video_widget_can_direct_seek (BaconVideoWidget * bvw)
{
  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  if (bvw->priv->mrl == NULL)
    return FALSE;

  /* (instant seeking only make sense with video,
   * hence no cdda:// here) */
  if (g_str_has_prefix (bvw->priv->mrl, "file://") ||
      g_str_has_prefix (bvw->priv->mrl, "dvd:/") ||
      g_str_has_prefix (bvw->priv->mrl, "vcd:/"))
    return TRUE;

  return FALSE;
}

//If we want to seek throug a seekbar we want speed, so we use the KEY_UNIT flag
//Sometimes accurate position is requested so we use the ACCURATE flag
gboolean
bacon_video_widget_seek_time (BaconVideoWidget * bvw, gint64 time,
    gfloat rate, gboolean accurate)
{
  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  GST_LOG ("Seeking to %" GST_TIME_FORMAT, GST_TIME_ARGS (time * GST_MSECOND));

  if (time > bvw->priv->stream_length
      && bvw->priv->stream_length > 0
      && !g_str_has_prefix (bvw->priv->mrl, "dvd:")
      && !g_str_has_prefix (bvw->priv->mrl, "vcd:")) {
    if (bvw->priv->eos_id == 0)
      bvw->priv->eos_id = g_idle_add (bvw_signal_eos_delayed, bvw);
    return TRUE;
  }


  if (accurate) {
    got_time_tick (bvw->priv->play, time * GST_MSECOND, bvw);
    gst_element_seek (bvw->priv->play, rate,
        GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
        GST_SEEK_TYPE_SET, time * GST_MSECOND,
        GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
  } else {
    /* Emit a time tick of where we are going, we are paused */
    got_time_tick (bvw->priv->play, time * GST_MSECOND, bvw);
    gst_element_seek (bvw->priv->play, rate,
        GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
        GST_SEEK_TYPE_SET, time * GST_MSECOND,
        GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
  }
  return TRUE;
}

gboolean
bacon_video_widget_seek (BaconVideoWidget * bvw, gdouble position, gfloat rate)
{

  gint64 seek_time, length_nanos;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  length_nanos = (gint64) (bvw->priv->stream_length * GST_MSECOND);
  seek_time = (gint64) (length_nanos * position);

  GST_LOG ("Seeking to %3.2f%% %" GST_TIME_FORMAT, position,
      GST_TIME_ARGS (seek_time));

  return bacon_video_widget_seek_time (bvw, seek_time / GST_MSECOND, rate,
      FALSE);
}

gboolean
bacon_video_widget_seek_in_segment (BaconVideoWidget * bvw, gint64 pos,
    gfloat rate)
{

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  GST_LOG ("Segment seeking from %" GST_TIME_FORMAT,
      GST_TIME_ARGS (pos * GST_MSECOND));

  if (pos > bvw->priv->stream_length
      && bvw->priv->stream_length > 0
      && !g_str_has_prefix (bvw->priv->mrl, "dvd:")
      && !g_str_has_prefix (bvw->priv->mrl, "vcd:")) {
    if (bvw->priv->eos_id == 0)
      bvw->priv->eos_id = g_idle_add (bvw_signal_eos_delayed, bvw);
    return TRUE;
  }

  got_time_tick (bvw->priv->play, pos * GST_MSECOND, bvw);
  gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT |
      GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
      pos * GST_MSECOND, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

  return TRUE;
}

gboolean
bacon_video_widget_set_rate_in_segment (BaconVideoWidget * bvw, gfloat rate,
    gint64 stop)
{
  guint64 pos;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  pos = bacon_video_widget_get_accurate_current_time (bvw);
  if (pos == 0)
    return FALSE;

  gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE |
      GST_SEEK_FLAG_SEGMENT, GST_SEEK_TYPE_SET,
      pos * GST_MSECOND, GST_SEEK_TYPE_SET, stop * GST_MSECOND);

  return TRUE;
}

gboolean
bacon_video_widget_set_rate (BaconVideoWidget * bvw, gfloat rate)
{
  guint64 pos;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  pos = bacon_video_widget_get_accurate_current_time (bvw);
  if (pos == 0)
    return FALSE;

  gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
      GST_SEEK_TYPE_SET,
      pos * GST_MSECOND, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

  return TRUE;
}


gboolean
bacon_video_widget_new_file_seek (BaconVideoWidget * bvw, gint64 start,
    gint64 stop, gfloat rate)
{

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  GST_LOG ("Segment seeking from %" GST_TIME_FORMAT,
      GST_TIME_ARGS (start * GST_MSECOND));

  if (start > bvw->priv->stream_length
      && bvw->priv->stream_length > 0
      && !g_str_has_prefix (bvw->priv->mrl, "dvd:")
      && !g_str_has_prefix (bvw->priv->mrl, "vcd:")) {
    if (bvw->priv->eos_id == 0)
      bvw->priv->eos_id = g_idle_add (bvw_signal_eos_delayed, bvw);
    return TRUE;
  }


  GST_LOG ("Segment seeking from %" GST_TIME_FORMAT,
      GST_TIME_ARGS (start * GST_MSECOND));

  //FIXME Needs to wait until GST_STATE_PAUSED
  gst_element_get_state (bvw->priv->play, NULL, NULL, 0);

  got_time_tick (bvw->priv->play, start * GST_MSECOND, bvw);
  gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT |
      GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
      start * GST_MSECOND, GST_SEEK_TYPE_SET, stop * GST_MSECOND);
  gst_element_set_state (bvw->priv->play, GST_STATE_PLAYING);

  return TRUE;
}

gboolean
bacon_video_widget_segment_seek (BaconVideoWidget * bvw, gint64 start,
    gint64 stop, gfloat rate)
{

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  GST_LOG ("Segment seeking from %" GST_TIME_FORMAT,
      GST_TIME_ARGS (start * GST_MSECOND));


  if (start > bvw->priv->stream_length
      && bvw->priv->stream_length > 0
      && !g_str_has_prefix (bvw->priv->mrl, "dvd:")
      && !g_str_has_prefix (bvw->priv->mrl, "vcd:")) {
    if (bvw->priv->eos_id == 0)
      bvw->priv->eos_id = g_idle_add (bvw_signal_eos_delayed, bvw);
    return TRUE;
  }

  got_time_tick (bvw->priv->play, start * GST_MSECOND, bvw);
  gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT |
      GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
      start * GST_MSECOND, GST_SEEK_TYPE_SET, stop * GST_MSECOND);

  return TRUE;
}

gboolean
bacon_video_widget_seek_to_next_frame (BaconVideoWidget * bvw, gfloat rate,
    gboolean in_segment)
{
  gint64 pos = -1;
  gboolean ret;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  gst_element_send_event (bvw->priv->play,
      gst_event_new_step (GST_FORMAT_BUFFERS, 1, 1.0, TRUE, FALSE));

  pos = bacon_video_widget_get_accurate_current_time (bvw);
  got_time_tick (GST_ELEMENT (bvw->priv->play), pos * GST_MSECOND, bvw);

  return ret;
}

gboolean
bacon_video_widget_seek_to_previous_frame (BaconVideoWidget * bvw,
    gfloat rate, gboolean in_segment)
{
  gint fps;
  gint64 pos;
  gint64 final_pos;
  guint8 seek_flags;
  gboolean ret;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);


  //Round framerate to the nearest integer        
  fps = (bvw->priv->video_fps_n + bvw->priv->video_fps_d / 2) /
      bvw->priv->video_fps_d;
  pos = bacon_video_widget_get_accurate_current_time (bvw);
  final_pos = pos * GST_MSECOND - 1 * GST_SECOND / fps;

  if (pos == 0)
    return FALSE;

  if (bacon_video_widget_is_playing (bvw))
    bacon_video_widget_pause (bvw);

  seek_flags = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE;
  if (in_segment)
    seek_flags = seek_flags | GST_SEEK_FLAG_SEGMENT;
  ret = gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME, seek_flags, GST_SEEK_TYPE_SET,
      final_pos, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

  got_time_tick (GST_ELEMENT (bvw->priv->play), pos * GST_MSECOND, bvw);

  return ret;
}

gboolean
bacon_video_widget_segment_stop_update (BaconVideoWidget * bvw, gint64 stop,
    gfloat rate)
{
  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT |
      GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
      stop * GST_MSECOND - 1, GST_SEEK_TYPE_SET, stop * GST_MSECOND);

  if (bacon_video_widget_is_playing (bvw))
    bacon_video_widget_pause (bvw);

  return TRUE;
}

gboolean
bacon_video_widget_segment_start_update (BaconVideoWidget * bvw, gint64 start,
    gfloat rate)
{
  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  gst_element_seek (bvw->priv->play, rate,
      GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT |
      GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
      start * GST_MSECOND, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

  if (bacon_video_widget_is_playing (bvw))
    bacon_video_widget_pause (bvw);

  return TRUE;
}


static void
bvw_stop_play_pipeline (BaconVideoWidget * bvw)
{
  GstState cur_state;

  gst_element_get_state (bvw->priv->play, &cur_state, NULL, 0);
  if (cur_state > GST_STATE_READY) {
    GstMessage *msg;
    GstBus *bus;

    GST_INFO ("stopping");
    gst_element_set_state (bvw->priv->play, GST_STATE_READY);

    /* process all remaining state-change messages so everything gets
     * cleaned up properly (before the state change to NULL flushes them) */
    GST_INFO ("processing pending state-change messages");
    bus = gst_element_get_bus (bvw->priv->play);
    while ((msg = gst_bus_poll (bus, GST_MESSAGE_STATE_CHANGED, 0))) {
      gst_bus_async_signal_func (bus, msg, NULL);
      gst_message_unref (msg);
    }
    gst_object_unref (bus);
  }

  gst_element_set_state (bvw->priv->play, GST_STATE_NULL);
  bvw->priv->target_state = GST_STATE_NULL;
  bvw->priv->buffering = FALSE;
  bvw->priv->ignore_messages_mask = 0;
  GST_INFO ("stopped");
}

/**
 * bacon_video_widget_stop:
 * @bvw: a #BaconVideoWidget
 *
 * Stops playing the current stream and resets to the first position in the stream.
 **/
void
bacon_video_widget_stop (BaconVideoWidget * bvw)
{
  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  g_return_if_fail (GST_IS_ELEMENT (bvw->priv->play));

  GST_LOG ("Stopping");
  bvw_stop_play_pipeline (bvw);

  /* Reset position to 0 when stopping */
  got_time_tick (GST_ELEMENT (bvw->priv->play), 0, bvw);
}


/**
 * bacon_video_widget_close:
 * @bvw: a #BaconVideoWidget
 *
 * Closes the current stream and frees the resources associated with it.
 **/
void
bacon_video_widget_close (BaconVideoWidget * bvw)
{
  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  g_return_if_fail (GST_IS_ELEMENT (bvw->priv->play));

  GST_LOG ("Closing");
  bvw_stop_play_pipeline (bvw);

  g_free (bvw->priv->mrl);
  bvw->priv->mrl = NULL;
  bvw->priv->is_live = FALSE;
  bvw->priv->window_resized = FALSE;

  g_object_notify (G_OBJECT (bvw), "seekable");
  g_signal_emit (bvw, bvw_signals[SIGNAL_CHANNELS_CHANGE], 0);
  got_time_tick (GST_ELEMENT (bvw->priv->play), 0, bvw);
}

/**
 * bacon_video_widget_set_logo:
 * @bvw: a #BaconVideoWidget
 * @name: the icon name of the logo
 *
 * Sets the logo displayed on the video widget when no stream is loaded.
 **/
void
bacon_video_widget_set_logo (BaconVideoWidget * bvw, const gchar * filename)
{
  GError *error = NULL;

  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  g_return_if_fail (filename != NULL);

  if (bvw->priv->logo_pixbuf != NULL)
    g_object_unref (bvw->priv->logo_pixbuf);

  bvw->priv->logo_pixbuf = gdk_pixbuf_new_from_file (filename, &error);

  if (error) {
    g_warning ("An error occurred trying to open logo %s: %s", filename,
        error->message);
    g_error_free (error);
    return;
  }

  set_current_actor (bvw);
}

/**
 * bacon_video_widget_set_logo_mode:
 * @bvw: a #BaconVideoWidget
 * @logo_mode: %TRUE to display the logo, %FALSE otherwise
 *
 * Sets whether to display a logo set with @bacon_video_widget_set_logo when
 * no stream is loaded. If @logo_mode is %FALSE, nothing will be displayed
 * and the video widget will take up no space. Otherwise, the logo will be
 * displayed and will requisition a corresponding amount of space.
 **/
void
bacon_video_widget_set_logo_mode (BaconVideoWidget * bvw, gboolean logo_mode)
{
  BaconVideoWidgetPrivate *priv;

  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  priv = bvw->priv;

  logo_mode = logo_mode != FALSE;

  if (priv->logo_mode != logo_mode) {
    priv->logo_mode = logo_mode;

    set_current_actor (bvw);

    g_object_notify (G_OBJECT (bvw), "logo_mode");
    g_object_notify (G_OBJECT (bvw), "seekable");
  }
}

/**
 * bacon_video_widget_get_logo_mode
 * @bvw: a #BaconVideoWidget
 *
 * Gets whether the logo is displayed when no stream is loaded.
 *
 * Return value: %TRUE if the logo is displayed, %FALSE otherwise
 **/
gboolean
bacon_video_widget_get_logo_mode (BaconVideoWidget * bvw)
{
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);

  return bvw->priv->logo_mode;
}

/**
 * bacon_video_widget_pause:
 * @bvw: a #BaconVideoWidget
 *
 * Pauses the current stream in the video widget.
 *
 * If a live stream is being played, playback is stopped entirely.
 **/
void
bacon_video_widget_pause (BaconVideoWidget * bvw)
{
  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  g_return_if_fail (GST_IS_ELEMENT (bvw->priv->play));
  g_return_if_fail (bvw->priv->mrl != NULL);

  if (bvw->priv->is_live != FALSE) {
    GST_LOG ("Stopping because we have a live stream");
    bacon_video_widget_stop (bvw);
    return;
  }

  GST_LOG ("Pausing");
  gst_element_set_state (GST_ELEMENT (bvw->priv->play), GST_STATE_PAUSED);
  bvw->priv->target_state = GST_STATE_PAUSED;
}

/**
 * bacon_video_widget_can_set_volume:
 * @bvw: a #BaconVideoWidget
 *
 * Returns whether the volume level can be set, given the current settings.
 *
 * The volume cannot be set if the audio output type is set to
 * %BVW_AUDIO_SOUND_AC3PASSTHRU.
 *
 * Return value: %TRUE if the volume can be set, %FALSE otherwise
 **/
gboolean
bacon_video_widget_can_set_volume (BaconVideoWidget * bvw)
{
  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  return !bvw->priv->uses_fakesink;
}

/**
 * bacon_video_widget_set_volume:
 * @bvw: a #BaconVideoWidget
 * @volume: the new volume level, as a percentage between %0 and %1
 *
 * Sets the volume level of the stream as a percentage between %0 and %1.
 *
 * If bacon_video_widget_can_set_volume() returns %FALSE, this is a no-op.
 **/
void
bacon_video_widget_set_volume (BaconVideoWidget * bvw, double volume)
{
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  g_return_if_fail (GST_IS_ELEMENT (bvw->priv->play));

  if (bacon_video_widget_can_set_volume (bvw) != FALSE) {
    volume = CLAMP (volume, 0.0, 1.0);
    g_object_set (bvw->priv->play, "volume", (gdouble) volume, NULL);
    g_object_notify (G_OBJECT (bvw), "volume");
  }
}

/**
 * bacon_video_widget_get_volume:
 * @bvw: a #BaconVideoWidget
 *
 * Returns the current volume level, as a percentage between %0 and %1.
 *
 * Return value: the volume as a percentage between %0 and %1
 **/
double
bacon_video_widget_get_volume (BaconVideoWidget * bvw)
{
  double vol;

  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), 0.0);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), 0.0);

  g_object_get (G_OBJECT (bvw->priv->play), "volume", &vol, NULL);

  return vol;
}

/**
 * bacon_video_widget_set_fullscreen:
 * @bvw: a #BaconVideoWidget
 * @fullscreen: %TRUE to go fullscreen, %FALSE otherwise
 *
 * Sets whether the widget renders the stream in fullscreen mode.
 **/
void
bacon_video_widget_set_fullscreen (BaconVideoWidget * bvw, gboolean fullscreen)
{
  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));

  bvw->priv->fullscreen_mode = fullscreen;
}

/* Search for the color balance channel corresponding to type and return it. */
static GstColorBalanceChannel *
bvw_get_color_balance_channel (GstColorBalance * color_balance,
    BvwVideoProperty type)
{
  const GList *channels;

  channels = gst_color_balance_list_channels (color_balance);

  for (; channels != NULL; channels = channels->next) {
    GstColorBalanceChannel *c = channels->data;

    if (type == BVW_VIDEO_BRIGHTNESS && g_strrstr (c->label, "BRIGHTNESS"))
      return g_object_ref (c);
    else if (type == BVW_VIDEO_CONTRAST && g_strrstr (c->label, "CONTRAST"))
      return g_object_ref (c);
    else if (type == BVW_VIDEO_SATURATION && g_strrstr (c->label, "SATURATION"))
      return g_object_ref (c);
    else if (type == BVW_VIDEO_HUE && g_strrstr (c->label, "HUE"))
      return g_object_ref (c);
  }

  return NULL;
}

/**
 * bacon_video_widget_get_video_property:
 * @bvw: a #BaconVideoWidget
 * @type: the type of property
 *
 * Returns the given property of the video, such as its brightness or saturation.
 *
 * It is returned as a percentage in the full range of integer values; from %0
 * to %G_MAXINT, where %G_MAXINT/2 is the default.
 *
 * Return value: the property's value, in the range %0 to %G_MAXINT
 **/
int
bacon_video_widget_get_video_property (BaconVideoWidget * bvw,
    BvwVideoProperty type)
{
  int ret;

  g_return_val_if_fail (bvw != NULL, 65535 / 2);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), 65535 / 2);

  ret = 0;

  if (bvw->priv->balance && GST_IS_COLOR_BALANCE (bvw->priv->balance)) {
    GstColorBalanceChannel *found_channel = NULL;

    found_channel = bvw_get_color_balance_channel (bvw->priv->balance, type);

    if (found_channel && GST_IS_COLOR_BALANCE_CHANNEL (found_channel)) {
      gint cur;

      cur = gst_color_balance_get_value (bvw->priv->balance, found_channel);

      GST_DEBUG ("channel %s: cur=%d, min=%d, max=%d",
          found_channel->label, cur, found_channel->min_value,
          found_channel->max_value);

      ret = floor (0.5 +
          ((double) cur - found_channel->min_value) * 65535 /
          ((double) found_channel->max_value - found_channel->min_value));

      GST_DEBUG ("channel %s: returning value %d", found_channel->label, ret);
      g_object_unref (found_channel);
      goto done;
    } else {
      ret = -1;
    }
  }

done:

  return ret;
}

/**
 * bacon_video_widget_set_video_property:
 * @bvw: a #BaconVideoWidget
 * @type: the type of property
 * @value: the property's value, in the range %0 to %G_MAXINT
 *
 * Sets the given property of the video, such as its brightness or saturation.
 *
 * It should be given as a percentage in the full range of integer values; from %0
 * to %G_MAXINT, where %G_MAXINT/2 is the default.
 **/
void
bacon_video_widget_set_video_property (BaconVideoWidget * bvw,
    BvwVideoProperty type, int value)
{
  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));

  GST_DEBUG ("set video property type %d to value %d", type, value);

  if (!(value <= 65535 && value >= 0))
    return;

  if (bvw->priv->balance && GST_IS_COLOR_BALANCE (bvw->priv->balance)) {
    GstColorBalanceChannel *found_channel = NULL;

    found_channel = bvw_get_color_balance_channel (bvw->priv->balance, type);

    if (found_channel && GST_IS_COLOR_BALANCE_CHANNEL (found_channel)) {
      int i_value;

      i_value = floor (0.5 + value * ((double) found_channel->max_value -
              found_channel->min_value) / 65535 + found_channel->min_value);

      GST_DEBUG ("channel %s: set to %d/65535", found_channel->label, value);

      gst_color_balance_set_value (bvw->priv->balance, found_channel, i_value);

      GST_DEBUG ("channel %s: val=%d, min=%d, max=%d",
          found_channel->label, i_value, found_channel->min_value,
          found_channel->max_value);

      g_object_unref (found_channel);
    }
  }
}

/**
 * bacon_video_widget_get_position:
 * @bvw: a #BaconVideoWidget
 *
 * Returns the current position in the stream, as a value between
 * %0 and %1.
 *
 * Return value: the current position, or %-1
 **/
double
bacon_video_widget_get_position (BaconVideoWidget * bvw)
{
  g_return_val_if_fail (bvw != NULL, -1);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), -1);
  return bvw->priv->current_position;
}

/**
 * bacon_video_widget_get_current_time:
 * @bvw: a #BaconVideoWidget
 *
 * Returns the current position in the stream, as the time (in milliseconds)
 * since the beginning of the stream.
 *
 * Return value: time since the beginning of the stream, in milliseconds, or %-1
 **/
gint64
bacon_video_widget_get_current_time (BaconVideoWidget * bvw)
{
  g_return_val_if_fail (bvw != NULL, -1);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), -1);
  return bvw->priv->current_time;
}

/**
 * bacon_video_widget_get_accurate_current_time:
 * @bvw: a #BaconVideoWidget
 *
 * Returns the current position in the stream, as the time (in milliseconds)
 * since the beginning of the stream.
 *
 * Return value: time since the beginning of the stream querying directly to the pipeline, in milliseconds, or %-1
 **/
gint64
bacon_video_widget_get_accurate_current_time (BaconVideoWidget * bvw)
{
  GstFormat fmt;
  gint64 pos;

  g_return_val_if_fail (bvw != NULL, -1);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), -1);

  fmt = GST_FORMAT_TIME;
  pos = -1;

  gst_element_query_position (bvw->priv->play, &fmt, &pos);

  return pos / GST_MSECOND;

}



/**
 * bacon_video_widget_get_stream_length:
 * @bvw: a #BaconVideoWidget
 *
 * Returns the total length of the stream, in milliseconds.
 *
 * Return value: the stream length, in milliseconds, or %-1
 **/
gint64
bacon_video_widget_get_stream_length (BaconVideoWidget * bvw)
{
  g_return_val_if_fail (bvw != NULL, -1);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), -1);

  if (bvw->priv->stream_length == 0 && bvw->priv->play != NULL) {
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 len = -1;

    if (gst_element_query_duration (bvw->priv->play, &fmt, &len)
        && len != -1) {
      bvw->priv->stream_length = len / GST_MSECOND;
    }
  }

  return bvw->priv->stream_length;
}

/**
 * bacon_video_widget_is_playing:
 * @bvw: a #BaconVideoWidget
 *
 * Returns whether the widget is currently playing a stream.
 *
 * Return value: %TRUE if a stream is playing, %FALSE otherwise
 **/
gboolean
bacon_video_widget_is_playing (BaconVideoWidget * bvw)
{
  gboolean ret;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  ret = (bvw->priv->target_state == GST_STATE_PLAYING);
  GST_LOG ("%splaying", (ret) ? "" : "not ");

  return ret;
}

/**
 * bacon_video_widget_is_seekable:
 * @bvw: a #BaconVideoWidget
 *
 * Returns whether seeking is possible in the current stream.
 *
 * If no stream is loaded, %FALSE is returned.
 *
 * Return value: %TRUE if the stream is seekable, %FALSE otherwise
 **/
gboolean
bacon_video_widget_is_seekable (BaconVideoWidget * bvw)
{
  gboolean res;
  gint old_seekable;

  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  if (bvw->priv->mrl == NULL)
    return FALSE;

  old_seekable = bvw->priv->seekable;

  if (bvw->priv->seekable == -1) {
    GstQuery *query;

    query = gst_query_new_seeking (GST_FORMAT_TIME);
    if (gst_element_query (bvw->priv->play, query)) {
      gst_query_parse_seeking (query, NULL, &res, NULL, NULL);
      bvw->priv->seekable = (res) ? 1 : 0;
    } else {
      GST_DEBUG ("seeking query failed");
    }
    gst_query_unref (query);
  }

  if (bvw->priv->seekable != -1) {
    res = (bvw->priv->seekable != 0);
    goto done;
  }

  /* try to guess from duration (this is very unreliable though) */
  if (bvw->priv->stream_length == 0) {
    res = (bacon_video_widget_get_stream_length (bvw) > 0);
  } else {
    res = (bvw->priv->stream_length > 0);
  }

done:

  if (old_seekable != bvw->priv->seekable)
    g_object_notify (G_OBJECT (bvw), "seekable");

  GST_DEBUG ("stream is%s seekable", (res) ? "" : " not");
  return res;
}


static struct _metadata_map_info
{
  BvwMetadataType type;
  const gchar *str;
} metadata_str_map[] = {
  {
  BVW_INFO_TITLE, "title"}, {
  BVW_INFO_ARTIST, "artist"}, {
  BVW_INFO_YEAR, "year"}, {
  BVW_INFO_COMMENT, "comment"}, {
  BVW_INFO_ALBUM, "album"}, {
  BVW_INFO_DURATION, "duration"}, {
  BVW_INFO_TRACK_NUMBER, "track-number"}, {
  BVW_INFO_HAS_VIDEO, "has-video"}, {
  BVW_INFO_DIMENSION_X, "dimension-x"}, {
  BVW_INFO_DIMENSION_Y, "dimension-y"}, {
  BVW_INFO_VIDEO_BITRATE, "video-bitrate"}, {
  BVW_INFO_VIDEO_CODEC, "video-codec"}, {
  BVW_INFO_FPS, "fps"}, {
  BVW_INFO_HAS_AUDIO, "has-audio"}, {
  BVW_INFO_AUDIO_BITRATE, "audio-bitrate"}, {
  BVW_INFO_AUDIO_CODEC, "audio-codec"}, {
  BVW_INFO_AUDIO_SAMPLE_RATE, "samplerate"}, {
  BVW_INFO_AUDIO_CHANNELS, "channels"}
};

static const gchar *
get_metadata_type_name (BvwMetadataType type)
{
  guint i;
  for (i = 0; i < G_N_ELEMENTS (metadata_str_map); ++i) {
    if (metadata_str_map[i].type == type)
      return metadata_str_map[i].str;
  }
  return "unknown";
}

static gint
bvw_get_current_stream_num (BaconVideoWidget * bvw, const gchar * stream_type)
{
  gchar *lower, *cur_prop_str;
  gint stream_num = -1;

  if (bvw->priv->play == NULL)
    return stream_num;

  lower = g_ascii_strdown (stream_type, -1);
  cur_prop_str = g_strconcat ("current-", lower, NULL);
  g_object_get (bvw->priv->play, cur_prop_str, &stream_num, NULL);
  g_free (cur_prop_str);
  g_free (lower);

  GST_LOG ("current %s stream: %d", stream_type, stream_num);
  return stream_num;
}

static GstTagList *
bvw_get_tags_of_current_stream (BaconVideoWidget * bvw,
    const gchar * stream_type)
{
  GstTagList *tags = NULL;
  gint stream_num = -1;
  gchar *lower, *cur_sig_str;

  stream_num = bvw_get_current_stream_num (bvw, stream_type);
  if (stream_num < 0)
    return NULL;

  lower = g_ascii_strdown (stream_type, -1);
  cur_sig_str = g_strconcat ("get-", lower, "-tags", NULL);
  g_signal_emit_by_name (bvw->priv->play, cur_sig_str, stream_num, &tags);
  g_free (cur_sig_str);
  g_free (lower);

  GST_LOG ("current %s stream tags %" GST_PTR_FORMAT, stream_type, tags);
  return tags;
}

static GstCaps *
bvw_get_caps_of_current_stream (BaconVideoWidget * bvw,
    const gchar * stream_type)
{
  GstCaps *caps = NULL;
  gint stream_num = -1;
  GstPad *current;
  gchar *lower, *cur_sig_str;

  stream_num = bvw_get_current_stream_num (bvw, stream_type);
  if (stream_num < 0)
    return NULL;

  lower = g_ascii_strdown (stream_type, -1);
  cur_sig_str = g_strconcat ("get-", lower, "-pad", NULL);
  g_signal_emit_by_name (bvw->priv->play, cur_sig_str, stream_num, &current);
  g_free (cur_sig_str);
  g_free (lower);

  if (current != NULL) {
    caps = gst_pad_get_negotiated_caps (current);
    gst_object_unref (current);
  }
  GST_LOG ("current %s stream caps: %" GST_PTR_FORMAT, stream_type, caps);
  return caps;
}

static gboolean
audio_caps_have_LFE (GstStructure * s)
{
  GstAudioChannelPosition *positions;
  gint i, channels;

  if (!gst_structure_get_value (s, "channel-positions") ||
      !gst_structure_get_int (s, "channels", &channels)) {
    return FALSE;
  }

  positions = gst_audio_get_channel_positions (s);
  if (positions == NULL)
    return FALSE;

  for (i = 0; i < channels; ++i) {
    if (positions[i] == GST_AUDIO_CHANNEL_POSITION_LFE) {
      g_free (positions);
      return TRUE;
    }
  }

  g_free (positions);
  return FALSE;
}

static void
bacon_video_widget_get_metadata_string (BaconVideoWidget * bvw,
    BvwMetadataType type, GValue * value)
{
  char *string = NULL;
  gboolean res = FALSE;

  g_value_init (value, G_TYPE_STRING);

  if (bvw->priv->play == NULL) {
    g_value_set_string (value, NULL);
    return;
  }

  switch (type) {
    case BVW_INFO_TITLE:
      if (bvw->priv->tagcache != NULL) {
        res = gst_tag_list_get_string_index (bvw->priv->tagcache,
            GST_TAG_TITLE, 0, &string);
      }
      break;
    case BVW_INFO_ARTIST:
      if (bvw->priv->tagcache != NULL) {
        res = gst_tag_list_get_string_index (bvw->priv->tagcache,
            GST_TAG_ARTIST, 0, &string);
      }
      break;
    case BVW_INFO_YEAR:
      if (bvw->priv->tagcache != NULL) {
        GDate *date;

        if ((res = gst_tag_list_get_date (bvw->priv->tagcache,
                    GST_TAG_DATE, &date))) {
          string = g_strdup_printf ("%d", g_date_get_year (date));
          g_date_free (date);
        }
      }
      break;
    case BVW_INFO_COMMENT:
      if (bvw->priv->tagcache != NULL) {
        res = gst_tag_list_get_string_index (bvw->priv->tagcache,
            GST_TAG_COMMENT, 0, &string);
      }
      break;
    case BVW_INFO_ALBUM:
      if (bvw->priv->tagcache != NULL) {
        res = gst_tag_list_get_string_index (bvw->priv->tagcache,
            GST_TAG_ALBUM, 0, &string);
      }
      break;
    case BVW_INFO_VIDEO_CODEC:
    {
      GstTagList *tags;

      /* try to get this from the stream info first */
      if ((tags = bvw_get_tags_of_current_stream (bvw, "video"))) {
        res = gst_tag_list_get_string (tags, GST_TAG_CODEC, &string);
        gst_tag_list_free (tags);
      }

      /* if that didn't work, try the aggregated tags */
      if (!res && bvw->priv->tagcache != NULL) {
        res = gst_tag_list_get_string (bvw->priv->tagcache,
            GST_TAG_VIDEO_CODEC, &string);
      }
      break;
    }
    case BVW_INFO_AUDIO_CODEC:
    {
      GstTagList *tags;

      /* try to get this from the stream info first */
      if ((tags = bvw_get_tags_of_current_stream (bvw, "audio"))) {
        res = gst_tag_list_get_string (tags, GST_TAG_CODEC, &string);
        gst_tag_list_free (tags);
      }

      /* if that didn't work, try the aggregated tags */
      if (!res && bvw->priv->tagcache != NULL) {
        res = gst_tag_list_get_string (bvw->priv->tagcache,
            GST_TAG_AUDIO_CODEC, &string);
      }
      break;
    }
    case BVW_INFO_AUDIO_CHANNELS:
    {
      GstStructure *s;
      GstCaps *caps;

      caps = bvw_get_caps_of_current_stream (bvw, "audio");
      if (caps) {
        gint channels = 0;

        s = gst_caps_get_structure (caps, 0);
        if ((res = gst_structure_get_int (s, "channels", &channels))) {
          /* FIXME: do something more sophisticated - but what? */
          if (channels > 2 && audio_caps_have_LFE (s)) {
            string = g_strdup_printf ("%s %d.1", _("Surround"), channels - 1);
          } else if (channels == 1) {
            string = g_strdup (_("Mono"));
          } else if (channels == 2) {
            string = g_strdup (_("Stereo"));
          } else {
            string = g_strdup_printf ("%d", channels);
          }
        }
        gst_caps_unref (caps);
      }
      break;
    }
    default:
      g_assert_not_reached ();
  }

  /* Remove line feeds */
  if (string && strstr (string, "\n") != NULL)
    g_strdelimit (string, "\n", ' ');

  if (res && string && g_utf8_validate (string, -1, NULL)) {
    g_value_take_string (value, string);
    GST_DEBUG ("%s = '%s'", get_metadata_type_name (type), string);
  } else {
    g_value_set_string (value, NULL);
    g_free (string);
  }

  return;
}

static void
bacon_video_widget_get_metadata_int (BaconVideoWidget * bvw,
    BvwMetadataType type, GValue * value)
{
  int integer = 0;

  g_value_init (value, G_TYPE_INT);

  if (bvw->priv->play == NULL) {
    g_value_set_int (value, 0);
    return;
  }

  switch (type) {
    case BVW_INFO_DURATION:
      integer = bacon_video_widget_get_stream_length (bvw) / 1000;
      break;
    case BVW_INFO_TRACK_NUMBER:
      if (bvw->priv->tagcache == NULL)
        break;
      if (!gst_tag_list_get_uint (bvw->priv->tagcache,
              GST_TAG_TRACK_NUMBER, (guint *) & integer))
        integer = 0;
      break;
    case BVW_INFO_DIMENSION_X:
      integer = bvw->priv->video_width;
      break;
    case BVW_INFO_DIMENSION_Y:
      integer = bvw->priv->video_height;
      break;
    case BVW_INFO_FPS:
      if (bvw->priv->video_fps_d > 0) {
        /* Round up/down to the nearest integer framerate */
        integer = (bvw->priv->video_fps_n + bvw->priv->video_fps_d / 2) /
            bvw->priv->video_fps_d;
      } else
        integer = 0;
      break;
    case BVW_INFO_AUDIO_BITRATE:
      if (bvw->priv->audiotags == NULL)
        break;
      if (gst_tag_list_get_uint (bvw->priv->audiotags, GST_TAG_BITRATE,
              (guint *) & integer) ||
          gst_tag_list_get_uint (bvw->priv->audiotags,
              GST_TAG_NOMINAL_BITRATE, (guint *) & integer)) {
        integer /= 1000;
      }
      break;
    case BVW_INFO_VIDEO_BITRATE:
      if (bvw->priv->videotags == NULL)
        break;
      if (gst_tag_list_get_uint (bvw->priv->videotags, GST_TAG_BITRATE,
              (guint *) & integer) ||
          gst_tag_list_get_uint (bvw->priv->videotags,
              GST_TAG_NOMINAL_BITRATE, (guint *) & integer)) {
        integer /= 1000;
      }
      break;
    case BVW_INFO_AUDIO_SAMPLE_RATE:
    {
      GstStructure *s;
      GstCaps *caps;

      caps = bvw_get_caps_of_current_stream (bvw, "audio");
      if (caps) {
        s = gst_caps_get_structure (caps, 0);
        gst_structure_get_int (s, "rate", &integer);
        gst_caps_unref (caps);
      }
      break;
    }
    default:
      g_assert_not_reached ();
  }

  g_value_set_int (value, integer);
  GST_DEBUG ("%s = %d", get_metadata_type_name (type), integer);

  return;
}

static void
bacon_video_widget_get_metadata_bool (BaconVideoWidget * bvw,
    BvwMetadataType type, GValue * value)
{
  gboolean boolean = FALSE;

  g_value_init (value, G_TYPE_BOOLEAN);

  if (bvw->priv->play == NULL) {
    g_value_set_boolean (value, FALSE);
    return;
  }

  GST_DEBUG ("tagcache  = %" GST_PTR_FORMAT, bvw->priv->tagcache);
  GST_DEBUG ("videotags = %" GST_PTR_FORMAT, bvw->priv->videotags);
  GST_DEBUG ("audiotags = %" GST_PTR_FORMAT, bvw->priv->audiotags);

  switch (type) {
    case BVW_INFO_HAS_VIDEO:
      boolean = bvw->priv->media_has_video;
      /* if properties dialog, show the metadata we
       * have even if we cannot decode the stream */
      if (!boolean && bvw->priv->use_type == BVW_USE_TYPE_METADATA &&
          bvw->priv->tagcache != NULL &&
          gst_structure_has_field ((GstStructure *) bvw->priv->tagcache,
              GST_TAG_VIDEO_CODEC)) {
        boolean = TRUE;
      }
      break;
    case BVW_INFO_HAS_AUDIO:
      boolean = bvw->priv->media_has_audio;
      /* if properties dialog, show the metadata we
       * have even if we cannot decode the stream */
      if (!boolean && bvw->priv->use_type == BVW_USE_TYPE_METADATA &&
          bvw->priv->tagcache != NULL &&
          gst_structure_has_field ((GstStructure *) bvw->priv->tagcache,
              GST_TAG_AUDIO_CODEC)) {
        boolean = TRUE;
      }
      break;
    default:
      g_assert_not_reached ();
  }

  g_value_set_boolean (value, boolean);
  GST_DEBUG ("%s = %s", get_metadata_type_name (type),
      (boolean) ? "yes" : "no");

  return;
}

static void
bvw_process_pending_tag_messages (BaconVideoWidget * bvw)
{
  GstMessageType events;
  GstMessage *msg;
  GstBus *bus;

  /* process any pending tag messages on the bus NOW, so we can get to
   * the information without/before giving control back to the main loop */

  /* application message is for stream-info */
  events = GST_MESSAGE_TAG | GST_MESSAGE_DURATION | GST_MESSAGE_APPLICATION;
  bus = gst_element_get_bus (bvw->priv->play);
  while ((msg = gst_bus_poll (bus, events, 0))) {
    gst_bus_async_signal_func (bus, msg, NULL);
  }
  gst_object_unref (bus);
}


/**
 * bacon_video_widget_get_metadata:
 * @bvw: a #BaconVideoWidget
 * @type: the type of metadata to return
 * @value: a #GValue
 *
 * Provides metadata of the given @type about the current stream in @value.
 *
 * Free the #GValue with g_value_unset().
 **/
void
bacon_video_widget_get_metadata (BaconVideoWidget * bvw,
    BvwMetadataType type, GValue * value)
{
  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  g_return_if_fail (GST_IS_ELEMENT (bvw->priv->play));

  switch (type) {
    case BVW_INFO_TITLE:
    case BVW_INFO_ARTIST:
    case BVW_INFO_YEAR:
    case BVW_INFO_COMMENT:
    case BVW_INFO_ALBUM:
    case BVW_INFO_VIDEO_CODEC:
      bacon_video_widget_get_metadata_string (bvw, type, value);
      break;
    case BVW_INFO_AUDIO_CODEC:
      bacon_video_widget_get_metadata_string (bvw, type, value);
      break;
    case BVW_INFO_AUDIO_CHANNELS:
      bacon_video_widget_get_metadata_string (bvw, type, value);
      break;
    case BVW_INFO_DURATION:
      bacon_video_widget_get_metadata_int (bvw, type, value);
      break;
    case BVW_INFO_DIMENSION_X:
      bacon_video_widget_get_metadata_int (bvw, type, value);
      break;
    case BVW_INFO_DIMENSION_Y:
      bacon_video_widget_get_metadata_int (bvw, type, value);
      break;
    case BVW_INFO_FPS:
      bacon_video_widget_get_metadata_int (bvw, type, value);
      break;
    case BVW_INFO_AUDIO_BITRATE:
      bacon_video_widget_get_metadata_int (bvw, type, value);
      break;
    case BVW_INFO_VIDEO_BITRATE:
      bacon_video_widget_get_metadata_int (bvw, type, value);
      break;
    case BVW_INFO_TRACK_NUMBER:
    case BVW_INFO_AUDIO_SAMPLE_RATE:
      bacon_video_widget_get_metadata_int (bvw, type, value);
      break;
    case BVW_INFO_HAS_VIDEO:
      bacon_video_widget_get_metadata_bool (bvw, type, value);
      break;
    case BVW_INFO_HAS_AUDIO:
      bacon_video_widget_get_metadata_bool (bvw, type, value);
      break;
    case BVW_INFO_COVER:
      break;
    default:
      g_return_if_reached ();
  }

  return;
}

/* Screenshot functions */

/**
 * bacon_video_widget_can_get_frames:
 * @bvw: a #BaconVideoWidget
 * @error: a #GError, or %NULL
 *
 * Determines whether individual frames from the current stream can
 * be returned using bacon_video_widget_get_current_frame().
 *
 * Frames cannot be returned for audio-only streams, unless visualisations
 * are enabled.
 *
 * Return value: %TRUE if frames can be captured, %FALSE otherwise
 **/
gboolean
bacon_video_widget_can_get_frames (BaconVideoWidget * bvw, GError ** error)
{
  g_return_val_if_fail (bvw != NULL, FALSE);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), FALSE);

  /* check for video */
  if (!bvw->priv->media_has_video) {
    g_set_error_literal (error, BVW_ERROR, BVW_ERROR_GENERIC,
        _("Media contains no supported video streams."));
    return FALSE;
  }

  return TRUE;
}

void
bacon_video_widget_unref_pixbuf (GdkPixbuf * pixbuf)
{
  g_object_unref (pixbuf);
}

/**
 * bacon_video_widget_get_current_frame:
 * @bvw: a #BaconVideoWidget
 *
 * Returns a #GdkPixbuf containing the current frame from the playing
 * stream. This will wait for any pending seeks to complete before
 * capturing the frame.
 *
 * Return value: the current frame, or %NULL; unref with g_object_unref()
 **/
GdkPixbuf *
bacon_video_widget_get_current_frame (BaconVideoWidget * bvw)
{
  g_return_val_if_fail (bvw != NULL, NULL);
  g_return_val_if_fail (BACON_IS_VIDEO_WIDGET (bvw), NULL);
  g_return_val_if_fail (GST_IS_ELEMENT (bvw->priv->play), NULL);

  /* no video info */
  if (!bvw->priv->video_width || !bvw->priv->video_height) {
    GST_DEBUG ("Could not take screenshot: %s", "no video info");
    g_warning ("Could not take screenshot: %s", "no video info");
    return NULL;
  }

  return gst_playbin_get_frame (bvw->priv->play);
}

void
bacon_video_widget_set_drawing_pixbuf (BaconVideoWidget * bvw,
    const GdkPixbuf * drawings)
{
  g_return_if_fail (bvw != NULL);
  g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
  g_return_if_fail (bvw->priv->drawings != NULL);

  if (!draw_pixbuf (bvw, drawings, bvw->priv->drawings)) {
    clutter_actor_hide (CLUTTER_ACTOR (bvw->priv->drawings_frame));
  } else {
    clutter_actor_show (CLUTTER_ACTOR (bvw->priv->drawings_frame));
  }
}

/* =========================================== */
/*                                             */
/*          Widget typing & Creation           */
/*                                             */
/* =========================================== */

G_DEFINE_TYPE (BaconVideoWidget, bacon_video_widget, GTK_CLUTTER_TYPE_EMBED)

/**
 * bacon_video_widget_init_backend:
 * @argc: pointer to application's argc
 * @argv: pointer to application's argv
 *
 * Initialises #BaconVideoWidget's GStreamer backend. If this fails
 * for the GStreamer backend, your application will be terminated.
 *
 * Applications must call either this or bacon_video_widget_get_option_group() exactly
 * once; but not both.
 **/
     void bacon_video_widget_init_backend (int *argc, char ***argv)
{
  gst_init (argc, argv);
}

GQuark
bacon_video_widget_error_quark (void)
{
  static GQuark q;              /* 0 */

  if (G_UNLIKELY (q == 0)) {
    q = g_quark_from_static_string ("bvw-error-quark");
  }
  return q;
}

/**
 * bacon_video_widget_new:
 * @error: a #GError, or %NULL
 *
 * Creates a new #BaconVideoWidget.
 *
 * A #BvwError will be returned on error.
 *
 * Return value: a new #BaconVideoWidget, or %NULL; destroy with gtk_widget_destroy()
 **/
GtkWidget *
bacon_video_widget_new (BvwUseType use_type, GError ** err)
{
  BaconVideoWidget *bvw;
  GstElement *audio_sink = NULL, *video_sink = NULL;
  gchar *version_str;
  GstPlayFlags flags;
  ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
  ClutterConstraint *constraint;
  GstElement *balance, *sink, *bin;
  GstPad *pad, *ghostpad;

#ifndef GST_DISABLE_GST_DEBUG
  if (_longomatch_gst_debug_cat == NULL) {
    GST_DEBUG_CATEGORY_INIT (_longomatch_gst_debug_cat, "longomatch", 0,
        "LongoMatch GStreamer Backend");
  }
#endif

  version_str = gst_version_string ();
  GST_DEBUG ("Initialised %s", version_str);
  g_free (version_str);

  gst_pb_utils_init ();

  bvw = BACON_VIDEO_WIDGET (g_object_new
      (bacon_video_widget_get_type (), NULL));

  bvw->priv->use_type = use_type;
  bvw->priv->play = gst_element_factory_make ("playbin2", "play");
  if (!bvw->priv->play) {
    g_set_error_literal (err, BVW_ERROR, BVW_ERROR_PLUGIN_LOAD,
        _("Failed to create a GStreamer play object. "
            "Please check your GStreamer installation."));
    g_object_ref_sink (bvw);
    g_object_unref (bvw);
    return NULL;
  }

  bvw->priv->bus = gst_element_get_bus (bvw->priv->play);

  /* Add the download flag, for streaming buffering,
   * and the deinterlace flag, for video only */
  g_object_get (bvw->priv->play, "flags", &flags, NULL);
  flags |= GST_PLAY_FLAG_DOWNLOAD;
  g_object_set (bvw->priv->play, "flags", flags, NULL);
  flags |= GST_PLAY_FLAG_DEINTERLACE;
  g_object_set (bvw->priv->play, "flags", flags, NULL);

  gst_bus_add_signal_watch (bvw->priv->bus);

  bvw->priv->sig_bus_async =
      g_signal_connect (bvw->priv->bus, "message",
      G_CALLBACK (bvw_bus_message_cb), bvw);

  bvw->priv->cursor_shown = TRUE;
  bvw->priv->logo_mode = FALSE;
  bvw->priv->auto_resize = FALSE;

  audio_sink = gst_element_factory_make ("autoaudiosink", "audio-sink");


  bvw->priv->stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (bvw));
  clutter_stage_set_color (CLUTTER_STAGE (bvw->priv->stage), &black);

  /* Bin */
  bin = gst_bin_new ("video_sink_bin");

  /* Video sink, with aspect frame */
  bvw->priv->texture = g_object_new (CLUTTER_TYPE_TEXTURE,
      "disable-slicing", TRUE, NULL);
  sink = clutter_gst_video_sink_new (CLUTTER_TEXTURE (bvw->priv->texture));
  if (sink == NULL)
    g_critical ("Could not create Clutter video sink");

  /* The logo */
  bvw->priv->logo_frame = longomatch_aspect_frame_new ();
  clutter_actor_set_name (bvw->priv->logo_frame, "logo-frame");
  bvw->priv->logo = clutter_texture_new ();
  mx_bin_set_child (MX_BIN (bvw->priv->logo_frame), bvw->priv->logo);
  clutter_container_add_actor (CLUTTER_CONTAINER (bvw->priv->stage),
      bvw->priv->logo_frame);
  mx_bin_set_fill (MX_BIN (bvw->priv->logo_frame), FALSE, FALSE);
  mx_bin_set_alignment (MX_BIN (bvw->priv->logo_frame), MX_ALIGN_MIDDLE,
      MX_ALIGN_MIDDLE);
  clutter_actor_set_size (bvw->priv->logo, LOGO_SIZE, LOGO_SIZE);
  constraint =
      clutter_bind_constraint_new (bvw->priv->stage, CLUTTER_BIND_SIZE, 0.0);
  clutter_actor_add_constraint_with_name (bvw->priv->logo_frame, "size",
      constraint);
  clutter_actor_hide (CLUTTER_ACTOR (bvw->priv->logo_frame));

  /* The video */
  bvw->priv->frame = longomatch_aspect_frame_new ();
  clutter_actor_set_name (bvw->priv->frame, "frame");
  mx_bin_set_child (MX_BIN (bvw->priv->frame), bvw->priv->texture);

  clutter_container_add_actor (CLUTTER_CONTAINER (bvw->priv->stage),
      bvw->priv->frame);
  constraint =
      clutter_bind_constraint_new (bvw->priv->stage, CLUTTER_BIND_SIZE, 0.0);
  clutter_actor_add_constraint_with_name (bvw->priv->frame, "size", constraint);

  /* The drawings */
  bvw->priv->drawings = clutter_texture_new ();
  bvw->priv->drawings_frame = longomatch_aspect_frame_new ();
  clutter_actor_set_name (bvw->priv->drawings_frame, "drawings");
  mx_bin_set_child (MX_BIN (bvw->priv->drawings_frame), bvw->priv->drawings);

  clutter_container_add_actor (CLUTTER_CONTAINER (bvw->priv->stage),
      bvw->priv->drawings_frame);
  constraint =
      clutter_bind_constraint_new (bvw->priv->stage, CLUTTER_BIND_SIZE, 0.0);
  clutter_actor_add_constraint_with_name (bvw->priv->drawings_frame, "size",
      constraint);

  clutter_actor_raise (CLUTTER_ACTOR (bvw->priv->drawings_frame),
      CLUTTER_ACTOR (bvw->priv->frame));
  clutter_actor_raise (CLUTTER_ACTOR (bvw->priv->frame),
      CLUTTER_ACTOR (bvw->priv->logo_frame));

  gst_bin_add (GST_BIN (bin), sink);

  /* Add video balance */
  balance = gst_element_factory_make ("videobalance", "video_balance");
  gst_bin_add (GST_BIN (bin), balance);
  bvw->priv->balance = GST_COLOR_BALANCE (balance);
  pad = gst_element_get_static_pad (balance, "sink");
  ghostpad = gst_ghost_pad_new ("sink", pad);
  gst_element_add_pad (bin, ghostpad);

  gst_element_link (balance, sink);

  video_sink = bin;
  gst_element_set_state (video_sink, GST_STATE_READY);

  if (audio_sink) {
    GstStateChangeReturn ret;
    GstBus *bus;

    /* need to set bus explicitly as it's not in a bin yet and
     * we need one to catch error messages */
    bus = gst_bus_new ();
    gst_element_set_bus (audio_sink, bus);

    /* state change NULL => READY should always be synchronous */
    ret = gst_element_set_state (audio_sink, GST_STATE_READY);
    gst_element_set_bus (audio_sink, NULL);

    if (ret == GST_STATE_CHANGE_FAILURE) {
      /* doesn't work, drop this audio sink */
      gst_element_set_state (audio_sink, GST_STATE_NULL);
      gst_object_unref (audio_sink);
      /* Hopefully, fakesink should always work */
      audio_sink = gst_element_factory_make ("fakesink", "audio-sink");
      if (audio_sink == NULL) {
        GstMessage *err_msg;

        err_msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
        if (err_msg == NULL) {
          g_warning ("Should have gotten an error message, please file a bug.");
          g_set_error_literal (err, BVW_ERROR, BVW_ERROR_AUDIO_PLUGIN,
              _("Failed to open audio output. You may not have "
                  "permission to open the sound device, or the sound "
                  "server may not be running. "
                  "Please select another audio output in the Multimedia "
                  "Systems Selector."));
        } else if (err) {
          *err = bvw_error_from_gst_error (bvw, err_msg);
          gst_message_unref (err_msg);
        }
        gst_object_unref (bus);
        goto sink_error;
      }
      /* make fakesink sync to the clock like a real sink */
      g_object_set (audio_sink, "sync", TRUE, NULL);
      GST_DEBUG ("audio sink doesn't work, using fakesink instead");
    }
    gst_object_unref (bus);
  } else {
    g_set_error_literal (err, BVW_ERROR, BVW_ERROR_AUDIO_PLUGIN,
        _("Could not find the audio output. "
            "You may need to install additional GStreamer plugins, or "
            "select another audio output in the Multimedia Systems "
            "Selector."));
    goto sink_error;
  }

  /* set back to NULL to close device again in order to avoid interrupts
   * being generated after startup while there's nothing to play yet
   * FIXME not needed anymore, PulseAudio? */
  gst_element_set_state (audio_sink, GST_STATE_NULL);

  /* now tell playbin */
  g_object_set (bvw->priv->play, "video-sink", video_sink, NULL);
  g_object_set (bvw->priv->play, "audio-sink", audio_sink, NULL);

  g_signal_connect (bvw->priv->play, "notify::source",
      G_CALLBACK (playbin_source_notify_cb), bvw);
  g_signal_connect (bvw->priv->play, "video-changed",
      G_CALLBACK (playbin_stream_changed_cb), bvw);
  g_signal_connect (bvw->priv->play, "audio-changed",
      G_CALLBACK (playbin_stream_changed_cb), bvw);
  g_signal_connect (bvw->priv->play, "text-changed",
      G_CALLBACK (playbin_stream_changed_cb), bvw);

  return GTK_WIDGET (bvw);

  /* errors */
sink_error:
  {
    if (video_sink) {
      gst_element_set_state (video_sink, GST_STATE_NULL);
      gst_object_unref (video_sink);
    }
    if (audio_sink) {
      gst_element_set_state (audio_sink, GST_STATE_NULL);
      gst_object_unref (audio_sink);
    }

    g_object_ref (bvw);
    g_object_ref_sink (G_OBJECT (bvw));
    g_object_unref (bvw);

    return NULL;
  }
}
