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

#include "gstrtprtaudiodepay.h"
#include <gst/rtp/gstrtpbuffer.h>

static GstElementDetails gst_rtp_rtaudio_depay_details = {
  "RTP Depayloader for RTAudio",
  "Codec/Depayloader/Network",
  "Extracts RTAudio streams from RTP packets",
  "Ole André Vadla Ravnås <oleavr@gmail.com>"
};

GST_DEBUG_CATEGORY_STATIC (gst_rtp_rtaudio_depay_debug);
#define GST_CAT_DEFAULT (gst_rtp_rtaudio_depay_debug)

static GstStaticPadTemplate gst_rtp_rtaudio_depay_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("application/x-rtp, "
            "media = (string) \"audio\", "
            "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
            "clock-rate = (int) [ 8000, 16000 ], "
            "encoding-name = (string) \"x-msrta\""));

static GstStaticPadTemplate gst_rtp_rtaudio_depay_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("audio/x-msrta"));

GST_BOILERPLATE (GstRtpRTAudioDepay, gst_rtp_rtaudio_depay,
                 GstBaseRTPDepayload, GST_TYPE_BASE_RTP_DEPAYLOAD);

static gboolean gst_rtp_rtaudio_depay_set_caps (
  GstBaseRTPDepayload * depayload, GstCaps * caps);
static GstBuffer * gst_rtp_rtaudio_depay_process (
    GstBaseRTPDepayload * basedepayload, GstBuffer * buf);

static void
gst_rtp_rtaudio_depay_base_init (gpointer klass)
{
  GstElementClass * element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_rtaudio_depay_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_rtaudio_depay_src_template));

  gst_element_class_set_details (element_class, &gst_rtp_rtaudio_depay_details);
}

static void
gst_rtp_rtaudio_depay_class_init (GstRtpRTAudioDepayClass * klass)
{
  GstBaseRTPDepayloadClass * basedepayload_class;

  basedepayload_class = (GstBaseRTPDepayloadClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_BASE_RTP_DEPAYLOAD);

  basedepayload_class->set_caps = gst_rtp_rtaudio_depay_set_caps;
  basedepayload_class->process = gst_rtp_rtaudio_depay_process;

  GST_DEBUG_CATEGORY_INIT (gst_rtp_rtaudio_depay_debug, "rtprtaudiodepay", 0,
      "RTAudio RTP depayloader");
}

static void
gst_rtp_rtaudio_depay_init (GstRtpRTAudioDepay * self,
                            GstRtpRTAudioDepayClass * klass)
{
}

static gboolean
gst_rtp_rtaudio_depay_set_caps (GstBaseRTPDepayload * depayload,
                                GstCaps * caps)
{
  GstStructure * structure;
  gint rate;
  GstCaps * src_caps;
  gboolean ret;

  structure = gst_caps_get_structure (caps, 0);
  gst_structure_get_int (structure, "clock-rate", &rate);

  src_caps = gst_caps_new_simple ("audio/x-msrta",
      "rate", G_TYPE_INT, rate,
      "channels", G_TYPE_INT, 1,
      NULL);

  ret = gst_pad_set_caps (GST_BASE_RTP_DEPAYLOAD_SRCPAD (depayload), src_caps);

  gst_caps_unref (src_caps);

  depayload->clock_rate = rate;

  return ret;
}

static GstBuffer *
gst_rtp_rtaudio_depay_process (GstBaseRTPDepayload * basedepayload,
                               GstBuffer * buf)
{
  GstBuffer * outbuf;

  outbuf = gst_rtp_buffer_get_payload_buffer (buf);

  return outbuf;
}
