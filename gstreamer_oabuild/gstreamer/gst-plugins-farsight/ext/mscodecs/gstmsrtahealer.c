/*
 * Farsight Voice+Video library
 *
 *   @author: Ole André Vadla Ravnås <oleavr@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstmsrtahealer.h"

#include "msrtaudiohealer.h"
#include "msbufferstream.h"
#include "msbufferrtpextheader.h"
#include "msbuffercustom.h"
#include "mscodec.h"

#ifndef G_OS_WIN32
#include <winerror.h>
#include <ldt_keeper.h>
#endif

#include <gst/rtp/gstrtpbuffer.h>

GST_DEBUG_CATEGORY_STATIC (gst_ms_rta_healer_debug);
#define GST_CAT_DEFAULT (gst_ms_rta_healer_debug)

static GstElementDetails gst_ms_rta_healer_details = {
  "Magical depayloader, decoder and healer for RTAudio",
  "Codec/Depayloader/Network",
  "Extracts, decodes and heals RTAudio streams from RTP packets",
  "Ole André Vadla Ravnås <oleavr@gmail.com>"
};

struct _GstMSRTAHealerPrivate
{
  GstPad * sinkpad;
  GstPad * srcpad;

  gint clock_rate;

  gboolean flushing;

  guint offset;

  MSAudioHealer * instance;
  GMutex * instance_lock;

  MSBufferStream * stream;
  MSBufferRtpExtHeader * inbuf_metadata;
  MSBufferCustom * inbuf_audio;
  MSBufferCustom * outbuf_audio;
};

#define GST_MS_RTA_HEALER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GST_TYPE_MS_RTA_HEALER, \
                                GstMSRTAHealerPrivate))

#define HEALER_INSTANCE_LOCK() \
    g_mutex_lock (priv->instance_lock)

#define HEALER_INSTANCE_UNLOCK() \
    g_mutex_unlock (priv->instance_lock)

static GstStaticPadTemplate gst_ms_rta_healer_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("application/x-rtp, "
            "media = (string) \"audio\", "
            "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
            "clock-rate = (int) [ 8000, 16000 ], "
            "encoding-name = (string) \"RTAudio\""));

static GstStaticPadTemplate gst_ms_rta_healer_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("audio/x-raw-int, "
            "signed = (bool) true, "
            "width = (int) 16, "
            "depth = (int) 16, "
            "rate = (int) 16000, "
            "channels = (int) 1, "
            "endianness = (int) " G_STRINGIFY (G_BYTE_ORDER)));

GST_BOILERPLATE (GstMSRTAHealer, gst_ms_rta_healer, GstElement,
                 GST_TYPE_ELEMENT);

/* element overrides */
static GstStateChangeReturn gst_ms_rta_healer_change_state (
  GstElement * element, GstStateChange transition);

/* sinkpad overrides */
static gboolean gst_ms_rta_healer_sink_setcaps (GstPad * pad,
                                                GstCaps * caps);
static gboolean gst_ms_rta_healer_sink_event (GstPad * pad,
                                              GstEvent * event);
static GstFlowReturn gst_ms_rta_healer_sink_chain (GstPad * pad,
                                                   GstBuffer * buffer);

/* srcpad overrides */
static gboolean gst_ms_rta_healer_src_event (GstPad * pad,
                                             GstEvent * event);
static gboolean gst_ms_rta_healer_src_query (GstPad * pad,
                                             GstQuery * query);
static gboolean gst_ms_rta_healer_src_activate_push (GstPad * pad,
                                                     gboolean active);

static void
gst_ms_rta_healer_base_init (gpointer klass)
{
  GstElementClass * element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_ms_rta_healer_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_ms_rta_healer_src_template));

  gst_element_class_set_details (element_class, &gst_ms_rta_healer_details);
}

