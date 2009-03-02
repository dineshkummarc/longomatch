/* GStreamer
 * Copyright (C) <2007> Nokia Corporation
 * Copyright (C) <2007> Collabora Ltd
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
#include <config.h>
#endif

#include <gst/rtp/gstrtpbuffer.h>

#include "gstrtpg729depay.h"

static gboolean
gst_rtp_g729_depay_set_caps (GstBaseRTPDepayload * depayload, GstCaps * caps);
static GstBuffer *
gst_rtp_g729_depay_process (GstBaseRTPDepayload * depayload, GstBuffer * buf);

static const GstElementDetails gst_rtp_g729_depay_details =
GST_ELEMENT_DETAILS ("G729 RTP packet depayloader",
    "Codec/Depayloader/Network",
    "Extract G729 audio from RTP packets",
    "Nokia <unknown@nokia.com>");

static GstStaticPadTemplate gst_rtp_g729_depay_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\", "
        "clock-rate = (int) 8000, "
        "encoding-name = (string) \"G729\"")
    );

static GstStaticPadTemplate gst_rtp_g729_depay_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/g729, " /* according to RFC 3555 */
        "channels = (int) 1, "
        "rate = (int) 8000")
    );

GST_BOILERPLATE (GstRTPG729Depay, gst_rtp_g729_depay, GstBaseRTPDepayload,
    GST_TYPE_BASE_RTP_DEPAYLOAD);

static void
gst_rtp_g729_depay_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_g729_depay_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_g729_depay_src_template));
  gst_element_class_set_details (element_class, &gst_rtp_g729_depay_details);
}

static void
gst_rtp_g729_depay_class_init (GstRTPG729DepayClass * klass)
{
  GstBaseRTPDepayloadClass *depayload_class =
      GST_BASE_RTP_DEPAYLOAD_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  depayload_class->process = gst_rtp_g729_depay_process;
  depayload_class->set_caps = gst_rtp_g729_depay_set_caps;
}

static void
gst_rtp_g729_depay_init (GstRTPG729Depay * depay,
                         GstRTPG729DepayClass * klass)
{
  GstBaseRTPDepayload *depayload = GST_BASE_RTP_DEPAYLOAD (depay);

  depayload->clock_rate = 8000;
  gst_pad_use_fixed_caps (GST_BASE_RTP_DEPAYLOAD_SRCPAD (depayload));
}

static gboolean
gst_rtp_g729_depay_set_caps (GstBaseRTPDepayload * depayload, GstCaps * caps)
{
  GstCaps *srccaps;
  gboolean ret;

  GstStructure *structure = gst_caps_get_structure (caps, 0);
  gint clock_rate = 8000;      /* default */

  gst_structure_get_int (structure, "clock-rate", &clock_rate);
  depayload->clock_rate = clock_rate;

  srccaps = gst_caps_new_simple ("audio/g729", 
      "channels", G_TYPE_INT, 1, "rate", G_TYPE_INT, 8000, NULL);
  if (srccaps == NULL)
    return FALSE;
  ret = gst_pad_set_caps (GST_BASE_RTP_DEPAYLOAD_SRCPAD (depayload), srccaps);
  gst_caps_unref (srccaps);

  return ret;
}

#define G729_FRAME_SIZE 10
#define G729B_CN_FRAME_SIZE 2

static GstBuffer *
gst_rtp_g729_depay_process (GstBaseRTPDepayload * depayload, GstBuffer * buf)
{
  guint len;

  GST_DEBUG ("process: got %d bytes, mark %d, ts %u, seqn %d",
      GST_BUFFER_SIZE (buf), gst_rtp_buffer_get_marker (buf),
      gst_rtp_buffer_get_timestamp (buf), gst_rtp_buffer_get_seq (buf));

  len = gst_rtp_buffer_get_payload_len (buf);
  if (len % G729_FRAME_SIZE != 0 &&
      len % G729_FRAME_SIZE != G729B_CN_FRAME_SIZE)
    return NULL;

  return gst_rtp_buffer_get_payload_buffer (buf);
}

gboolean
gst_rtp_g729_depay_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpg729depay",
      GST_RANK_NONE, GST_TYPE_RTP_G729_DEPAY);
}
