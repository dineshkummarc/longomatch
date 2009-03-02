/*
 * Farsight
 * GStreamer RTP Session element using JRTPlib
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
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

#include <gst/gst.h>

/* include this header if you want to use dynamic parameters
#include <gst/control/control.h>
*/

#include "gstrtpsession.h"

GST_DEBUG_CATEGORY (rtpsession_debug);
#define GST_CAT_DEFAULT (rtpsession_debug)

/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_DESTADDRS,
  ARG_LOCALPORTBASE,
  ARG_DEFAULT_PT,
  ARG_DEFAULT_TSINC,
  ARG_DEFAULT_MARK,
  ARG_SILENT
};

// takes in src info events interleaved with rtp packets
static GstStaticPadTemplate rtp_sink_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtpsink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

// takes in src info events interleaved with rtcp packets
static GstStaticPadTemplate rtcp_sink_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtcpsink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

// takes in full RTP packeta to send out
static GstStaticPadTemplate data_sink_factory =
GST_STATIC_PAD_TEMPLATE (
  "datasink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS("application/x-rtp-noheader")
//  GST_STATIC_CAPS ("ANY")
);

// gives out dest info events interleaved with rtp packets
static GstStaticPadTemplate rtp_src_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtpsrc",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

// gives out dest info events interleaved with rtcp packets
static GstStaticPadTemplate rtcp_src_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtcpsrc",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS("application/x-rtp-noheader")
  //GST_STATIC_CAPS ("ANY")
);

// gives out received RTP packets
static GstStaticPadTemplate data_src_factory =
GST_STATIC_PAD_TEMPLATE (
  "datasrc",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

static void	gst_rtpsession_class_init	(GstRTPSessionClass *klass);
static void	gst_rtpsession_base_init	(GstRTPSessionClass *klass);
static void	gst_rtpsession_init	(GstRTPSession *filter);

static void	gst_rtpsession_set_property(GObject *object, guint prop_id,
        const GValue *value,
        GParamSpec *pspec);
static void	gst_rtpsession_get_property(GObject *object, guint prop_id,
        GValue *value,
        GParamSpec *pspec);

static void	gst_rtpsession_update_plugin(const GValue *value,
        gpointer data);
static void	gst_rtpsession_update_mute	(const GValue *value,
        gpointer data);

static GstFlowReturn gst_rtpsession_rtpsink_chain (GstPad *pad, GstBuffer *in);

static GstFlowReturn gst_rtpsession_rtcpsink_chain (GstPad *pad, GstBuffer *in);

static GstFlowReturn gst_rtpsession_datasink_chain (GstPad *pad, GstBuffer *in);

static gboolean gst_rtpsession_data_setcaps (GstPad * pad, GstCaps * caps);

static GstElementClass *parent_class = NULL;

/* this function handles the link on the data sink pad */
static gboolean
gst_rtpsession_data_setcaps (GstPad *pad, GstCaps *caps)
{
  GstRTPSession *filter;
  GstPad *otherpad;
  GstStructure *structure;
  int ret;

  GST_DEBUG("Calling setcaps and _create");
  filter = GST_RTPSESSION (gst_pad_get_parent (pad));
  g_return_val_if_fail (filter != NULL, FALSE);
  g_return_val_if_fail (GST_IS_RTPSESSION (filter),
                        FALSE);
  
  // TEMP ONLY FOR TESTING
/*  structure = gst_caps_get_structure( caps, 0 );
  ret = gst_structure_get_int( structure, "clock_rate", &filter->_clockrate );
  if (!ret) {
      return GST_PAD_LINK_REFUSED;
  }
*/
  filter->_clockrate = 100;
  // create session and init those values
  filter->_params = jrtpsession_create(filter->_sess, filter->_localportbase, 
          filter->_clockrate, filter->rtpsrcpad, filter->rtcpsrcpad, 
          filter->rtpsinkpad, filter->rtcpsinkpad);

  jrtpsession_setdestinationaddrs(filter->_sess, filter->_destaddrs);
  jrtpsession_setdefaultpt(filter->_sess, filter->_defaultpt);
  jrtpsession_setdefaultinc(filter->_sess, filter->_defaulttsinc);
  jrtpsession_setdefaultmark(filter->_sess, filter->_defaultmark);
  //otherpad = (pad == filter->srcpad ? filter->sinkpad : filter->srcpad);

  /* set caps on next or previous element's pad, and see what they
   * think. In real cases, we would (after this step) extract
   * properties from the caps such as video size or audio samplerat. */
  //return gst_pad_try_set_caps (otherpad, caps);
  gst_object_unref(filter);
  return TRUE;
}

GType
gst_gst_rtpsession_get_type (void)
{
  static GType plugin_type = 0;

  if (!plugin_type)
  {
    static const GTypeInfo plugin_info =
    {
      sizeof (GstRTPSessionClass),
      (GBaseInitFunc) gst_rtpsession_base_init,
      NULL,
      (GClassInitFunc) gst_rtpsession_class_init,
      NULL,
      NULL,
      sizeof (GstRTPSession),
      0,
      (GInstanceInitFunc) gst_rtpsession_init,
    };
    plugin_type = g_type_register_static (GST_TYPE_ELEMENT,
	                                  "GstRTPSession",
	                                  &plugin_info, 0);
  }
  return plugin_type;
}

static void
gst_rtpsession_base_init (GstRTPSessionClass *klass)
{
  static GstElementDetails plugin_details = {
    "JRTP Session",
    "Manage/RTP",
    "RTP Session management element",
    "Philippe Khalaf <burger@speedy.org>"
  };
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
	gst_static_pad_template_get (&rtp_src_factory));
  gst_element_class_add_pad_template (element_class,
	gst_static_pad_template_get (&rtcp_src_factory));
  gst_element_class_add_pad_template (element_class,
	gst_static_pad_template_get (&data_src_factory));
  gst_element_class_add_pad_template (element_class,
	gst_static_pad_template_get (&rtcp_sink_factory));
  gst_element_class_add_pad_template (element_class,
	gst_static_pad_template_get (&rtp_sink_factory));
  gst_element_class_add_pad_template (element_class,
	gst_static_pad_template_get (&data_sink_factory));
  gst_element_class_set_details (element_class, &plugin_details);
}

