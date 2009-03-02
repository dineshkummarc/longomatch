/*
 * Copyright 2007 Ole André Vadla Ravnås <ole.andre.ravnas@tandberg.com>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-audiochunk
 *
 * <refsect2>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch-0.10 -v audiotestsrc samplesperbuffer=640 ! audiochunk samples-per-buffer=160 ! fakesink
 * </programlisting>
 * </para>
 * </refsect2>
 */

/*
 * This element is only a temporary hack, this should be included in
 * audioparse as soon as slomo's ongoing implementation has been released.
 *
 * TODO:
 *  - Handle new-segment.
 *  - Forward events.
 */

#include "gstaudiochunk.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

enum
{
  PROP_0,
  PROP_SAMPLES_PER_BUFFER,
};

GST_DEBUG_CATEGORY_STATIC (gst_audio_chunk_debug);
#define GST_CAT_DEFAULT gst_audio_chunk_debug

#define HARDCODED_CAPS \
    GST_STATIC_CAPS ("audio/x-raw-int, " \
        "endianness = (int) BYTE_ORDER, " \
        "signed = (boolean) true, " \
        "width = (int) 16, " \
        "depth = (int) 16, " \
        "rate = (int) 8000, " \
        "channels = (int) 1")

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    HARDCODED_CAPS);

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    HARDCODED_CAPS);

static void gst_audio_chunk_dispose (GObject * object);
static void gst_audio_chunk_finalize (GObject * object);
static void gst_audio_chunk_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_audio_chunk_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);

static void gst_audio_chunk_reset (GstAudioChunk * self);

static GstFlowReturn gst_audio_chunk_chain (GstPad * pad, GstBuffer * buffer);

GST_BOILERPLATE (GstAudioChunk, gst_audio_chunk, GstElement, GST_TYPE_ELEMENT);

static void
gst_audio_chunk_base_init (gpointer gclass)
{
  GstElementClass * element_class = GST_ELEMENT_CLASS (gclass);
  static GstElementDetails element_details = {
    "AudioChunk",
    "Raw/Audio",
    "Chunks an audio stream",
    "Ole André Vadla Ravnås <ole.andre.ravnas@tandberg.com>"
  };

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));

  gst_element_class_set_details (element_class, &element_details);
}

static void
gst_audio_chunk_class_init (GstAudioChunkClass * klass)
{
  GObjectClass * gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = gst_audio_chunk_dispose;
  gobject_class->finalize = gst_audio_chunk_finalize;
  gobject_class->get_property = gst_audio_chunk_get_property;
  gobject_class->set_property = gst_audio_chunk_set_property;

  g_object_class_install_property (gobject_class,
      PROP_SAMPLES_PER_BUFFER, g_param_spec_int (
          "samples-per-buffer", "Samples per buffer",
          "Number of samples per buffer",
          -1, G_MAXINT32, -1, G_PARAM_READWRITE));

  GST_DEBUG_CATEGORY_INIT (gst_audio_chunk_debug, "audiochunk",
    0, "audio chunk");
}

static void
gst_audio_chunk_init (GstAudioChunk * self,
                      GstAudioChunkClass * gclass)
{
  self->sink_pad = gst_pad_new_from_static_template (&sink_template, "sink");
  gst_pad_set_chain_function (self->sink_pad,
      GST_DEBUG_FUNCPTR (gst_audio_chunk_chain));
  gst_pad_use_fixed_caps (self->sink_pad);
  gst_element_add_pad (GST_ELEMENT (self), self->sink_pad);

  self->src_pad = gst_pad_new_from_static_template (&src_template, "src");
  gst_pad_use_fixed_caps (self->src_pad);
  gst_element_add_pad (GST_ELEMENT (self), self->src_pad);

  self->samples_per_buffer = -1;

  self->adapter = gst_adapter_new ();

  gst_audio_chunk_reset (self);
}