static void
gst_ms_rta_healer_class_init (GstMSRTAHealerClass * klass)
{
  GstElementClass * gstelement_class = GST_ELEMENT_CLASS (klass);

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_type_class_add_private (klass, sizeof (GstMSRTAHealerPrivate));

  gstelement_class->change_state = gst_ms_rta_healer_change_state;

  GST_DEBUG_CATEGORY_INIT (gst_ms_rta_healer_debug, "msrtahealer", 0,
      "RTAudio healer");
}

static void
gst_ms_rta_healer_init (GstMSRTAHealer * self,
                        GstMSRTAHealerClass * klass)
{
  GstMSRTAHealerPrivate * priv = GST_MS_RTA_HEALER_GET_PRIVATE (self);
  GstElement * element = GST_ELEMENT (self);

  priv->flushing = FALSE;

  priv->sinkpad =
      gst_pad_new_from_static_template (&gst_ms_rta_healer_sink_template,
          "sink");
  gst_pad_set_setcaps_function (priv->sinkpad,
      GST_DEBUG_FUNCPTR (gst_ms_rta_healer_sink_setcaps));
  gst_pad_set_event_function (priv->sinkpad,
      GST_DEBUG_FUNCPTR (gst_ms_rta_healer_sink_event));
  gst_pad_set_chain_function (priv->sinkpad,
      GST_DEBUG_FUNCPTR (gst_ms_rta_healer_sink_chain));

  priv->srcpad =
      gst_pad_new_from_static_template (&gst_ms_rta_healer_src_template,
          "src");
  gst_pad_set_event_function (priv->srcpad,
      GST_DEBUG_FUNCPTR (gst_ms_rta_healer_src_event));
  gst_pad_set_query_function (priv->srcpad,
      GST_DEBUG_FUNCPTR (gst_ms_rta_healer_src_query));
  gst_pad_set_activatepush_function (priv->srcpad,
      GST_DEBUG_FUNCPTR (gst_ms_rta_healer_src_activate_push));

  gst_element_add_pad (element, priv->sinkpad);
  gst_element_add_pad (element, priv->srcpad);

  priv->instance_lock = g_mutex_new ();
}

static gpointer WINAPI
gst_ms_rta_healer_malloc (gsize size)
{
  gpointer ret = g_malloc (size);
  /*g_debug ("%s: %d => %p", G_STRFUNC, size, ret);*/
  return ret;
}

static void WINAPI
gst_ms_rta_healer_free (gpointer memory)
{
  /*g_debug ("%s: %p", G_STRFUNC, memory);*/
  g_free (memory);
}

static gboolean
gst_ms_rta_healer_start (GstMSRTAHealer * healer)
{
  GstMSRTAHealerPrivate * priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);
  HRESULT hr;
  gpointer p;

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  /* Create the healer instance and initialize it */
  hr = ms_rtaudio_healer_new (&priv->instance, gst_ms_rta_healer_malloc,
      gst_ms_rta_healer_free);
  if (FAILED (hr) || priv->instance == NULL)
    goto error;

  hr = ms_audio_healer_start (priv->instance, 0);
  if (FAILED (hr))
    goto error;

  /* Create a buffer stream and add two buffers to it */
  priv->stream = ms_buffer_stream_new ();
  g_assert (priv->stream != NULL);

  /* Metadata (RTP header info) */
  priv->inbuf_metadata = ms_buffer_rtp_ext_header_new ();
  g_assert (priv->inbuf_metadata != NULL);

  p = ms_buffer_stream_add_buffer (priv->stream,
      MS_BUFFER_INDEX_RTP_EXT_HEADER,
      MS_BUFFER_BASE (priv->inbuf_metadata));
  g_assert (p != NULL);

  p = ms_buffer_stream_update_offset_and_size (priv->stream,
      MS_BUFFER_INDEX_RTP_EXT_HEADER, 0,
      priv->inbuf_metadata->parent.payload_size);
  g_assert (p != NULL);

  /* Audio data in */
  priv->inbuf_audio = ms_buffer_custom_new (NULL, 0);
  g_assert (priv->inbuf_audio != NULL);

  p = ms_buffer_stream_add_buffer (priv->stream,
      MS_BUFFER_INDEX_ENCODED_MEDIA,
      MS_BUFFER_BASE (priv->inbuf_audio));
  g_assert (p != NULL);

  /* Audio data out */
  priv->outbuf_audio = ms_buffer_custom_new (NULL, 0);
  g_assert (priv->outbuf_audio != NULL);

  p = ms_buffer_stream_add_buffer (priv->stream,
      MS_BUFFER_INDEX_DECODED_MEDIA,
      MS_BUFFER_BASE (priv->outbuf_audio));
  g_assert (p != NULL);

  ms_buffer_stream_update_offset_and_size (priv->stream,
      MS_BUFFER_INDEX_DECODED_MEDIA, 0, 640);

  return TRUE;

