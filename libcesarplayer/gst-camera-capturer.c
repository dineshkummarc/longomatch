/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
* Gstreamer DV capturer
* Copyright (C)  Andoni Morales Alastruey 2008 <ylatuya@gmail.com>
* 
* Gstreamer DV capturer is free software.
* 
* You may redistribute it and/or modify it under the terms of the
* GNU General Public License, as published by the Free Software
* Foundation; either version 2 of the License, or (at your option)
* any later version.
* 
* Gstreamer DV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with foob.  If not, write to:
*       The Free Software Foundation, Inc.,
*       51 Franklin Street, Fifth Floor
*       Boston, MA  02110-1301, USA.
*/

#include <string.h>
#include <stdio.h>

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/interfaces/xoverlay.h>
#include <gst/interfaces/propertyprobe.h>
#include <gst/interfaces/colorbalance.h>

#include "gst-camera-capturer.h"
#include "gst-helpers.h"

/* clutter */
#include <clutter-gst/clutter-gst.h>
#include <mx/mx.h>
#include "longomatch-aspect-frame.h"

/*Default video source*/
#ifdef WIN32
#define DVVIDEOSRC "dshowvideosrc"
#define RAWVIDEOSRC "dshowvideosrc"
#define AUDIOSRC "dshowaudiosrc"
#else
#define DVVIDEOSRC "dv1394src"
#define RAWVIDEOSRC "gsettingsvideosrc"
#define AUDIOSRC "gsettingsaudiosrc"
#define RAWVIDEOSRC_GCONF "gconfvideosrc"
#define AUDIOSRC_GCONF "gconfaudiosrc"
#endif

/* gtk+/gnome */
#ifdef WIN32
#include <gdk/gdkwin32.h>
#else
#include <gdk/gdkx.h>
#endif

#ifdef WIN32
#define DEFAULT_SOURCE_TYPE  GST_CAMERA_CAPTURE_SOURCE_TYPE_DSHOW
#else
#define DEFAULT_SOURCE_TYPE  GST_CAMERA_CAPTURE_SOURCE_TYPE_RAW
#endif

#define LOGO_SIZE_H 166
#define LOGO_SIZE_W 540

typedef enum
{
  GST_CAMERABIN_FLAG_SOURCE_RESIZE = (1 << 0),
  GST_CAMERABIN_FLAG_SOURCE_COLOR_CONVERSION = (1 << 1),
  GST_CAMERABIN_FLAG_VIEWFINDER_COLOR_CONVERSION = (1 << 2),
  GST_CAMERABIN_FLAG_VIEWFINDER_SCALE = (1 << 3),
  GST_CAMERABIN_FLAG_AUDIO_CONVERSION = (1 << 4),
  GST_CAMERABIN_FLAG_DISABLE_AUDIO = (1 << 5),
  GST_CAMERABIN_FLAG_IMAGE_COLOR_CONVERSION = (1 << 6)
} GstCameraBinFlags;

/* Signals */
enum
{
  SIGNAL_ERROR,
  SIGNAL_EOS,
  SIGNAL_STATE_CHANGED,
  SIGNAL_DEVICE_CHANGE,
  LAST_SIGNAL
};

/* Properties */
enum
{
  PROP_0,
  PROP_OUTPUT_HEIGHT,
  PROP_OUTPUT_WIDTH,
  PROP_VIDEO_BITRATE,
  PROP_AUDIO_BITRATE,
  PROP_AUDIO_ENABLED,
  PROP_OUTPUT_FILE,
  PROP_DEVICE_ID,
};

struct GstCameraCapturerPrivate
{
  /*Encoding properties */
  gchar *output_file;
  gchar *device_id;
  guint output_height;
  guint output_width;
  guint output_fps_n;
  guint output_fps_d;
  guint audio_bitrate;
  guint video_bitrate;
  gboolean audio_enabled;
  VideoEncoderType video_encoder_type;
  AudioEncoderType audio_encoder_type;

  /*Video input info */
  gint video_width;             /* Movie width */
  gint video_height;            /* Movie height */
  gint movie_par_n;             /* Movie pixel aspect ratio numerator */
  gint movie_par_d;             /* Movie pixel aspect ratio denominator */
  gint video_width_pixels;      /* Scaled movie width */
  gint video_height_pixels;     /* Scaled movie height */
  gint video_fps_n;
  gint video_fps_d;
  gboolean media_has_video;
  gboolean media_has_audio;
  GstCameraCaptureSourceType source_type;

  /* Snapshots */
  GstBuffer *last_buffer;

  /*GStreamer elements */
  GstElement *main_pipeline;
  GstElement *camerabin;
  GstElement *videosrc;
  GstElement *device_source;
  GstElement *videofilter;
  GstElement *audiosrc;
  GstElement *videoenc;
  GstElement *audioenc;
  GstElement *videomux;
  GstColorBalance *balance;

  gboolean logo_mode;
  GdkPixbuf *logo_pixbuf;

  /* Clutter */
  ClutterActor *stage;
  ClutterActor *texture;
  ClutterActor *frame;

  ClutterActor *logo_frame;
  ClutterActor *logo;

  /*GStreamer bus */
  GstBus *bus;
  gulong sig_bus_async;
  gulong sig_bus_sync;
};

static GtkWidgetClass *parent_class = NULL;

static GThread *gui_thread;

static int gcc_signals[LAST_SIGNAL] = { 0 };

static void gcc_error_msg (GstCameraCapturer * gcc, GstMessage * msg);
static void gcc_bus_message_cb (GstBus * bus, GstMessage * message,
    gpointer data);
