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

/*
 * RTP DV depayloader.
 *
 * Important note for NTSC-users:
 *
 * Because the author uses PAL video, and he does not have proper DV
 * documentation (the DV format specification is not freely available),
 * this code may very well contain PAL-specific assumptions.
 */

#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>

#include "gstdvdepayload.h"

GST_DEBUG_CATEGORY (rtpdvdepay_debug);
#define GST_CAT_DEFAULT (rtpdvdepay_debug)
/* Filter signals and args */
enum {
    /* FILL ME */
    LAST_SIGNAL
};

enum {
    ARG_0,
};

static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-dv")
    );

static GstStaticPadTemplate sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = video,"
        "clock-rate = (int) 9000")
    );

static void gst_dvdepayload_class_init (GstDVDepayloadClass *klass);
static void gst_dvdepayload_base_init (GstDVDepayloadClass *klass);
static void gst_dvdepayload_init (GstDVDepayload *filter);
static void gst_dvdepayload_finalize (GObject * object);
static void gst_dvdepayload_set_property (GObject *object, guint prop_id,
        const GValue *value, GParamSpec *pspec);
static void gst_dvdepayload_get_property (GObject *object, guint prop_id,
        GValue *value, GParamSpec *pspec);
static GstBuffer *gst_dvdepayload_process (GstBaseRTPDepayload *base, 
        GstBuffer *in);
static int calculate_difblock_location( guint8 *block );

static GstElementClass *parent_class = NULL;
static GstPad *sinkpad, *srcpad;

GType
gst_dvdepayload_get_type (void)
{
    static GType plugin_type = 0;

    if (!plugin_type)
    {
        static const GTypeInfo plugin_info =
        {
            sizeof (GstDVDepayloadClass),
            (GBaseInitFunc) gst_dvdepayload_base_init,
            NULL,
            (GClassInitFunc) gst_dvdepayload_class_init,
            NULL,
            NULL,
            sizeof (GstDVDepayload),
            0,
            (GInstanceInitFunc) gst_dvdepayload_init,
        };
        plugin_type = g_type_register_static (GST_TYPE_BASE_RTP_DEPAYLOAD,
                "GstDVDepayload",
                &plugin_info, 0);
    }
    return plugin_type;
}

static void
gst_dvdepayload_base_init (GstDVDepayloadClass *klass)
{
    static GstElementDetails plugin_details = {
        "RTP DV Depayloader",
        "Codec/Depayloader/Network",
        "Depayloads DV from RTP packets",
        "Marcel Moreaux <marcelm@spacelabs.nl>"
    };
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

    gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&src_factory));
    gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&sink_factory));

    gst_element_class_set_details (element_class, &plugin_details);
}

