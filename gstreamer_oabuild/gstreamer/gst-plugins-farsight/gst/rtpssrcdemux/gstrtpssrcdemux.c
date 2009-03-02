/*
 * RTP SSRC Demux element
 *
 * Copyright (C) 2006 Websterwood consulting
 * @author Behan Webster <behanw@websterwood.com
 *
 * Based on rtpdemux
 * Copyright (C) 2005 Nokia Corporation.
 * by Kai Vehmanen <kai.vehmanen@nokia.com>
 *
 * Loosely based on GStreamer gstdecodebin
 * Copyright (C) <2004> Wim Taymans <wim@fluendo.com>
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
#include <gst/gst.h>
#include <gst/rtp/gstrtpbuffer.h>

/* generic templates */
static GstStaticPadTemplate rtp_ssrcdemux_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "clock-rate = (int) [ 0, 2147483647 ]")
    );

static GstStaticPadTemplate rtp_ssrcdemux_src_template =
GST_STATIC_PAD_TEMPLATE ("src%d",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gst_rtp_ssrcdemux_debug);
#define GST_CAT_DEFAULT gst_rtp_ssrcdemux_debug

#define GST_TYPE_RTP_DEMUX            (gst_rtp_ssrcdemux_get_type())
#define GST_RTP_DEMUX(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTP_DEMUX,GstRtpSsrcDemux))
#define GST_RTP_DEMUX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTP_DEMUX,GstRtpSsrcDemuxClass))
#define GST_IS_RTP_DEMUX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTP_DEMUX))
#define GST_IS_RTP_DEMUX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTP_DEMUX))

typedef struct _GstRtpSsrcDemux GstRtpSsrcDemux;
typedef struct _GstRtpSsrcDemuxClass GstRtpSsrcDemuxClass;
typedef struct _GstRtpSsrcDemuxPad GstRtpSsrcDemuxPad;

struct _GstRtpSsrcDemux
{
  GstElement parent;  /**< parent class */

  GstPad *sink;       /**< the sink pad */
  GSList *srcpads;    /**< a linked list of GstRtpSsrcDemuxPad objects */
  guint16 num_pads;
  guint16 max_pads;
};

struct _GstRtpSsrcDemuxClass
{
  GstElementClass parent_class;

  /* signal emmited when a new SSRC is found from a new incoming stream */
  void (* new_ssrc) (GstElement *element, gint ssrc, GstPad *pad);
  /* signal emmited when a SSRC is no longer used in an incoming stream */
  void (* remove_ssrc) (GstElement *element, gint ssrc, GstPad *pad);
};

/**
 * Item for storing GstPad<->ssrc pairs.
 */
struct _GstRtpSsrcDemuxPad {
  GstPad *pad;        /**< pointer to the actual pad */
  gint ssrc;          /**< RTP SSRC attached to pad */
};

/* props */
enum
{
  ARG_0,
  ARG_MAX_PADS,
  LAST_ARG
};

/* signals */
enum
{
  SIGNAL_NEW_SSRC,
  SIGNAL_REMOVE_SSRC,
  LAST_SIGNAL
};

#define DEBUG_INIT(bla) \
  GST_DEBUG_CATEGORY_INIT (gst_rtp_ssrcdemux_debug, "rtpssrcdemux", 0, "RTP SSRC Demux");

GST_BOILERPLATE_FULL (GstRtpSsrcDemux, gst_rtp_ssrcdemux, GstElement, GST_TYPE_ELEMENT, DEBUG_INIT);

static void gst_rtp_ssrcdemux_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_rtp_ssrcdemux_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

static void                 gst_rtp_ssrcdemux_dispose      (GObject       *object);

static void                 gst_rtp_ssrcdemux_release      (GstElement    *element);
static gboolean             gst_rtp_ssrcdemux_setup        (GstElement    *element);

static GstFlowReturn        gst_rtp_ssrcdemux_chain        (GstPad        *pad,
                                                        GstBuffer     *buf);
static GstStateChangeReturn gst_rtp_ssrcdemux_change_state (GstElement    *element,
                                                        GstStateChange transition);

static void                lru_pad_for_ssrc              (GstRtpSsrcDemux   *rtpssrcdemux,
#if 0
                                                           GstRtpSsrcDemuxPad *pad);
#else
                                                           GSList *pad);