error:
  /* TODO: post an error message */
  GST_ERROR_OBJECT (healer, "Error 0x%08x", hr);
  return FALSE;
}

static void
gst_ms_rta_healer_stop (GstMSRTAHealer * healer)
{
  GstMSRTAHealerPrivate * priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);
  HRESULT hr;

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  hr = ms_rtaudio_healer_free (priv->instance);
  if (FAILED (hr)) {
    GST_WARNING_OBJECT (healer, "Failed to destroy healer instance: 0x%08x",
        hr);
  }
  priv->instance = NULL;
}

static GstStateChangeReturn
gst_ms_rta_healer_change_state (GstElement * element,
                                GstStateChange transition)
{
  GstMSRTAHealer * healer = GST_MS_RTA_HEALER (element);
  GstStateChangeReturn ret;

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_ms_rta_healer_start (healer))
        return GST_STATE_CHANGE_FAILURE;
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      /* We are a live element as we sync to the clock */
      if (ret != GST_STATE_CHANGE_FAILURE)
        ret = GST_STATE_CHANGE_NO_PREROLL;
      break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      gst_ms_rta_healer_stop (healer);
      break;
    default:
      break;
  }

  return ret;
}

static gboolean
gst_ms_rta_healer_parse_caps (GstMSRTAHealer * healer, GstCaps * caps)
{
  GstMSRTAHealerPrivate * priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);
  GstStructure * structure;

  structure = gst_caps_get_structure (caps, 0);

  if (!gst_structure_get_int (structure, "clock-rate", &priv->clock_rate))
    goto error;

  if (priv->clock_rate != 8000 && priv->clock_rate != 16000)
    goto wrong_rate;

  return TRUE;

error:
  {
    GST_DEBUG_OBJECT (healer, "No clock-rate in caps");
    return FALSE;
  }

wrong_rate:
  {
    GST_DEBUG_OBJECT (healer, "Invalid clock-rate %d", priv->clock_rate);
    return FALSE;
  }
}

static gboolean
gst_ms_rta_healer_sink_setcaps (GstPad * pad, GstCaps * caps)
{
  GstMSRTAHealer * healer;
  GstMSRTAHealerPrivate * priv;
  gboolean res;

  healer = GST_MS_RTA_HEALER (gst_pad_get_parent (pad));
  priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);

  res = gst_ms_rta_healer_parse_caps (healer, caps);

  if (res) {
    GstCaps * src_caps;
    /*GstStructure * structure;*/

    src_caps = gst_caps_copy (gst_pad_get_pad_template_caps (priv->srcpad));
    /*
    structure = gst_caps_get_structure (src_caps, 0);
    gst_structure_set (structure, "rate", G_TYPE_INT, priv->clock_rate, NULL);
    */

    res = gst_pad_set_caps (priv->srcpad, src_caps);

    gst_caps_unref (src_caps);
  }

  gst_object_unref (healer);
  return res;
}

