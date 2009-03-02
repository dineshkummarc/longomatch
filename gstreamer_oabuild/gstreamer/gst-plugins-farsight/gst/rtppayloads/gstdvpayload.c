/* Farsight
 * Copyright (C) 2006 Marcel Moreaux <marcelm@spacelabs.nl>
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
#  include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <gst/rtp/gstrtpbuffer.h>

#include "gstdvpayload.h"

GST_DEBUG_CATEGORY (rtpdvpay_debug);
#define GST_CAT_DEFAULT (rtpdvpay_debug)

/* Elementfactory information */
static GstElementDetails gst_dvpayload_details = {
  "RTP DV Payloader",
  "Codec/Payloader/Network",
  "Payloads DV into RTP packets",
  "Marcel Moreaux <marcelm@spacelabs.nl>"
};

static GstStaticPadTemplate gst_dvpayload_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-dv, "
        "format = PAL")
    );

static GstStaticPadTemplate gst_dvpayload_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = video,"
        "clock-rate = (int) 9000")
    );

static void gst_dvpayload_class_init (GstDVPayloadClass * klass);
static void gst_dvpayload_base_init (GstDVPayloadClass * klass);
static void gst_dvpayload_init (GstDVPayload * dvpayload);

static gboolean gst_dvpayload_setcaps (GstBaseRTPPayload * payload,
    GstCaps * caps);
static GstFlowReturn gst_dvpayload_handle_buffer (GstBaseRTPPayload * payload,
    GstBuffer * buffer);

static GstBaseRTPPayloadClass *parent_class = NULL;

static GType
gst_dvpayload_get_type (void)
{
  static GType dvpayload_type = 0;

  if (!dvpayload_type) {
    static const GTypeInfo dvpayload_info = {
      sizeof (GstDVPayloadClass),
      (GBaseInitFunc) gst_dvpayload_base_init,
      NULL,
      (GClassInitFunc) gst_dvpayload_class_init,
      NULL,
      NULL,
      sizeof (GstDVPayload),
      0,
      (GInstanceInitFunc) gst_dvpayload_init,
    };

    dvpayload_type =
        g_type_register_static (GST_TYPE_BASE_RTP_PAYLOAD, "GstDVPayload",
        &dvpayload_info, 0);
  }
  return dvpayload_type;
}

static void
gst_dvpayload_base_init (GstDVPayloadClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_dvpayload_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_dvpayload_src_template));
  gst_element_class_set_details (element_class, &gst_dvpayload_details);
}

static void
gst_dvpayload_class_init (GstDVPayloadClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPPayloadClass *gstbasertppayload_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertppayload_class = (GstBaseRTPPayloadClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_BASE_RTP_PAYLOAD);

  gstbasertppayload_class->set_caps = gst_dvpayload_setcaps;
  gstbasertppayload_class->handle_buffer = gst_dvpayload_handle_buffer;

  GST_DEBUG_CATEGORY_INIT (rtpdvpay_debug, "rtpdvpay", 0, "DV RTP Payloader");
}

static void
gst_dvpayload_init (GstDVPayload * dvpayload)
{
}

static gboolean
gst_dvpayload_setcaps (GstBaseRTPPayload * payload, GstCaps * caps)
{
  GstDVPayload *dvpayload;

  dvpayload = GST_RTPDVPAYLOAD (payload);

  gst_basertppayload_set_options (payload, "audio", TRUE, "DV", 8000);

  return TRUE;
}

/* Get a DV frame, chop it up in pieces, and push the pieces to the RTP layer.
 */
static GstFlowReturn
gst_dvpayload_handle_buffer (GstBaseRTPPayload * basepayload,
    GstBuffer * buffer)
{
  GstDVPayload *dvpayload;
  guint buffer_len, current, payload_len, max_payload_size;
  GstBuffer *outbuf;
  GstFlowReturn ret = GST_FLOW_OK;
  int hdrlen;

  dvpayload = GST_RTPDVPAYLOAD (basepayload);

  hdrlen = gst_rtp_buffer_calc_header_len (0);
  /* DV frames are made up from a bunch of DIF blocks. DIF blocks are 80 bytes
   * each, and we should put an integral number of them in each RTP packet.
   * Therefor, we round the available room down to the nearest multiple of 80.
   *
   * The available room is just the packet MTU, minus the RTP header length. */
  max_payload_size = ((GST_BASE_RTP_PAYLOAD_MTU(dvpayload) - hdrlen) / 80) * 80;

  /* The length of the buffer to transmit. */
  buffer_len = GST_BUFFER_SIZE (buffer);

  GST_DEBUG ("DV RTP payloader got buffer of %i bytes, splitting in %i byte "
      "payload fragments, at time %" GST_TIME_FORMAT, buffer_len, max_payload_size,
      GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buffer)) );

  /* This is an index in the buffer, indexing the first byte that's not yet
   * sent. */
  current = 0;

  while (current < buffer_len)
  {
    /* payload_len is the number of bytes to put in this packet.
     * Try and fit in the rest of this frame. */
    payload_len = buffer_len - current;
    /* If it exceeds the maximum payload size, trim it. */
    if( payload_len > max_payload_size)
      payload_len = max_payload_size;

    /* Allocate a new buffer, set the timestamp, and put in the payload */
    outbuf = gst_rtp_buffer_new_allocate (payload_len, 0, 0);
    GST_BUFFER_TIMESTAMP (outbuf) = GST_BUFFER_TIMESTAMP (buffer);
    memcpy (gst_rtp_buffer_get_payload(outbuf),
        GST_BUFFER_DATA(buffer) + current, payload_len);

    /* Push out the created piece, and check for errors. */
    ret = gst_basertppayload_push (basepayload, outbuf);
    if( ret != 0 )
    {
      gst_buffer_unref (buffer);
      return ret;
    }

    current += payload_len;
  }

  gst_buffer_unref (buffer);
  return 0;
}

gboolean
gst_dvpayload_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpdvpay",
      GST_RANK_NONE, GST_TYPE_RTPDVPAYLOAD);
}