#endif
static GstPad              *find_pad_for_ssrc            (GstRtpSsrcDemux   *rtpssrcdemux,
                                                           guint8         ssrc);
static GstPad              *reuse_pad_for_ssrc           (GstRtpSsrcDemux   *rtpssrcdemux,
                                                           guint8         ssrc);
static GstPad              *new_pad_for_ssrc             (GstRtpSsrcDemux   *rtpssrcdemux,
                                                           guint8         ssrc);

static guint gst_rtp_ssrcdemux_signals[LAST_SIGNAL] = { 0 };

static GstElementDetails gst_rtp_ssrcdemux_details = {
  "RTP SSRC Demux",
  /* XXX: what's the correct hierarchy? */
  "Codec/SsrcDemux/Network",
  "Demuxes RTP Streams (each with it's own SSRC) received as a part of the same Multicast group",
  "Behan Webster <behanw@websterwood.com>"
};

static void
gst_rtp_ssrcdemux_base_init (gpointer g_class)
{
  GstElementClass *gstelement_klass = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (gstelement_klass,
      gst_static_pad_template_get (&rtp_ssrcdemux_sink_template));
  gst_element_class_add_pad_template (gstelement_klass,
      gst_static_pad_template_get (&rtp_ssrcdemux_src_template));

  gst_element_class_set_details (gstelement_klass, &gst_rtp_ssrcdemux_details);
}

static void
gst_rtp_ssrcdemux_class_init (GstRtpSsrcDemuxClass * klass)
{
  GObjectClass *gobject_klass;
  GstElementClass *gstelement_klass;

  gobject_klass = (GObjectClass *) klass;
  gstelement_klass = (GstElementClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_klass->set_property = gst_rtp_ssrcdemux_set_property;
  gobject_klass->get_property = gst_rtp_ssrcdemux_get_property;

  g_object_class_install_property (gobject_klass, ARG_MAX_PADS,
      g_param_spec_uint ("max_pads", "Maximum number of src pads",
            "Maximum number of src pads to create.  After this number of pads, the least recently used src pads will be reused. 0 means an unlimited number of pads (currently up to 2^16 pads)",
            0, G_MAXUINT16, 0, G_PARAM_READWRITE));

  gst_rtp_ssrcdemux_signals[SIGNAL_NEW_SSRC] =
      g_signal_new ("new-ssrc",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (GstRtpSsrcDemuxClass, new_ssrc),
            NULL, NULL,
            g_cclosure_marshal_VOID__UINT_POINTER,
            G_TYPE_NONE,
            2,
            G_TYPE_INT,
            GST_TYPE_PAD);

  gst_rtp_ssrcdemux_signals[SIGNAL_REMOVE_SSRC] =
      g_signal_new ("remove-ssrc",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (GstRtpSsrcDemuxClass, remove_ssrc),
            NULL, NULL,
            g_cclosure_marshal_VOID__UINT_POINTER,
            G_TYPE_NONE,
            2,
            G_TYPE_INT,
            GST_TYPE_PAD);

  gobject_klass->dispose = GST_DEBUG_FUNCPTR (gst_rtp_ssrcdemux_dispose);

  gstelement_klass->change_state =
      GST_DEBUG_FUNCPTR (gst_rtp_ssrcdemux_change_state);
}

static void
gst_rtp_ssrcdemux_init (GstRtpSsrcDemux *rtp_ssrcdemux,
                    GstRtpSsrcDemuxClass *g_class)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS(rtp_ssrcdemux);

  rtp_ssrcdemux->sink = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "sink"), "sink");
  g_assert (rtp_ssrcdemux->sink != NULL);

  gst_pad_set_chain_function (rtp_ssrcdemux->sink, gst_rtp_ssrcdemux_chain);

  gst_element_add_pad (GST_ELEMENT(rtp_ssrcdemux), rtp_ssrcdemux->sink);
}