static gboolean
gst_ms_rta_healer_sink_event (GstPad * pad, GstEvent * event)
{
  GstMSRTAHealer * healer;
  GstMSRTAHealerPrivate * priv;
  gboolean ret;

  healer = GST_MS_RTA_HEALER (gst_pad_get_parent (pad));
  priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);

  GST_DEBUG_OBJECT (healer, "received %s", GST_EVENT_TYPE_NAME (event));

  /* TODO: handle events */
  ret = gst_pad_push_event (priv->srcpad, event);

  gst_object_unref (healer);
  return ret;
}

static GstFlowReturn
gst_ms_rta_healer_sink_chain (GstPad * pad, GstBuffer * buffer)
{
  GstMSRTAHealer * healer;
  GstMSRTAHealerPrivate * priv;
  MSBufferRtpExtHeaderData * hdr;
  HRESULT hr;

  healer = GST_MS_RTA_HEALER (gst_pad_get_parent (pad));
  priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  if (!gst_rtp_buffer_validate (buffer))
    goto invalid_buffer;

  HEALER_INSTANCE_LOCK ();

  priv->inbuf_metadata->parent.timestamp = GST_BUFFER_TIMESTAMP (buffer) / 100;

  hdr = &priv->inbuf_metadata->header_data;
  hdr->packet_ntp_timestamp = 0.02; /* not used by the RTAudio healer anyway */
  hdr->seq_no = gst_rtp_buffer_get_seq (buffer);
  hdr->timestamp = gst_rtp_buffer_get_timestamp (buffer);
  hdr->unknown_enum_value = 1;
  hdr->codec_id =
      (priv->clock_rate == 16000) ? MS_CODEC_ID_RTA16 : MS_CODEC_ID_RTA8;
  hdr->ssrc = gst_rtp_buffer_get_ssrc (buffer);
  hdr->marker_bit_set = gst_rtp_buffer_get_marker (buffer);
  hdr->unknown_bool_for_enum_set = FALSE;
  hdr->is_dtmf = FALSE;
  hdr->csrc_count = 0;

  priv->inbuf_audio->parent.payload = gst_rtp_buffer_get_payload (buffer);
  ms_buffer_stream_update_offset_and_size (priv->stream,
      MS_BUFFER_INDEX_ENCODED_MEDIA, 0,
      gst_rtp_buffer_get_payload_len (buffer));

  hr = ms_audio_healer_push_samples (priv->instance, priv->stream, 0);
  if (FAILED (hr)) {
    GST_ELEMENT_WARNING (healer, STREAM, DECODE, (NULL),
        ("MSAudioHealer::PushSamples failed, error = %08x", hr));
  }

  HEALER_INSTANCE_UNLOCK ();

  gst_buffer_unref (buffer);
  gst_object_unref (healer);
  return GST_FLOW_OK;

invalid_buffer:
  {
    GST_ELEMENT_WARNING (healer, STREAM, DECODE, (NULL),
        ("Received invalid RTP payload, dropping"));
    gst_buffer_unref (buffer);
    gst_object_unref (healer);
    return GST_FLOW_OK;
  }
}

static gboolean
gst_ms_rta_healer_src_event (GstPad * pad, GstEvent * event)
{
  GstMSRTAHealer * healer;
  GstMSRTAHealerPrivate * priv;
  gboolean ret;

  healer = GST_MS_RTA_HEALER (gst_pad_get_parent (pad));
  priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);

  GST_DEBUG_OBJECT (healer, "received %s", GST_EVENT_TYPE_NAME (event));

  ret = gst_pad_push_event (priv->sinkpad, event);

  gst_object_unref (healer);
  return ret;
}