/* initialize the plugin's class */
static void
gst_dvdepayload_class_init (GstDVDepayloadClass *klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
    GstBaseRTPDepayloadClass *gstbasertpdepayload_class;

    gobject_class = (GObjectClass*) klass;
    gstelement_class = (GstElementClass*) klass;
    gstbasertpdepayload_class = (GstBaseRTPDepayloadClass*) klass;

    parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

    gobject_class->set_property = gst_dvdepayload_set_property;
    gobject_class->get_property = gst_dvdepayload_get_property;

    gobject_class->finalize = gst_dvdepayload_finalize;

    gstbasertpdepayload_class->process = gst_dvdepayload_process;

    GST_DEBUG_CATEGORY_INIT (rtpdvdepay_debug, "rtpdvdepay", 0, "DV RTP Depayloader");
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_dvdepayload_init (GstDVDepayload *filter)
{
    GstElementClass *klass = GST_ELEMENT_GET_CLASS (filter);
    GstBaseRTPDepayload *base = GST_BASE_RTP_DEPAYLOAD (filter);

    // clock rate for this payload type
    base->clock_rate = 8000;

    sinkpad = gst_pad_new_from_template( gst_element_class_get_pad_template (klass, "sink"), "sink");
    srcpad = gst_pad_new_from_template( gst_element_class_get_pad_template (klass, "src"), "src");
}

static void gst_dvdepayload_finalize (GObject * object)
{
    if (G_OBJECT_CLASS (parent_class)->finalize)
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* Process one RTP packet. Accumulate RTP payload in the proper place in a DV
 * frame, and return that frame if we detect a new frame, or NULL otherwise.
 * We assume a DV frame is 144000 bytes. That should accomodate PAL as well as
 * NTSC.
 */
static GstBuffer *gst_dvdepayload_process (GstBaseRTPDepayload *base, GstBuffer *in)
{
    /* This is the accumulator frame. Packets are assembled into this frame
     * until we detect a new frame. */
    static GstBuffer *acc = NULL;
    static int prevts = 0, frame = -1, header_received = 0;

    GstBuffer *out = NULL;
    guint8 *payload;
    int payload_len, location;

    GST_DEBUG ("process : got %d bytes, mark %d rtp-ts %u seqn %d, gst ts %"
            GST_TIME_FORMAT, 
            GST_BUFFER_SIZE (in),
            gst_rtp_buffer_get_marker (in),
            gst_rtp_buffer_get_timestamp (in),
            gst_rtp_buffer_get_seq (in),
            GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (in)));

    /* Check if the received packet contains (the start of) a new frame */
    if (gst_rtp_buffer_get_timestamp(in) != prevts) {
      prevts = gst_rtp_buffer_get_timestamp(in);
      frame++;
      GST_DEBUG ("NEW FRAME %i at %u", frame, prevts);

      /* If the accumulator frame is non-NULL, make sure it is returned. */
      if (acc != NULL && header_received) {
        GST_DEBUG ("RETURNING FRAME %i ts %" GST_TIME_FORMAT, frame - 1,
           GST_TIME_ARGS(GST_BUFFER_TIMESTAMP(acc)));
        out = acc;
      }

      /* Allocate a new accumulator frame. */
      acc = gst_buffer_new_and_alloc (144000);

      /* Initialize the new accumulator frame.
       * If the previous frame exists, copy that into the accumulator frame.
       * This way, missing packets in the stream won't show up badly. */
      if (out != NULL)
        memcpy (GST_BUFFER_DATA(acc), GST_BUFFER_DATA(out), 144000);
      else
        memset (GST_BUFFER_DATA(acc), 0, 144000);
    }

    /* Extract the payload */
    payload_len = gst_rtp_buffer_get_payload_len(in);
    payload = gst_rtp_buffer_get_payload(in);

    /* Calculate where in the frame the payload should go */
    location = calculate_difblock_location (payload);

    /* Check if we received a header. We will not pass on frames until
     * we've received a header, otherwise the DV decoder goes wacko. */
    if (location == 0)
      header_received = 1;

    /* And copy it in, provided the location is sane. */
    if(location >= 0 && location <= 144000 - payload_len)
      memcpy (GST_BUFFER_DATA(acc) + location, payload, payload_len);

    return out;
}

/* A DV frame consists of a bunch of 80-byte DIF blocks.
 * Each DIF block contains a 3-byte header telling where in the DV frame the
 * DIF block should go. We use this information to calculate its position.
 */
static int calculate_difblock_location (guint8 *block)
{
    int block_type, dif_sequence, dif_block, location;

    block_type = block[0] >> 5;
    dif_sequence = block[1] >> 4;
    dif_block = block[2];

    switch (block_type)
    {
      case 0: /* Header block */
        location = dif_sequence * 150 * 80;
        break;
      case 1: /* Subcode block */
        location = dif_sequence * 150 * 80 + (1 + dif_block) * 80;
        break;
      case 2: /* VAUX block */
        location = dif_sequence * 150 * 80 + (3 + dif_block) * 80;
        break;
      case 3: /* Audio block */
        location = dif_sequence * 150 * 80 + (6 + dif_block * 16) * 80;
        break;
      case 4: /* Video block */
        location = dif_sequence * 150 * 80 +
            (7 + (dif_block / 15) + dif_block) * 80;
        break;
      default: /* Something bogus */
        GST_DEBUG ("UNKNOWN BLOCK");
        location = -1;
        break;
    }

    return location;
}

static void
gst_dvdepayload_set_property (GObject *object, guint prop_id,
        const GValue *value, GParamSpec *pspec)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gst_dvdepayload_get_property (GObject *object, guint prop_id,
        GValue *value, GParamSpec *pspec)
{
    switch (prop_id) 
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

gboolean
gst_dvdepayload_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpdvdepay",
      GST_RANK_NONE, GST_TYPE_DVDEPAYLOAD);
}