static void
gst_rtp_ssrcdemux_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstRtpSsrcDemux *rtp_ssrc_demux;

  g_return_if_fail (GST_IS_RTP_DEMUX (object));

  rtp_ssrc_demux = GST_RTP_DEMUX (object);

  GST_DEBUG("Setting props");

  switch (prop_id) {
    case ARG_MAX_PADS:
      rtp_ssrc_demux->max_pads = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_rtp_ssrcdemux_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstRtpSsrcDemux *rtp_ssrc_demux;

  g_return_if_fail (GST_IS_RTP_DEMUX (object));

  rtp_ssrc_demux = GST_RTP_DEMUX (object);

  switch (prop_id) {
    case ARG_MAX_PADS:
      g_value_set_uint (value, rtp_ssrc_demux->max_pads);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_rtp_ssrcdemux_dispose (GObject *object)
{
  gst_rtp_ssrcdemux_release (GST_ELEMENT (object));

  if (G_OBJECT_CLASS (parent_class)->dispose) {
    G_OBJECT_CLASS (parent_class)->dispose (object);
  }
}

static GstFlowReturn
gst_rtp_ssrcdemux_chain (GstPad *pad, GstBuffer *buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstRtpSsrcDemux *rtpssrcdemux;
  guint8 ssrc;
  GstPad *srcpad;

  g_return_val_if_fail (gst_rtp_buffer_validate (buf), GST_FLOW_ERROR);

  rtpssrcdemux = GST_RTP_DEMUX (GST_OBJECT_PARENT (pad));
  ssrc = gst_rtp_buffer_get_ssrc (buf);

  /* Look for previously used ssrc->pad mapping */
  //GST_DEBUG ("Look for SSRC=%d", ssrc);
  srcpad = find_pad_for_ssrc (rtpssrcdemux, ssrc);
  if (srcpad == NULL) {
    /* See if we've exceeded the maximum number of pads we can create */
    if ( rtpssrcdemux->max_pads && rtpssrcdemux->num_pads >= rtpssrcdemux->max_pads) {
      /* Reuse the Least Recently used pad,
       * making the assumption that it's no longer used */
      srcpad = reuse_pad_for_ssrc (rtpssrcdemux, ssrc);
    } else {
      /* Create new SSRC-> pad mapping */
      GstElement *element = GST_ELEMENT(GST_OBJECT_PARENT (pad));
      srcpad = new_pad_for_ssrc (rtpssrcdemux, ssrc);

      /* This could break with gstreamer 0.10.9 */
      gst_pad_set_active (srcpad, TRUE);

      /* XXX: set _link () function */
      gst_element_add_pad (element, srcpad);
    }
  }

  /* push buf to srcpad */
  if (srcpad) {
    //GST_DEBUG ("Push SSRC=%d to PAD:%08X", ssrc, srcpad);
    gst_pad_push (srcpad, GST_BUFFER (buf));
  }

  return ret;
}

#if 0
/* Put the found pad at the end of ssrcpads list for LRU */
static void
lru_pad_for_ssrc (GstRtpSsrcDemux *rtpssrcdemux, GstRtpSsrcDemuxPad *pad)
{
  //GST_DEBUG ("Most recent pad SSRC=%d PAD:%08X", pad->ssrc, pad->pad);

  GSList *item;
  item = rtpssrcdemux->srcpads;
  item = g_slist_remove (item, pad);
  item = g_slist_append (item, pad);
  rtpssrcdemux->srcpads = item;
}
#else
/* Put the found pad at the end of ssrcpads list for LRU */
static void
lru_pad_for_ssrc (GstRtpSsrcDemux *rtpssrcdemux, GSList *pad)
{
  //GST_DEBUG ("Most recent pad SSRC=%d PAD:%08X", pad->ssrc, pad->pad);

  GSList *item = rtpssrcdemux->srcpads;
  item = g_slist_remove_link (item, pad);
  item = g_slist_concat (item, pad);
  rtpssrcdemux->srcpads = item;
}
#endif

/* Look for previous SSRC->pad mapping
 */
static GstPad *
find_pad_for_ssrc (GstRtpSsrcDemux *rtpssrcdemux, guint8 ssrc)
{
  GSList *item = rtpssrcdemux->srcpads;
  for (; item ; item = g_slist_next (item)) {
    GstRtpSsrcDemuxPad *pad = item->data;
    if (pad->ssrc == ssrc) {
      //lru_pad_for_ssrc(rtpssrcdemux, pad);
      lru_pad_for_ssrc(rtpssrcdemux, item);
      return pad->pad;
    }
  }

  return NULL;
}

/* If we are at our maximum number of pads, reuse the least recently used pad.
 * The LRU pad should be the first one in the list.
 */
static GstPad *
reuse_pad_for_ssrc (GstRtpSsrcDemux *rtpssrcdemux, guint8 ssrc)
{
  GSList *item = rtpssrcdemux->srcpads;
  GstRtpSsrcDemuxPad *pad = item->data;
  GstPad *srcpad = pad->pad;

  GST_DEBUG ("emitting remove-ssrc for SSRC=%d", ssrc);
  g_signal_emit (G_OBJECT (rtpssrcdemux),
          gst_rtp_ssrcdemux_signals[SIGNAL_REMOVE_SSRC], 0, pad->ssrc, srcpad);

  GST_DEBUG ("Reusing pad SSRC %d->%d PAD:%08X", pad->ssrc, ssrc, srcpad);
  pad->ssrc = ssrc;

  //lru_pad_for_ssrc(rtpssrcdemux, pad);
  lru_pad_for_ssrc(rtpssrcdemux, item);

  return srcpad;
}

/* Create new SSRC->pad mapping
 */
static GstPad *
new_pad_for_ssrc (GstRtpSsrcDemux *rtpssrcdemux, guint8 ssrc)
{
  GstElementClass *klass;
  GstPadTemplate *templ;
  gchar *padname;
  GstPad *srcpad;

  GST_DEBUG ("New srcpad for SSRC=%d", ssrc);

  /* new SSRC, create a src pad */
  klass = GST_ELEMENT_GET_CLASS (rtpssrcdemux);
  templ = gst_element_class_get_pad_template (klass, "src%d");
  padname = g_strdup_printf ("src%d", ssrc);
  srcpad = gst_pad_new_from_template (templ, padname);
  g_free (padname);

  if (srcpad) {
    /* Add SSRC->pad mapping to list */
    GstRtpSsrcDemuxPad *rtpssrcdemuxpad;
    GST_DEBUG ("Adding SSRC=%d -> %08X to the list.", ssrc, srcpad);
    rtpssrcdemuxpad = g_new0 (GstRtpSsrcDemuxPad, 1);
    rtpssrcdemuxpad->ssrc = ssrc;
    rtpssrcdemuxpad->pad = srcpad;
    rtpssrcdemux->srcpads = g_slist_append (rtpssrcdemux->srcpads, rtpssrcdemuxpad);
    rtpssrcdemux->num_pads++;

    GST_DEBUG ("emitting new-ssrc for SSRC=%d", ssrc);
    g_signal_emit (G_OBJECT (rtpssrcdemux),
            gst_rtp_ssrcdemux_signals[SIGNAL_NEW_SSRC], 0, ssrc, srcpad);
  }

  return srcpad;
}

/**
 * Reserves resources for the object.
 */
static gboolean
gst_rtp_ssrcdemux_setup (GstElement * element)
{
  GstRtpSsrcDemux *rtp_ssrcdemux = GST_RTP_DEMUX (element);
  gboolean res = TRUE;

  if (rtp_ssrcdemux) {
    rtp_ssrcdemux->srcpads = NULL;
  }

  return res;
}

/**
 * Free resources for the object.
 */
static void
gst_rtp_ssrcdemux_release (GstElement * element)
{
  GstRtpSsrcDemux *rtp_ssrcdemux = GST_RTP_DEMUX (element);

  if (rtp_ssrcdemux) {
    /* note: GstElement's dispose() will handle the pads */
    g_slist_free (rtp_ssrcdemux->srcpads);
    rtp_ssrcdemux->srcpads = NULL;
  }
}

static GstStateChangeReturn
gst_rtp_ssrcdemux_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstRtpSsrcDemux *rtp_ssrcdemux;

  rtp_ssrcdemux = GST_RTP_DEMUX (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (gst_rtp_ssrcdemux_setup (element) != TRUE)
        ret = GST_STATE_CHANGE_FAILURE;
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      gst_rtp_ssrcdemux_release (element);
      break;
    default:
      break;
  }

  return ret;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_rtp_ssrcdemux_debug,
          "rtpssrcdemux", 0, "RTP codec SSRC demuxer");

  return gst_element_register (plugin,
          "rtpssrcdemux", GST_RANK_NONE, GST_TYPE_RTP_DEMUX);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "rtpssrcdemux",
    "RTP codec SSRC demuxer",
    plugin_init,
    VERSION,
    "LGPL",
    "Farsight",
    "http://farsight.sf.net")