static void gst_camera_capturer_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_camera_capturer_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static int gcc_get_video_stream_info (GstCameraCapturer * gcc);

G_DEFINE_TYPE (GstCameraCapturer, gst_camera_capturer, GTK_CLUTTER_TYPE_EMBED);

static void
gst_camera_capturer_init (GstCameraCapturer * object)
{
  GstCameraCapturerPrivate *priv;
  object->priv = priv =
      G_TYPE_INSTANCE_GET_PRIVATE (object, GST_TYPE_CAMERA_CAPTURER,
      GstCameraCapturerPrivate);

  priv->output_height = 576;
  priv->output_width = 720;
  priv->output_fps_n = 25;
  priv->output_fps_d = 1;
  priv->audio_bitrate = 128;
  priv->video_bitrate = 5000;
  priv->last_buffer = NULL;
  priv->source_type = GST_CAMERA_CAPTURE_SOURCE_TYPE_NONE;
}

void
gst_camera_capturer_finalize (GObject * object)
{
  GstCameraCapturer *gcc = (GstCameraCapturer *) object;

  GST_DEBUG_OBJECT (gcc, "Finalizing.");
  if (gcc->priv->bus) {
    /* make bus drop all messages to make sure none of our callbacks is ever
     * called again (main loop might be run again to display error dialog) */
    gst_bus_set_flushing (gcc->priv->bus, TRUE);

    if (gcc->priv->sig_bus_async)
      g_signal_handler_disconnect (gcc->priv->bus, gcc->priv->sig_bus_async);

    if (gcc->priv->sig_bus_sync)
      g_signal_handler_disconnect (gcc->priv->bus, gcc->priv->sig_bus_sync);

    gst_object_unref (gcc->priv->bus);
    gcc->priv->bus = NULL;
  }

  if (gcc->priv->output_file) {
    g_free (gcc->priv->output_file);
    gcc->priv->output_file = NULL;
  }

  if (gcc->priv->device_id) {
    g_free (gcc->priv->device_id);
    gcc->priv->device_id = NULL;
  }

  if (gcc->priv->logo_pixbuf) {
    g_object_unref (gcc->priv->logo_pixbuf);
    gcc->priv->logo_pixbuf = NULL;
  }

  if (gcc->priv->last_buffer != NULL)
    gst_buffer_unref (gcc->priv->last_buffer);

  if (gcc->priv->main_pipeline != NULL
      && GST_IS_ELEMENT (gcc->priv->main_pipeline)) {
    gst_element_set_state (gcc->priv->main_pipeline, GST_STATE_NULL);
    gst_object_unref (gcc->priv->main_pipeline);
    gcc->priv->main_pipeline = NULL;
  }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_camera_capturer_apply_resolution (GstCameraCapturer * gcc)
{
  GST_INFO_OBJECT (gcc, "Changed video resolution to %dx%d@%d/%dfps",
      gcc->priv->output_width, gcc->priv->output_height,
      gcc->priv->output_fps_n, gcc->priv->output_fps_d);

  g_signal_emit_by_name (G_OBJECT (gcc->priv->camerabin),
      "set-video-resolution-fps", gcc->priv->output_width,
      gcc->priv->output_height, gcc->priv->output_fps_n,
      gcc->priv->output_fps_d);
}

static void
gst_camera_capturer_set_video_bit_rate (GstCameraCapturer * gcc, gint bitrate)
{
  gcc->priv->video_bitrate = bitrate;
  if (gcc->priv->video_encoder_type == VIDEO_ENCODER_MPEG4 ||
      gcc->priv->video_encoder_type == VIDEO_ENCODER_XVID)
    g_object_set (gcc->priv->videoenc, "bitrate", bitrate * 1000, NULL);
  else
    g_object_set (gcc->priv->videoenc, "bitrate", gcc->priv->video_bitrate,
        NULL);
  GST_INFO_OBJECT (gcc, "Changed video bitrate to :\n%d",
      gcc->priv->video_bitrate);
}

static void
gst_camera_capturer_set_audio_bit_rate (GstCameraCapturer * gcc, gint bitrate)
{

  gcc->priv->audio_bitrate = bitrate;
  if (gcc->priv->audio_encoder_type == AUDIO_ENCODER_MP3)
    g_object_set (gcc->priv->audioenc, "bitrate", bitrate, NULL);
  else
    g_object_set (gcc->priv->audioenc, "bitrate", 1000 * bitrate, NULL);
  GST_INFO_OBJECT (gcc, "Changed audio bitrate to :\n%d",
      gcc->priv->audio_bitrate);

}

static void
gst_camera_capturer_set_audio_enabled (GstCameraCapturer * gcc,
    gboolean enabled)
{
  gint flags;
  gcc->priv->audio_enabled = enabled;

  g_object_get (gcc->priv->main_pipeline, "flags", &flags, NULL);
  if (!enabled) {
    flags &= ~GST_CAMERABIN_FLAG_DISABLE_AUDIO;
    GST_INFO_OBJECT (gcc, "Audio disabled");
  } else {
    flags |= GST_CAMERABIN_FLAG_DISABLE_AUDIO;
    GST_INFO_OBJECT (gcc, "Audio enabled");
  }
}

static void
gst_camera_capturer_set_output_file (GstCameraCapturer * gcc,
    const gchar * file)
{
  gcc->priv->output_file = g_strdup (file);
  g_object_set (gcc->priv->camerabin, "filename", file, NULL);
  GST_INFO_OBJECT (gcc, "Changed output filename to :\n%s", file);

}

static void
gst_camera_capturer_set_device_id (GstCameraCapturer * gcc,
    const gchar * device_id)
{
  gcc->priv->device_id = g_strdup (device_id);
  GST_INFO_OBJECT (gcc, "Changed device id/name to :\n%s", device_id);
}

static void
gst_camera_capturer_realize (GtkWidget * widget)
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

static void
gst_camera_capturer_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstCameraCapturer *gcc;

  gcc = GST_CAMERA_CAPTURER (object);

  switch (property_id) {
    case PROP_OUTPUT_HEIGHT:
      gcc->priv->output_height = g_value_get_uint (value);
      gst_camera_capturer_apply_resolution (gcc);
      break;
    case PROP_OUTPUT_WIDTH:
      gcc->priv->output_width = g_value_get_uint (value);
      gst_camera_capturer_apply_resolution (gcc);
      break;
    case PROP_VIDEO_BITRATE:
      gst_camera_capturer_set_video_bit_rate (gcc, g_value_get_uint (value));
      break;
    case PROP_AUDIO_BITRATE:
      gst_camera_capturer_set_audio_bit_rate (gcc, g_value_get_uint (value));
      break;
    case PROP_AUDIO_ENABLED:
      gst_camera_capturer_set_audio_enabled (gcc, g_value_get_boolean (value));
      break;
    case PROP_OUTPUT_FILE:
      gst_camera_capturer_set_output_file (gcc, g_value_get_string (value));
      break;
    case PROP_DEVICE_ID:
      gst_camera_capturer_set_device_id (gcc, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gst_camera_capturer_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstCameraCapturer *gcc;

  gcc = GST_CAMERA_CAPTURER (object);

  switch (property_id) {
    case PROP_OUTPUT_HEIGHT:
      g_value_set_uint (value, gcc->priv->output_height);
      break;
    case PROP_OUTPUT_WIDTH:
      g_value_set_uint (value, gcc->priv->output_width);
      break;
    case PROP_AUDIO_BITRATE:
      g_value_set_uint (value, gcc->priv->audio_bitrate);
      break;
    case PROP_VIDEO_BITRATE:
      g_value_set_uint (value, gcc->priv->video_bitrate);
      break;
    case PROP_AUDIO_ENABLED:
      g_value_set_boolean (value, gcc->priv->audio_enabled);
      break;
    case PROP_OUTPUT_FILE:
      g_value_set_string (value, gcc->priv->output_file);
      break;
    case PROP_DEVICE_ID:
      g_value_set_string (value, gcc->priv->device_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gst_camera_capturer_class_init (GstCameraCapturerClass * klass)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;
  parent_class = g_type_class_peek_parent (klass);

  g_type_class_add_private (object_class, sizeof (GstCameraCapturerPrivate));

  widget_class->realize = gst_camera_capturer_realize;

  /* GObject */
  object_class->set_property = gst_camera_capturer_set_property;
  object_class->get_property = gst_camera_capturer_get_property;
  object_class->finalize = gst_camera_capturer_finalize;

  /* Properties */
  g_object_class_install_property (object_class, PROP_OUTPUT_HEIGHT,
      g_param_spec_uint ("output_height", NULL,
          NULL, 0, 5600, 576, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_OUTPUT_WIDTH,
      g_param_spec_uint ("output_width", NULL,
          NULL, 0, 5600, 720, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_VIDEO_BITRATE,
      g_param_spec_uint ("video_bitrate", NULL,
          NULL, 100, G_MAXUINT, 1000, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_AUDIO_BITRATE,
      g_param_spec_uint ("audio_bitrate", NULL,
          NULL, 12, G_MAXUINT, 128, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_AUDIO_ENABLED,
      g_param_spec_boolean ("audio_enabled", NULL,
          NULL, FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_OUTPUT_FILE,
      g_param_spec_string ("output_file", NULL,
          NULL, FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (object_class, PROP_DEVICE_ID,
      g_param_spec_string ("device_id", NULL, NULL, FALSE, G_PARAM_READWRITE));

  /* Signals */
  gcc_signals[SIGNAL_ERROR] =
      g_signal_new ("error",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstCameraCapturerClass, error),
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  gcc_signals[SIGNAL_EOS] =
      g_signal_new ("eos",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstCameraCapturerClass, eos),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gcc_signals[SIGNAL_DEVICE_CHANGE] =
      g_signal_new ("device-change",
      G_TYPE_FROM_CLASS (object_class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstCameraCapturerClass, device_change),
      NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

void
gst_camera_capturer_init_backend (int *argc, char ***argv)
{
  gst_init (argc, argv);
}

GQuark
gst_camera_capturer_error_quark (void)
{
  static GQuark q;              /* 0 */

  if (G_UNLIKELY (q == 0)) {
    q = g_quark_from_static_string ("gcc-error-quark");
  }
  return q;
}

gboolean
gst_camera_capture_videosrc_buffer_probe (GstPad * pad, GstBuffer * buf,
    gpointer data)
{
  GstCameraCapturer *gcc = GST_CAMERA_CAPTURER (data);

  if (gcc->priv->last_buffer) {
    gst_buffer_unref (gcc->priv->last_buffer);
    gcc->priv->last_buffer = NULL;
  }

  gst_buffer_ref (buf);
  gcc->priv->last_buffer = buf;

  return TRUE;
}

static void
cb_new_pad (GstElement * element, GstPad * pad, gpointer data)
{
  GstCaps *caps;
  const gchar *mime;
  GstElement *sink;
  GstBin *bin = GST_BIN (data);

  caps = gst_pad_get_caps (pad);
  mime = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  if (g_strrstr (mime, "video")) {
    sink = gst_bin_get_by_name (bin, "source_video_sink");
    gst_pad_link (pad, gst_element_get_pad (sink, "sink"));
  }
  if (g_strrstr (mime, "audio")) {
    /* Not implemented yet */
  }
}

/* On linux GStreamer packages provided by distributions might still have the
 * dv1394src clock bug and the dvdemuxer buffers duration bug. That's why we
 * can't use decodebin2 and we need to force the use of ffdemux_dv */
static GstElement *
gst_camera_capture_create_dv1394_source_bin (GstCameraCapturer * gcc)
{
  GstElement *bin;
  GstElement *demuxer;
  GstElement *queue1;
  GstElement *decoder;
  GstElement *queue2;
  GstElement *deinterlacer;
  GstElement *colorspace;
  GstElement *videorate;
  GstElement *videoscale;
  GstPad *src_pad;

  bin = gst_bin_new ("videosource");
  gcc->priv->device_source =
      gst_element_factory_make (DVVIDEOSRC, "source_device");
  demuxer = gst_element_factory_make ("ffdemux_dv", NULL);
  queue1 = gst_element_factory_make ("queue", "source_video_sink");
  decoder = gst_element_factory_make ("ffdec_dvvideo", NULL);
  queue2 = gst_element_factory_make ("queue", NULL);
  deinterlacer = gst_element_factory_make ("ffdeinterlace", NULL);
  videorate = gst_element_factory_make ("videorate", NULL);
  colorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
  videoscale = gst_element_factory_make ("videoscale", NULL);

  /* this property needs to be set before linking the element, where the device
   * id configured in get_caps() */
  g_object_set (G_OBJECT (gcc->priv->device_source), "guid",
      g_ascii_strtoull (gcc->priv->device_id, NULL, 0), NULL);

  gst_bin_add_many (GST_BIN (bin), gcc->priv->device_source, demuxer, queue1,
      decoder, queue2, deinterlacer, colorspace, videorate, videoscale, NULL);
  gst_element_link (gcc->priv->device_source, demuxer);
  gst_element_link_many (queue1, decoder, queue2, deinterlacer, videorate,
      colorspace, videoscale, NULL);

  g_signal_connect (demuxer, "pad-added", G_CALLBACK (cb_new_pad), bin);

  /* add ghostpad */
  src_pad = gst_element_get_static_pad (videoscale, "src");
  gst_element_add_pad (bin, gst_ghost_pad_new ("src", src_pad));
  gst_object_unref (GST_OBJECT (src_pad));

  return bin;
}

static GstElement *
gst_camera_capture_create_dshow_source_bin (GstCameraCapturer * gcc)
{
  GstElement *bin;
  GstElement *decoder;
  GstElement *deinterlacer;
  GstElement *colorspace;
  GstElement *videorate;
  GstElement *videoscale;
  GstPad *src_pad;
  GstCaps *source_caps;

  bin = gst_bin_new ("videosource");
  gcc->priv->device_source =
      gst_element_factory_make (DVVIDEOSRC, "source_device");
  decoder = gst_element_factory_make ("decodebin2", NULL);
  colorspace = gst_element_factory_make ("ffmpegcolorspace",
      "source_video_sink");
  deinterlacer = gst_element_factory_make ("ffdeinterlace", NULL);
  videorate = gst_element_factory_make ("videorate", NULL);
  videoscale = gst_element_factory_make ("videoscale", NULL);

  /* this property needs to be set before linking the element, where the device
   * id configured in get_caps() */
  g_object_set (G_OBJECT (gcc->priv->device_source), "device-name",
      gcc->priv->device_id, NULL);

  gst_bin_add_many (GST_BIN (bin), gcc->priv->device_source, decoder,
      colorspace, deinterlacer, videorate, videoscale, NULL);
  source_caps =
      gst_caps_from_string ("video/x-dv, systemstream=true;"
      "video/x-raw-rgb; video/x-raw-yuv");
  gst_element_link_filtered (gcc->priv->device_source, decoder, source_caps);
  gst_element_link_many (colorspace, deinterlacer, videorate, videoscale, NULL);

  g_signal_connect (decoder, "pad-added", G_CALLBACK (cb_new_pad), bin);

  /* add ghostpad */
  src_pad = gst_element_get_static_pad (videoscale, "src");
  gst_element_add_pad (bin, gst_ghost_pad_new ("src", src_pad));
  gst_object_unref (GST_OBJECT (src_pad));

  return bin;
}

gboolean
gst_camera_capturer_set_source (GstCameraCapturer * gcc,
    GstCameraCaptureSourceType source_type, GError ** err)
{
  GstPad *videosrcpad;

  g_return_val_if_fail (gcc != NULL, FALSE);
  g_return_val_if_fail (GST_IS_CAMERA_CAPTURER (gcc), FALSE);

  if (gcc->priv->source_type == source_type)
    return TRUE;
  gcc->priv->source_type = source_type;

  switch (gcc->priv->source_type) {
    case GST_CAMERA_CAPTURE_SOURCE_TYPE_DV:
    {
      gcc->priv->videosrc = gst_camera_capture_create_dv1394_source_bin (gcc);
      /*gcc->priv->audiosrc = gcc->priv->videosrc; */
      break;
    }
    case GST_CAMERA_CAPTURE_SOURCE_TYPE_DSHOW:
    {
      gcc->priv->videosrc = gst_camera_capture_create_dshow_source_bin (gcc);
      /*gcc->priv->audiosrc = gcc->priv->videosrc; */
      break;
    }
    case GST_CAMERA_CAPTURE_SOURCE_TYPE_RAW:
    default:
    {
      gchar *videosrc = RAWVIDEOSRC;

#ifndef WIN32
      GstElementFactory *fact = gst_element_factory_find (RAWVIDEOSRC);
      if (fact == NULL)
        videosrc = RAWVIDEOSRC_GCONF;
      else
        gst_object_unref (fact);
#endif

      gchar *bin =
          g_strdup_printf ("%s name=device_source ! videorate ! "
          "ffmpegcolorspace ! videoscale", videosrc);
      gcc->priv->videosrc = gst_parse_bin_from_description (bin, TRUE, err);
      gcc->priv->device_source =
          gst_bin_get_by_name (GST_BIN (gcc->priv->videosrc), "device_source");
      gcc->priv->audiosrc = gst_element_factory_make (AUDIOSRC, "audiosource");
      break;
    }
  }
  if (*err) {
    GST_ERROR_OBJECT (gcc, "Error changing source: %s", (*err)->message);
    return FALSE;
  }

  g_object_set (gcc->priv->camerabin, "video-source", gcc->priv->videosrc,
      NULL);

  /* Install pad probe to store the last buffer */
  videosrcpad = gst_element_get_pad (gcc->priv->videosrc, "src");
  gst_pad_add_buffer_probe (videosrcpad,
      G_CALLBACK (gst_camera_capture_videosrc_buffer_probe), gcc);
  return TRUE;
}

GstCameraCapturer *
gst_camera_capturer_new (gchar * filename, GError ** err)
{
  GstCameraCapturer *gcc = NULL;
  GstElement *video_sink = NULL;
  GstElement *balance, *sink, *bin;
  GstPad *pad, *ghostpad;
  ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
  ClutterConstraint *constraint;
  gchar *plugin;
  gint flags = 0;

  gcc = g_object_new (GST_TYPE_CAMERA_CAPTURER, NULL);

  gcc->priv->main_pipeline = gst_pipeline_new ("main_pipeline");

  if (!gcc->priv->main_pipeline) {
    plugin = "pipeline";
    goto missing_plugin;
  }

  /* Setup */
  GST_INFO_OBJECT (gcc, "Initializing camerabin");
  gcc->priv->camerabin = gst_element_factory_make ("camerabin", "camerabin");
  gst_bin_add (GST_BIN (gcc->priv->main_pipeline), gcc->priv->camerabin);
  if (!gcc->priv->camerabin) {
    plugin = "camerabin";
    goto missing_plugin;
  }
  GST_INFO_OBJECT (gcc, "Setting capture mode to \"video\"");
  g_object_set (gcc->priv->camerabin, "mode", 1, NULL);


  GST_INFO_OBJECT (gcc, "Disabling audio");
  flags = GST_CAMERABIN_FLAG_DISABLE_AUDIO;
#ifdef WIN32
  flags |= GST_CAMERABIN_FLAG_VIEWFINDER_COLOR_CONVERSION;
#endif
  g_object_set (gcc->priv->camerabin, "flags", flags, NULL);

  /* assume we're always called from the main Gtk+ GUI thread */
  gui_thread = g_thread_self ();

  /*Connect bus signals */
  GST_INFO_OBJECT (gcc, "Connecting bus signals");
  gcc->priv->bus = gst_element_get_bus (GST_ELEMENT (gcc->priv->main_pipeline));
  gst_bus_add_signal_watch (gcc->priv->bus);
  gcc->priv->sig_bus_async =
      g_signal_connect (gcc->priv->bus, "message",
      G_CALLBACK (gcc_bus_message_cb), gcc);

  gcc->priv->stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (gcc));
  clutter_stage_set_color (CLUTTER_STAGE (gcc->priv->stage), &black);

  /* Bin */
  bin = gst_bin_new ("video_sink_bin");

  /* Video sink, with aspect frame */
  gcc->priv->texture = g_object_new (CLUTTER_TYPE_TEXTURE,
      "disable-slicing", TRUE, NULL);
  sink = clutter_gst_video_sink_new (CLUTTER_TEXTURE (gcc->priv->texture));
  if (sink == NULL)
    g_critical ("Could not create Clutter video sink");

  /* The logo */
  gcc->priv->logo_frame = longomatch_aspect_frame_new ();
  clutter_actor_set_name (gcc->priv->logo_frame, "logo-frame");
  gcc->priv->logo = clutter_texture_new ();
  mx_bin_set_child (MX_BIN (gcc->priv->logo_frame), gcc->priv->logo);
  clutter_container_add_actor (CLUTTER_CONTAINER (gcc->priv->stage),
      gcc->priv->logo_frame);
  mx_bin_set_fill (MX_BIN (gcc->priv->logo_frame), FALSE, FALSE);
  mx_bin_set_alignment (MX_BIN (gcc->priv->logo_frame), MX_ALIGN_MIDDLE,
      MX_ALIGN_MIDDLE);
  clutter_actor_set_size (gcc->priv->logo, LOGO_SIZE_W, LOGO_SIZE_H);
  constraint =
      clutter_bind_constraint_new (gcc->priv->stage, CLUTTER_BIND_SIZE, 0.0);
  clutter_actor_add_constraint_with_name (gcc->priv->logo_frame, "size",
      constraint);
  clutter_actor_hide (CLUTTER_ACTOR (gcc->priv->logo_frame));

  /* The video */
  gcc->priv->frame = longomatch_aspect_frame_new ();
  clutter_actor_set_name (gcc->priv->frame, "frame");
  mx_bin_set_child (MX_BIN (gcc->priv->frame), gcc->priv->texture);

  clutter_container_add_actor (CLUTTER_CONTAINER (gcc->priv->stage),
      gcc->priv->frame);
  constraint =
      clutter_bind_constraint_new (gcc->priv->stage, CLUTTER_BIND_SIZE, 0.0);
  clutter_actor_add_constraint_with_name (gcc->priv->frame, "size", constraint);

  clutter_actor_raise (CLUTTER_ACTOR (gcc->priv->frame),
      CLUTTER_ACTOR (gcc->priv->logo_frame));

  gst_bin_add (GST_BIN (bin), sink);

  /* Add video balance */
  balance = gst_element_factory_make ("videobalance", "video_balance");
  gst_bin_add (GST_BIN (bin), balance);
  gcc->priv->balance = GST_COLOR_BALANCE (balance);
  pad = gst_element_get_static_pad (balance, "sink");
  ghostpad = gst_ghost_pad_new ("sink", pad);
  gst_element_add_pad (bin, ghostpad);

  gst_element_link (balance, sink);

  video_sink = bin;
  gst_element_set_state (video_sink, GST_STATE_READY);
  return gcc;

/* Missing plugin */
missing_plugin:
  {
    g_set_error (err, GCC_ERROR, GST_ERROR_PLUGIN_LOAD,
        ("Failed to create a GStreamer element. "
            "The element \"%s\" is missing. "
            "Please check your GStreamer installation."), plugin);
    g_object_ref_sink (gcc);
    g_object_unref (gcc);
    return NULL;
  }
}

void
gst_camera_capturer_run (GstCameraCapturer * gcc)
{
  GError *err = NULL;

  g_return_if_fail (gcc != NULL);
  g_return_if_fail (GST_IS_CAMERA_CAPTURER (gcc));

  /* the source needs to be created before the 'device-is' is set
   * because dshowsrcwrapper can't change the device-name after
   * it has been linked for the first time */
  if (!gcc->priv->videosrc)
    gst_camera_capturer_set_source (gcc, gcc->priv->source_type, &err);
  gst_element_set_state (gcc->priv->main_pipeline, GST_STATE_PLAYING);
}

void
gst_camera_capturer_close (GstCameraCapturer * gcc)
{
  g_return_if_fail (gcc != NULL);
  g_return_if_fail (GST_IS_CAMERA_CAPTURER (gcc));

  gst_element_set_state (gcc->priv->main_pipeline, GST_STATE_NULL);
}

void
gst_camera_capturer_start (GstCameraCapturer * gcc)
{
  g_return_if_fail (gcc != NULL);
  g_return_if_fail (GST_IS_CAMERA_CAPTURER (gcc));

  g_signal_emit_by_name (G_OBJECT (gcc->priv->camerabin), "capture-start", 0,
      0);
}

void
gst_camera_capturer_toggle_pause (GstCameraCapturer * gcc)
{
  g_return_if_fail (gcc != NULL);
  g_return_if_fail (GST_IS_CAMERA_CAPTURER (gcc));

  g_signal_emit_by_name (G_OBJECT (gcc->priv->camerabin), "capture-pause", 0,
      0);
}

void
gst_camera_capturer_stop (GstCameraCapturer * gcc)
{
  g_return_if_fail (gcc != NULL);
  g_return_if_fail (GST_IS_CAMERA_CAPTURER (gcc));

#ifdef WIN32
  //On windows we can't handle device disconnections until dshowvideosrc
  //supports it. When a device is disconnected, the source is locked
  //in ::create(), blocking the streaming thread. We need to change its
  //state to null, this way camerabin doesn't block in ::do_stop().
  gst_element_set_state (gcc->priv->device_source, GST_STATE_NULL);
#endif
  g_signal_emit_by_name (G_OBJECT (gcc->priv->camerabin), "capture-stop", 0, 0);
}

gboolean
gst_camera_capturer_set_video_encoder (GstCameraCapturer * gcc,
    VideoEncoderType type, GError ** err)
{
  gchar *name = NULL;

  g_return_val_if_fail (gcc != NULL, FALSE);
  g_return_val_if_fail (GST_IS_CAMERA_CAPTURER (gcc), FALSE);

  switch (type) {
    case VIDEO_ENCODER_MPEG4:
      gcc->priv->videoenc =
          gst_element_factory_make ("ffenc_mpeg4", "video-encoder");
      g_object_set (gcc->priv->videoenc, "pass", 512,
          "max-key-interval", -1, NULL);
      name = "FFmpeg mpeg4 video encoder";
      break;

    case VIDEO_ENCODER_XVID:
      gcc->priv->videoenc =
          gst_element_factory_make ("xvidenc", "video-encoder");
      g_object_set (gcc->priv->videoenc, "pass", 1,
          "profile", 146, "max-key-interval", -1, NULL);
      name = "Xvid video encoder";
      break;

    case VIDEO_ENCODER_H264:
      gcc->priv->videoenc =
          gst_element_factory_make ("x264enc", "video-encoder");
      g_object_set (gcc->priv->videoenc, "key-int-max", 25, "pass", 17,
          "speed-preset", 3, NULL);
      name = "X264 video encoder";
      break;

    case VIDEO_ENCODER_THEORA:
      gcc->priv->videoenc =
          gst_element_factory_make ("theoraenc", "video-encoder");
      g_object_set (gcc->priv->videoenc, "keyframe-auto", FALSE,
          "keyframe-force", 25, NULL);
      name = "Theora video encoder";
      break;

    case VIDEO_ENCODER_VP8:
    default:
      gcc->priv->videoenc =
          gst_element_factory_make ("vp8enc", "video-encoder");
      g_object_set (gcc->priv->videoenc, "speed", 2, "threads", 8,
          "max-keyframe-distance", 25, NULL);
      name = "VP8 video encoder";
      break;

  }
  if (!gcc->priv->videoenc) {
    g_set_error (err,
        GCC_ERROR,
        GST_ERROR_PLUGIN_LOAD,
        "Failed to create the %s element. "
        "Please check your GStreamer installation.", name);
  } else {
    g_object_set (gcc->priv->camerabin, "video-encoder", gcc->priv->videoenc,
        NULL);
    gcc->priv->video_encoder_type = type;
  }
  return TRUE;
}

gboolean
gst_camera_capturer_set_audio_encoder (GstCameraCapturer * gcc,
    AudioEncoderType type, GError ** err)
{
  gchar *name = NULL;

  g_return_val_if_fail (gcc != NULL, FALSE);
  g_return_val_if_fail (GST_IS_CAMERA_CAPTURER (gcc), FALSE);

  switch (type) {
    case AUDIO_ENCODER_MP3:
      gcc->priv->audioenc =
          gst_element_factory_make ("lamemp3enc", "audio-encoder");
      g_object_set (gcc->priv->audioenc, "target", 0, NULL);
      name = "Mp3 audio encoder";
      break;

    case AUDIO_ENCODER_AAC:
      gcc->priv->audioenc = gst_element_factory_make ("faac", "audio-encoder");
      name = "AAC audio encoder";
      break;

    case AUDIO_ENCODER_VORBIS:
    default:
      gcc->priv->audioenc =
          gst_element_factory_make ("vorbisenc", "audio-encoder");
      name = "Vorbis audio encoder";
      break;
  }

  if (!gcc->priv->audioenc) {
    g_set_error (err,
        GCC_ERROR,
        GST_ERROR_PLUGIN_LOAD,
        "Failed to create the %s element. "
        "Please check your GStreamer installation.", name);
  } else {
    g_object_set (gcc->priv->camerabin, "audio-encoder", gcc->priv->audioenc,
        NULL);
    gcc->priv->audio_encoder_type = type;
  }

  return TRUE;
}

gboolean
gst_camera_capturer_set_video_muxer (GstCameraCapturer * gcc,
    VideoMuxerType type, GError ** err)
{
  gchar *name = NULL;

  g_return_val_if_fail (gcc != NULL, FALSE);
  g_return_val_if_fail (GST_IS_CAMERA_CAPTURER (gcc), FALSE);

  switch (type) {
    case VIDEO_MUXER_OGG:
      name = "OGG muxer";
      gcc->priv->videomux = gst_element_factory_make ("oggmux", "video-muxer");
      break;
    case VIDEO_MUXER_AVI:
      name = "AVI muxer";
      gcc->priv->videomux = gst_element_factory_make ("avimux", "video-muxer");
      break;
    case VIDEO_MUXER_MATROSKA:
      name = "Matroska muxer";
      gcc->priv->videomux =
          gst_element_factory_make ("matroskamux", "video-muxer");
      break;
    case VIDEO_MUXER_MP4:
      name = "MP4 muxer";
      gcc->priv->videomux = gst_element_factory_make ("qtmux", "video-muxer");
      break;
    case VIDEO_MUXER_WEBM:
    default:
      name = "WebM muxer";
      gcc->priv->videomux = gst_element_factory_make ("webmmux", "video-muxer");
      break;
  }

  if (!gcc->priv->videomux) {
    g_set_error (err,
        GCC_ERROR,
        GST_ERROR_PLUGIN_LOAD,
        "Failed to create the %s element. "
        "Please check your GStreamer installation.", name);
  } else {
    g_object_set (gcc->priv->camerabin, "video-muxer", gcc->priv->videomux,
        NULL);
  }

  return TRUE;
}

static void
gcc_bus_message_cb (GstBus * bus, GstMessage * message, gpointer data)
{
  GstCameraCapturer *gcc = (GstCameraCapturer *) data;
  GstMessageType msg_type;

  g_return_if_fail (gcc != NULL);
  g_return_if_fail (GST_IS_CAMERA_CAPTURER (gcc));

  msg_type = GST_MESSAGE_TYPE (message);

  switch (msg_type) {
    case GST_MESSAGE_ERROR:
    {
      if (gcc->priv->main_pipeline) {
        gst_camera_capturer_stop (gcc);
        gst_camera_capturer_close (gcc);
        gst_element_set_state (gcc->priv->main_pipeline, GST_STATE_NULL);
      }
      gcc_error_msg (gcc, message);
      break;
    }

    case GST_MESSAGE_WARNING:
    {
      GST_WARNING ("Warning message: %" GST_PTR_FORMAT, message);
      break;
    }

    case GST_MESSAGE_EOS:
    {
      GST_INFO_OBJECT (gcc, "EOS message");
      g_signal_emit (gcc, gcc_signals[SIGNAL_EOS], 0);
      break;
    }

    case GST_MESSAGE_STATE_CHANGED:
    {
      GstState old_state, new_state;

      gst_message_parse_state_changed (message, &old_state, &new_state, NULL);

      if (old_state == new_state)
        break;

      /* we only care about playbin (pipeline) state changes */
      if (GST_MESSAGE_SRC (message) != GST_OBJECT (gcc->priv->main_pipeline))
        break;

      if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_PLAYING) {
        gcc_get_video_stream_info (gcc);
      }
    }

    case GST_MESSAGE_ELEMENT:
    {
      const GstStructure *s;
      gint device_change = 0;

      /* We only care about messages sent by the device source */
      if (GST_MESSAGE_SRC (message) != GST_OBJECT (gcc->priv->device_source))
        break;

      s = gst_message_get_structure (message);
      /* check if it's bus reset message and it contains the
       * 'current-device-change' field */
      if (g_strcmp0 (gst_structure_get_name (s), "ieee1394-bus-reset"))
        break;
      if (!gst_structure_has_field (s, "current-device-change"))
        break;


      /* emit a signal if the device was connected or disconnected */
      gst_structure_get_int (s, "current-device-change", &device_change);

      if (device_change != 0)
        g_signal_emit (gcc, gcc_signals[SIGNAL_DEVICE_CHANGE], 0,
            device_change);
      break;
    }

    default:
      GST_LOG ("Unhandled message: %" GST_PTR_FORMAT, message);
      break;
  }
}

static void
gcc_error_msg (GstCameraCapturer * gcc, GstMessage * msg)
{
  GError *err = NULL;
  gchar *dbg = NULL;

  gst_message_parse_error (msg, &err, &dbg);
  if (err) {
    GST_ERROR ("message = %s", GST_STR_NULL (err->message));
    GST_ERROR ("domain  = %d (%s)", err->domain,
        GST_STR_NULL (g_quark_to_string (err->domain)));
    GST_ERROR ("code    = %d", err->code);
    GST_ERROR ("debug   = %s", GST_STR_NULL (dbg));
    GST_ERROR ("source  = %" GST_PTR_FORMAT, msg->src);


    g_message ("Error: %s\n%s\n", GST_STR_NULL (err->message),
        GST_STR_NULL (dbg));
    g_signal_emit (gcc, gcc_signals[SIGNAL_ERROR], 0, err->message);
    g_error_free (err);
  }
  g_free (dbg);
}

static int
gcc_get_video_stream_info (GstCameraCapturer * gcc)
{
  GstPad *sourcepad;
  GstCaps *caps;
  GstStructure *s;

  sourcepad = gst_element_get_pad (gcc->priv->videosrc, "src");
  caps = gst_pad_get_negotiated_caps (sourcepad);

  if (!(caps)) {
    GST_WARNING_OBJECT (gcc, "Could not get stream info");
    return -1;
  }

  /* Get the source caps */
  s = gst_caps_get_structure (caps, 0);
  if (s) {
    const GValue *movie_par;
    /* We need at least width/height and framerate */
    if (!
        (gst_structure_get_fraction
            (s, "framerate", &gcc->priv->video_fps_n, &gcc->priv->video_fps_d)
            && gst_structure_get_int (s, "width", &gcc->priv->video_width)
            && gst_structure_get_int (s, "height", &gcc->priv->video_height)))
      return -1;

    /* Get the movie PAR if available */
    movie_par = gst_structure_get_value (s, "pixel-aspect-ratio");
    if (movie_par) {
      gcc->priv->movie_par_n = gst_value_get_fraction_numerator (movie_par);
      gcc->priv->movie_par_d = gst_value_get_fraction_denominator (movie_par);
    } else {
      /* Square pixels */
      gcc->priv->movie_par_n = 1;
      gcc->priv->movie_par_d = 1;
    }
  }
  return 1;
}

GList *
gst_camera_capturer_enum_devices (gchar * device_name)
{
  GstElement *device;
  GstPropertyProbe *probe;
  GValueArray *va;
  gchar *prop_name;
  GList *list = NULL;
  guint i = 0;

  device = gst_element_factory_make (device_name, "source");
  if (!device || !GST_IS_PROPERTY_PROBE (device))
    goto finish;
  gst_element_set_state (device, GST_STATE_READY);
  gst_element_get_state (device, NULL, NULL, 5 * GST_SECOND);
  probe = GST_PROPERTY_PROBE (device);

  if (!g_strcmp0 (device_name, "dv1394src"))
    prop_name = "guid";
  else if (!g_strcmp0 (device_name, "v4l2src"))
    prop_name = "device";
  else
    prop_name = "device-name";

  va = gst_property_probe_get_values_name (probe, prop_name);
  if (!va)
    goto finish;

  for (i = 0; i < va->n_values; ++i) {
    GValue *v = g_value_array_get_nth (va, i);
    GValue valstr = { 0, };

    g_value_init (&valstr, G_TYPE_STRING);
    if (!g_value_transform (v, &valstr))
      continue;
    list = g_list_append (list, g_value_dup_string (&valstr));
    g_value_unset (&valstr);
  }
  g_value_array_free (va);

finish:
  {
    gst_element_set_state (device, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (device));
    return list;
  }
}

GList *
gst_camera_capturer_enum_video_devices (void)
{
  return gst_camera_capturer_enum_devices (DVVIDEOSRC);
}

GList *
gst_camera_capturer_enum_audio_devices (void)
{
  return gst_camera_capturer_enum_devices (AUDIOSRC);
}

gboolean
gst_camera_capturer_can_get_frames (GstCameraCapturer * gcc, GError ** error)
{
  g_return_val_if_fail (gcc != NULL, FALSE);
  g_return_val_if_fail (GST_IS_CAMERA_CAPTURER (gcc), FALSE);
  g_return_val_if_fail (GST_IS_ELEMENT (gcc->priv->camerabin), FALSE);

  /* check for video */
  if (!gcc->priv->media_has_video) {
    g_set_error_literal (error, GCC_ERROR, GST_ERROR_GENERIC,
        "Media contains no supported video streams.");
    return FALSE;
  }
  return TRUE;
}

void
gst_camera_capturer_unref_pixbuf (GdkPixbuf * pixbuf)
{
  gdk_pixbuf_unref (pixbuf);
}
