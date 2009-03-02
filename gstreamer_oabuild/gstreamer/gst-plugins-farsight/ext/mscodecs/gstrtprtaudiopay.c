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

#include <string.h>

#include "gstrtprtaudiopay.h"
#include <gst/rtp/gstrtpbuffer.h>

static GstElementDetails gst_rtp_rtaudio_pay_details = {
  "RTP Payloader for RTAudio",
  "Codec/Payloader/Network",
  "Packetize RTAudio streams into RTP packets",
  "Ole André Vadla Ravnås <oleavr@gmail.com>"
};

GST_DEBUG_CATEGORY_STATIC (gst_rtp_rtaudio_pay_debug);
#define GST_CAT_DEFAULT (gst_rtp_rtaudio_pay_debug)

static GstStaticPadTemplate gst_rtp_rtaudio_pay_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("audio/x-msrta"));

static GstStaticPadTemplate gst_rtp_rtaudio_pay_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("application/x-rtp, "
            "media = (string) \"audio\", "
            "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
            "clock-rate = (int) [ 8000, 16000 ], "
            "encoding-name = (string) \"x-msrta\""));

GST_BOILERPLATE (GstRtpRTAudioPay, gst_rtp_rtaudio_pay, GstBaseRTPPayload,
    GST_TYPE_BASE_RTP_PAYLOAD);

static gboolean gst_rtp_rtaudio_pay_setcaps (GstBaseRTPPayload * basepayload,
                                             GstCaps * caps);
static GstFlowReturn gst_rtp_rtaudio_pay_handle_buffer (GstBaseRTPPayload * basepayload,
                                                        GstBuffer * buffer);

static void
gst_rtp_rtaudio_pay_base_init (gpointer klass)
{
  GstElementClass * element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_rtaudio_pay_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_rtaudio_pay_src_template));

  gst_element_class_set_details (element_class, &gst_rtp_rtaudio_pay_details);
}

static void
gst_rtp_rtaudio_pay_class_init (GstRtpRTAudioPayClass * klass)
{
  GstBaseRTPPayloadClass * basepayload_class;

  basepayload_class = (GstBaseRTPPayloadClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_BASE_RTP_PAYLOAD);

  basepayload_class->set_caps = gst_rtp_rtaudio_pay_setcaps;
  basepayload_class->handle_buffer = gst_rtp_rtaudio_pay_handle_buffer;

  GST_DEBUG_CATEGORY_INIT (gst_rtp_rtaudio_pay_debug, "rtprtaudiopay", 0,
      "RTAudio RTP payloader");
}

static void
gst_rtp_rtaudio_pay_init (GstRtpRTAudioPay * self,
                          GstRtpRTAudioPayClass * klass)
{
}

static gboolean
gst_rtp_rtaudio_pay_setcaps (GstBaseRTPPayload * basepayload,
                             GstCaps * caps)
{
  GstStructure * structure;
  const gchar * payload_name;
  gint rate;

  structure = gst_caps_get_structure (caps, 0);

  payload_name = gst_structure_get_name (structure);
  if (g_strcasecmp ("audio/x-msrta", payload_name) != 0)
    goto wrong_caps;

  /* FIXME: rate vs clock-rate? */
  if (!gst_structure_get_int (structure, "rate", &rate))
    goto missing_clock_rate;

  if (rate != 8000 && rate != 16000)
    goto invalid_clock_rate;

  gst_basertppayload_set_options (basepayload, "audio", TRUE, "RTAudio",
      rate);

  return gst_basertppayload_set_outcaps (basepayload, NULL);

wrong_caps:
  {
    GST_ERROR_OBJECT (basepayload, "expected audio/x-msrta, got %s",
        payload_name);
    return FALSE;
  }
missing_clock_rate:
  {
    GST_ERROR_OBJECT (basepayload, "no clock-rate specified");
    return FALSE;
  }
invalid_clock_rate:
  {
    GST_ERROR_OBJECT (basepayload, "invalid clock-rate %d specified", rate);
    return FALSE;
  }
}

static GstFlowReturn
gst_rtp_rtaudio_pay_handle_buffer (GstBaseRTPPayload * basepayload,
                                   GstBuffer * buffer)
{
  guint payload_len;
  GstBuffer * outbuf;
  guint8 * payload;

  payload_len = GST_BUFFER_SIZE (buffer);
  outbuf = gst_rtp_buffer_new_allocate (payload_len, 0, 0);

  payload = gst_rtp_buffer_get_payload (outbuf);
  memcpy (payload, GST_BUFFER_DATA (buffer), payload_len);

  gst_rtp_buffer_set_marker (outbuf, TRUE);

  GST_BUFFER_TIMESTAMP (outbuf) = GST_BUFFER_TIMESTAMP (buffer);
  GST_BUFFER_DURATION (outbuf) = GST_BUFFER_DURATION (buffer);

  return gst_basertppayload_push (basepayload, outbuf);
}