static void
gst_audio_chunk_dispose (GObject * object)
{
  GstAudioChunk * self = GST_AUDIO_CHUNK (object);

  g_object_unref (self->adapter);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_audio_chunk_finalize (GObject * object)
{
  GstAudioChunk * self = GST_AUDIO_CHUNK (object);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_audio_chunk_get_property (GObject * object, guint prop_id,
                              GValue * value, GParamSpec * pspec)
{
  GstAudioChunk * self = GST_AUDIO_CHUNK (object);

  switch (prop_id) {
    case PROP_SAMPLES_PER_BUFFER:
      g_value_set_int (value, self->samples_per_buffer);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_audio_chunk_set_property (GObject * object, guint prop_id,
                              const GValue * value, GParamSpec * pspec)
{
  GstAudioChunk * self = GST_AUDIO_CHUNK (object);

  switch (prop_id) {
    case PROP_SAMPLES_PER_BUFFER:
      self->samples_per_buffer = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_audio_chunk_reset (GstAudioChunk * self)
{
  gst_adapter_clear (self->adapter);
  self->offset = 0;
  self->time = GST_CLOCK_TIME_NONE;
}

static GstFlowReturn
gst_audio_chunk_chain (GstPad * pad, GstBuffer * buffer)
{
  GstAudioChunk * self = GST_AUDIO_CHUNK (GST_PAD_PARENT (pad));
  GstFlowReturn ret = GST_FLOW_OK;
  GstBuffer * out_buf = NULL;
  guint buf_samples, avail_samples;

  if (GST_PAD_CAPS (self->src_pad) == NULL) {
    if (!gst_pad_set_caps (self->src_pad, GST_PAD_CAPS (self->sink_pad))) {
      GST_ELEMENT_ERROR (self, CORE, NEGOTIATION, (NULL),
          ("failed to set caps on source pad"));
      return GST_FLOW_ERROR;
    }
  }

  buf_samples = GST_BUFFER_SIZE (buffer) / sizeof (gint16);
  avail_samples = gst_adapter_available (self->adapter) / sizeof (gint16);

  if (self->samples_per_buffer < 0 ||
    (avail_samples == 0 && buf_samples == self->samples_per_buffer)) {
    out_buf = buffer;
  } else {
    if (!GST_CLOCK_TIME_IS_VALID (self->time)) {
      if (GST_BUFFER_TIMESTAMP_IS_VALID (buffer))
        self->time = GST_BUFFER_TIMESTAMP (buffer);
      else
        self->time = 0;
    }

    gst_adapter_push (self->adapter, buffer);
    avail_samples += buf_samples;

    if (avail_samples >= self->samples_per_buffer) {
      gchar * chunk;
      guint chunk_size = self->samples_per_buffer * sizeof (gint16);

      chunk = gst_adapter_take (self->adapter, chunk_size);

      out_buf = gst_buffer_new ();
      GST_BUFFER_MALLOCDATA (out_buf) = chunk;
      GST_BUFFER_DATA (out_buf) = chunk;
      GST_BUFFER_SIZE (out_buf) = chunk_size;
      GST_BUFFER_CAPS (out_buf) = gst_pad_get_negotiated_caps (self->src_pad);
      GST_BUFFER_OFFSET (out_buf) = self->offset;
      GST_BUFFER_OFFSET_END (out_buf) = self->offset + chunk_size;
      GST_BUFFER_TIMESTAMP (out_buf) = self->time;
      GST_BUFFER_DURATION (out_buf) = gst_util_uint64_scale_int (
          GST_SECOND, self->samples_per_buffer, 8000);

      self->offset += chunk_size;
      self->time += GST_BUFFER_DURATION (out_buf);
    }
  }

  if (out_buf != NULL) {
    ret = gst_pad_push (self->src_pad, out_buf);
    if (ret != GST_FLOW_OK) {
      GST_ELEMENT_ERROR (self, CORE, PAD, (NULL),
        ("Failed to push buffer: %s", gst_flow_get_name (ret)));
      gst_buffer_unref (out_buf);
    }
  }

  return ret;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "audiochunk",
      GST_RANK_NONE, GST_TYPE_AUDIO_CHUNK);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "audiochunk",
    "Element for chunking audio streams",
    plugin_init, VERSION, "LGPL", "GStreamer", "http://gstreamer.net/")