/* initialize the plugin's class */
static void
gst_rtpsession_class_init (GstRTPSessionClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*) klass;
  gstelement_class = (GstElementClass*) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_rtpsession_set_property;
  gobject_class->get_property = gst_rtpsession_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_LOCALPORTBASE,
          g_param_spec_uint ("localportbase", "Local Port Base", 
              "An even upd port for the rtp socket, rtcp is bound to +1",
              0, G_MAXUINT16, 0, G_PARAM_READWRITE));

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_DESTADDRS,
          g_param_spec_string ("destaddrs", "Destination Address:Port;Address:Port...", "Destination addresses and ports",
              "", G_PARAM_READWRITE));

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_DEFAULT_PT,
          g_param_spec_uint ("defaultpt", "Default Payload Type", 
              "The default payload Type",
              0, G_MAXUINT8, 0, G_PARAM_READWRITE));

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_DEFAULT_TSINC,
          g_param_spec_uint ("defaulttsinc", "Default Timestamp Increment", 
              "The default timestamp increment",
              0, G_MAXUINT32, 0, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_DEFAULT_MARK,
          g_param_spec_boolean ("defaultmark", "Default Mark", "The default mark",
              FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_SILENT,
          g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
              FALSE, G_PARAM_READWRITE));
 
  GST_DEBUG_CATEGORY_INIT (rtpsession_debug, "rtpsession", 0, "RTP Session");
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_rtpsession_init (GstRTPSession *filter)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (filter);

  filter->rtpsinkpad = gst_pad_new_from_template (
	gst_element_class_get_pad_template (klass, "rtpsink"), "rtpsink");
//  gst_pad_set_link_function (filter->rtpsinkpad, gst_pad_proxy_link);
//  gst_pad_set_getcaps_function (filter->rtpsinkpad, gst_pad_proxy_getcaps);

  filter->rtcpsinkpad = gst_pad_new_from_template (
	gst_element_class_get_pad_template (klass, "rtcpsink"), "rtcpsink");
//  gst_pad_set_link_function (filter->rtcpsinkpad, gst_pad_proxy_link);
//  gst_pad_set_getcaps_function (filter->rtcpsinkpad, gst_pad_proxy_getcaps);

  filter->datasinkpad = gst_pad_new_from_template (
	gst_element_class_get_pad_template (klass, "datasink"), "datasink");
  gst_pad_set_setcaps_function (filter->datasinkpad, gst_rtpsession_data_setcaps);
//  gst_pad_set_getcaps_function (filter->datasinkpad, gst_pad_proxy_getcaps);

  filter->rtpsrcpad = gst_pad_new_from_template (
	gst_element_class_get_pad_template (klass, "rtpsrc"), "rtpsrc");