static gboolean
gst_ms_rta_healer_src_query (GstPad * pad, GstQuery * query)
{
  GstMSRTAHealer * healer;
  GstMSRTAHealerPrivate * priv;
  gboolean res = FALSE;

  healer = GST_MS_RTA_HEALER (gst_pad_get_parent (pad));
  priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_LATENCY:
    {
      GstClockTime min_latency, max_latency;

      min_latency = 20 * GST_MSECOND;
      max_latency = min_latency;

      gst_query_set_latency (query, TRUE, min_latency, max_latency);

      res = TRUE;
      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
      break;
  }

  gst_object_unref (healer);
  return res;
}

static void
gst_ms_rta_healer_loop (void * data)
{
  GstMSRTAHealer * healer = GST_MS_RTA_HEALER (data);
  GstMSRTAHealerPrivate * priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);
  GstClock * clock;
  GstClockTime base_time;
  GstClockID id;
  GstClockTime timestamp, duration, t;

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  GST_OBJECT_LOCK (healer);
  clock = GST_ELEMENT_CLOCK (healer);
  if (clock != NULL)
    gst_object_ref (clock);
  base_time = GST_ELEMENT (healer)->base_time;
  GST_OBJECT_UNLOCK (healer);

  if (clock == NULL)
    goto beach;

  do {
    GstCaps * caps;
    GstBuffer * buffer;
    HRESULT hr;

    caps = gst_pad_get_negotiated_caps (priv->srcpad);
    if (caps == NULL)
      goto beach;

    buffer = gst_buffer_new_and_alloc (640);

    HEALER_INSTANCE_LOCK ();

    priv->outbuf_audio->parent.payload = GST_BUFFER_DATA (buffer);
    hr = ms_audio_healer_pull_samples (priv->instance, priv->stream, 0, 20);

    HEALER_INSTANCE_UNLOCK ();

    if (SUCCEEDED (hr)) {
      GstFlowReturn ret;

      timestamp = gst_clock_get_time (clock) - base_time;
      duration = 20 * GST_MSECOND;

      if (timestamp > duration)
        timestamp -= duration;
      else
        timestamp = 0;

      GST_BUFFER_OFFSET (buffer) = priv->offset++;
      GST_BUFFER_OFFSET_END (buffer) = priv->offset;
      GST_BUFFER_TIMESTAMP (buffer) = timestamp;
      GST_BUFFER_DURATION (buffer) = duration;
      GST_BUFFER_CAPS (buffer) = caps;

      GST_DEBUG_OBJECT (healer, "Pushing one buffer");

      ret = gst_pad_push (priv->srcpad, buffer);
      if (ret != GST_FLOW_OK) {
        GST_ELEMENT_WARNING (healer, STREAM, DECODE, (NULL),
            ("Failed to push buffer"));
        break;
      }
    } else {
      GST_ELEMENT_WARNING (healer, STREAM, DECODE, (NULL),
          ("MSAudioHealer::PullSamples failed, error = %08x", hr));
      gst_buffer_unref (buffer);
    }

    t = base_time + timestamp + (2 * duration);
    id = gst_clock_new_single_shot_id (clock, t);
    gst_clock_id_wait (id, NULL);
  }
  while (!priv->flushing);

beach:
  if (clock != NULL)
    gst_object_unref (clock);
}

static gboolean
gst_ms_rta_healer_src_activate_push (GstPad * pad, gboolean active)
{
  GstMSRTAHealer * healer;
  GstMSRTAHealerPrivate * priv;
  gboolean ret = TRUE;

  healer = GST_MS_RTA_HEALER (gst_pad_get_parent (pad));
  priv = GST_MS_RTA_HEALER_GET_PRIVATE (healer);

  if (active) {
    GST_DEBUG_OBJECT (healer, "Starting task on srcpad");
    gst_pad_start_task (priv->srcpad, gst_ms_rta_healer_loop, healer);
  } else {
    GST_DEBUG_OBJECT (healer, "Stopping task on srcpad");
    ret = gst_pad_stop_task (pad);
  }

  gst_object_unref (healer);
  return ret;
}