//  gst_pad_set_link_function (filter->rtpsrcpad, gst_pad_proxy_link);
//  gst_pad_set_getcaps_function (filter->rtpsrcpad, gst_pad_proxy_getcaps);

  filter->rtcpsrcpad = gst_pad_new_from_template (
	gst_element_class_get_pad_template (klass, "rtcpsrc"), "rtcpsrc");
//  gst_pad_set_link_function (filter->rtcpsrcpad, gst_pad_proxy_link);
//  gst_pad_set_getcaps_function (filter->rtcpsrcpad, gst_pad_proxy_getcaps);

  filter->datasrcpad = gst_pad_new_from_template (
	gst_element_class_get_pad_template (klass, "datasrc"), "datasrc");
//  gst_pad_set_link_function (filter->datasrcpad, gst_pad_proxy_link);
//  gst_pad_set_getcaps_function (filter->datasrcpad, gst_pad_proxy_getcaps);

  gst_element_add_pad (GST_ELEMENT (filter), filter->rtpsinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->rtcpsinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->datasinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->rtpsrcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->rtcpsrcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->datasrcpad);

  gst_pad_set_chain_function (filter->rtpsinkpad, gst_rtpsession_rtpsink_chain);
  gst_pad_set_chain_function (filter->rtcpsinkpad, gst_rtpsession_rtcpsink_chain);
  gst_pad_set_chain_function (filter->datasinkpad, gst_rtpsession_datasink_chain);

//  gst_element_set_loopfunc (GST_ELEMENT (filter), gst_my_filter_loop);
  filter->silent = FALSE;

  // init our session
  filter->_sess = jrtpsession_init();
}

/* chain function
 * this function does the actual processing
 */

static GstFlowReturn
gst_rtpsession_rtpsink_chain (GstPad *pad, GstBuffer *in)
{
  GstRTPSession *filter;
  GstRTPBuffer *out_buf;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_BUFFER(in) != NULL, GST_FLOW_ERROR);

  filter = GST_RTPSESSION (GST_OBJECT_PARENT (pad));
  g_return_val_if_fail (GST_IS_RTPSESSION (filter), GST_FLOW_ERROR);

  // let's give the buffer to jrtplib's gsttransmitter
  if(filter->_params)
      jrtpsession_setcurrentdata(filter->_params, (GstNetBuffer *)in, 1);
  else
  {
      GST_DEBUG("_params is null, RTPsession not created yet?");
      return 0;
  }
  // poll that same packet data, this will ensure the packet
  // goes through all the jrtplib processing
  GST_DEBUG("Polling data");
  jrtpsession_polldata(filter->_sess);

  // if it's an rtp packet it will be returned by getpacket
  if ( GST_BUFFER_DATA(in) )
  {
      int size = GST_BUFFER_SIZE(GST_BUFFER(in));
      out_buf = jrtpsession_getpacket(filter->_sess, size);

      GST_DEBUG("Reading data");
      if(out_buf)
      {
          GST_DEBUG("Got packet from jrtplib size %d", GST_BUFFER_SIZE(out_buf));
          gst_pad_push( filter->datasrcpad, GST_BUFFER(out_buf) );
          GST_DEBUG("Pushed");
      } else {
          GST_DEBUG("No packet available in jrtplib!");
      }
  }
  gst_buffer_unref(in);

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_rtpsession_rtcpsink_chain (GstPad *pad, GstBuffer *in)
{
  GstRTPSession *filter;
  guint readbytes;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_BUFFER(in) != NULL, GST_FLOW_ERROR);

  filter = GST_RTPSESSION (GST_OBJECT_PARENT (pad));
  g_return_val_if_fail (GST_IS_RTPSESSION (filter), GST_FLOW_ERROR);

  // let's give the buffer to jrtplib's gsttransmitter
  if(filter->_params)
      jrtpsession_setcurrentdata(filter->_params, (GstNetBuffer *)in, 0);
  else
  {
      GST_DEBUG("_params is null, RTPsession not created yet?");
      return 0;
  }
  // poll that same packet data, this will ensure the packet
  // goes through all the jrtplib processing
  jrtpsession_polldata(filter->_sess);

  gst_buffer_unref(in);

  return GST_FLOW_OK;
}

static GstFlowReturn 
gst_rtpsession_datasink_chain (GstPad *pad, GstBuffer *buf)
{
  GstRTPSession *filter;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);
  g_return_val_if_fail (buf != NULL, GST_FLOW_ERROR);

  filter = GST_RTPSESSION (GST_OBJECT_PARENT (pad));
  g_return_val_if_fail (GST_IS_RTPSESSION (filter), GST_FLOW_ERROR);

  if (GST_BUFFER_DATA(buf))
  {
      if (GST_IS_RTPBUFFER (buf))
      {
          GstRTPBuffer * rtpbuf = GST_RTPBUFFER(buf);
          GST_DEBUG("Received a GstRTPBuffer size %d pt %d mark %d tsinc %d", 
                  GST_BUFFER_SIZE(rtpbuf), rtpbuf->pt, rtpbuf->mark, 
                  rtpbuf->timestampinc);
          jrtpsession_sendpacket(filter->_sess, GST_BUFFER_DATA(rtpbuf),
                  GST_BUFFER_SIZE(rtpbuf), rtpbuf->pt, rtpbuf->mark, 
                  rtpbuf->timestampinc);
      }
      else
      {
          GST_DEBUG ("Received GstBuffer not a GstRTPBuffer, using defaults");
          jrtpsession_sendpacket_default(filter->_sess, GST_BUFFER_DATA(buf),
                  GST_BUFFER_SIZE(buf));
      }
      GST_DEBUG ("Finished Sending rtp packet %d bytes ", GST_BUFFER_SIZE (buf));
      gst_buffer_unref(buf);

      return GST_FLOW_OK;
  }
  return GST_FLOW_ERROR;
}

static void
gst_rtpsession_set_property (GObject *object, guint prop_id,
                                  const GValue *value, GParamSpec *pspec)
{
  GstRTPSession *filter;

  g_return_if_fail (GST_IS_RTPSESSION (object));
  filter = GST_RTPSESSION (object);

  switch (prop_id)
  {
      case ARG_SILENT:
          filter->silent = g_value_get_boolean (value);
          break;
      case ARG_LOCALPORTBASE:
          filter->_localportbase = g_value_get_uint (value);
          break;
      case ARG_DESTADDRS:
          g_free(filter->_destaddrs);
          filter->_destaddrs = g_value_dup_string(value);
          //filter->_params = jrtpsession_create(filter->_sess, filter->_localportbase, 
          //        10, filter->rtpsrcpad, filter->rtcpsrcpad, 
          //        filter->rtpsinkpad, filter->rtcpsinkpad);

          //jrtpsession_setdestinationaddrs(filter->_sess, filter->_destaddrs);
          break;
      case ARG_DEFAULT_PT:
          filter->_defaultpt = g_value_get_uint (value);
          break;
      case ARG_DEFAULT_TSINC:
          filter->_defaulttsinc = g_value_get_uint (value);
          break;
      case ARG_DEFAULT_MARK:
          filter->_defaultmark = g_value_get_boolean (value);
          break;
      default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
  }
}

static void
gst_rtpsession_get_property (GObject *object, guint prop_id,
                                  GValue *value, GParamSpec *pspec)
{
  GstRTPSession *filter;

  g_return_if_fail (GST_IS_RTPSESSION (object));
  filter = GST_RTPSESSION (object);

  switch (prop_id) {
      case ARG_SILENT:
          g_value_set_boolean (value, filter->silent);
          break;
      case ARG_DESTADDRS:
          g_value_set_string (value, filter->_destaddrs);
          break;
      case ARG_LOCALPORTBASE:
          g_value_set_uint (value, filter->_localportbase);
          break;
      case ARG_DEFAULT_PT:
          g_value_set_uint (value, filter->_defaultpt);
          break;
      case ARG_DEFAULT_TSINC:
          g_value_set_uint (value, filter->_defaulttsinc);
          break;
      case ARG_DEFAULT_MARK:
          g_value_set_boolean (value, filter->_defaultmark);
          break;
      default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
  }
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and pad templates
 * register the features
 */
/*static gboolean
plugin_init (GstPlugin *plugin)
{
  return gst_element_register (plugin, "plugin",
			       GST_RANK_NONE,
			       GST_TYPE_RTPSESSION);
}*/

/* this is the structure that gst-register looks for
 * so keep the name plugin_desc, or you cannot get your plug-in registered */
/*GST_PLUGIN_DEFINE (
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "jrtp",
  "Template plugin",
  plugin_init,
  VERSION,
  "LGPL",
  "GStreamer",
  "http://gstreamer.net/"
)*/
