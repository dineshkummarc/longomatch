/* GStreamer
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org> 
 * Copyright (C) 1999-2005 Wang, Zhanglei <filamoon@hotmail.com> 
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

#include <string.h>

#include <gst/gst.h>

#include <gstr263enc.h>
#include <gst/rtp/gstrtpbuffer.h>

GST_DEBUG_CATEGORY (r263enc_debug);
#define GST_CAT_DEFAULT (r263enc_debug)

/* FIXME: this should be defined in an appropriate header file. */
#ifndef GST_RTP_PAYLOAD_H263
#define GST_RTP_PAYLOAD_H263 34
#define GST_RTP_H263_CLOCKRATE 90000
#endif /* GST_RTP_PAYLOAD_H263 */

/* Filter signals and args */
enum {
    /* FILL ME */
    LAST_SIGNAL
};

enum {
    ARG_0,
    ARG_SILENT,
    ARG_RTP,
};

static GstStaticPadTemplate sink_factory =
GST_STATIC_PAD_TEMPLATE (
        "sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("video/x-raw-yuv, "
            "format = (fourcc) I420, "
            "width = (int) [16, 4096], "
            "height = (int) [12, 4096], "
            "framerate = (double) [1, 30]" )
        );

static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
        "src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("video/x-h263; application/x-rtp")
//        GST_STATIC_CAPS("ANY")
        );

static void	gst_r263enc_class_init (GstR263EncClass *klass);
static void	gst_r263enc_base_init (GstR263EncClass *klass);
static void	gst_r263enc_init (GstR263Enc *filter);

/*
static GstStateChangeReturn
gst_r263enc_change_state (GstElement *element, GstStateChange transition);
*/

static void gst_r263enc_finalize (GObject * object);
static void	gst_r263enc_set_property (GObject *object, guint prop_id,
        const GValue *value,
        GParamSpec *pspec);
static void	gst_r263enc_get_property (GObject *object, guint prop_id,
        GValue *value,
        GParamSpec *pspec);

static GstFlowReturn gst_r263enc_chain (GstPad *pad, GstBuffer *in);

static gboolean gst_r263enc_setcaps (GstPad * pad, GstCaps * caps);
static GstCaps *gst_r263enc_src_getcaps (GstPad * pad);

static GstElementClass *parent_class = NULL;

/**********************************************
 *          libr263 functions begin here       *
 **********************************************/
static void uninit_encoder(GstR263Enc *enc)
{
    if (!enc->enc_inited) return;
    CloseCompress(&enc->cparams);  /* Use standard compression parameters */
    g_free(enc->src_int_frame);
    g_free(enc->EncodeBlocks);
    enc->enc_inited = FALSE;
    enc->is_qcif = FALSE;
}

static gboolean init_encoder(GstR263Enc *enc)
{
    if (enc->enc_inited)
    {
        uninit_encoder(enc);
    }

    if (enc->width==QCIF_YWIDTH && enc->height==QCIF_YHEIGHT)
    {
        enc->is_qcif = TRUE;
        enc->src_int_frame = g_malloc(sizeof(struct qcif));
        enc->cparams.format = CPARAM_QCIF;
    }
    else if (enc->width==CIF_YWIDTH && enc->height==CIF_YHEIGHT)
    {
        enc->src_int_frame = g_malloc(sizeof(struct cif));
        enc->cparams.format = CPARAM_CIF;
    }
    else if (enc->width==SQCIF_YWIDTH && enc->height==SQCIF_YHEIGHT)
    {
        enc->src_int_frame = g_malloc(sizeof(int)*SQCIF_YWIDTH*SQCIF_YHEIGHT*3/2);
        enc->cparams.format = CPARAM_SQCIF;
    }
    else if (enc->width==CIF4_YWIDTH && enc->height==CIF4_YHEIGHT)
    {
        enc->src_int_frame = g_malloc(sizeof(int)*CIF4_YWIDTH*CIF4_YHEIGHT*3/2);
        enc->cparams.format = CPARAM_4CIF;
    }
    else if (enc->width==CIF16_YWIDTH && enc->height==CIF16_YHEIGHT)
    {
        enc->src_int_frame = g_malloc(sizeof(int)*CIF16_YWIDTH*CIF16_YHEIGHT*3/2);
        enc->cparams.format = CPARAM_16CIF;
    }
    else
    {
        GST_ERROR("UNSUPORTED SIZE/WIDTH! %d %d quiting", enc->width, enc->height);
        return FALSE;
        //enc->cparams.format = CPARAM_OTHER;
        //enc->cparams.pels = enc->width;
        //enc->cparams.lines = enc->height;
        //enc->src_int_frame = g_malloc(sizeof(int)*enc->width*enc->height*3/2);
    }

    enc->EncodeBlocks = g_malloc0(sizeof(gint)*enc->width*enc->height/16/16);
    InitCompress(&enc->cparams);  /* Use standard compression parameters */
    enc->cparams.Q_intra = 8;
    enc->cparams.half_pixel_searchwindow = 4;
    enc->cparams.search_method = CPARAM_LOGARITHMIC;
    enc->cparams.Q_inter = 8;
    enc->cparams.advanced_method = CPARAM_ADVANCED;
    enc->cparams.EncodeThisBlock = (int *)enc->EncodeBlocks;
    enc->gop_length = 32;
    enc->cparams.rtp_threshold = enc->rtp_support? 850 : 0;
    enc->cparams.data = (unsigned int *)enc->src_int_frame;
    stream_ptr = 0;
    enc->frame_no = 0;

    enc->enc_inited = TRUE;

    return TRUE;
}


/**********************************************
 *          libr263 functions end here         *
 **********************************************/

GType
gst_r263enc_get_type (void)
{
    static GType plugin_type = 0;

    if (!plugin_type)
    {
        static const GTypeInfo plugin_info =
        {
            sizeof (GstR263EncClass),
            (GBaseInitFunc) gst_r263enc_base_init,
            NULL,
            (GClassInitFunc) gst_r263enc_class_init,
            NULL,
            NULL,
            sizeof (GstR263Enc),
            0,
            (GInstanceInitFunc) gst_r263enc_init,
        };
        plugin_type = g_type_register_static (GST_TYPE_ELEMENT,
                "GstR263Enc",
                &plugin_info, 0);
    }
    return plugin_type;
}

static gboolean gst_r263enc_setcaps (GstPad * pad, GstCaps * caps)
{
    GstStructure *structure = gst_caps_get_structure (caps, 0);
    GstR263Enc *filter;

    filter = GST_R263ENC (gst_pad_get_parent (pad));

    GST_DEBUG("calling setcaps");

    gboolean ret;

#if 0
    struct fourcc_list_struct *fourcc;
    fourcc = (struct fourcc_list_struct *)paintinfo_find_by_structure (structure);
    if (!fourcc) {
        g_critical ("videotestsrc format not found\n");
        gst_object_unref(filter);
        return GST_PAD_LINK_REFUSED;
    }
#endif

    if (pad == filter->sinkpad)
    {
        ret = gst_structure_get_int (structure, "width", &filter->width);
        ret &= gst_structure_get_int (structure, "height", &filter->height);
        ret &= gst_structure_get_double (structure, "framerate", &filter->framerate);

        if (!ret) {
            gst_object_unref(filter);
            return FALSE;
        }
        //if (filter->width % 16 || filter->height % 16)
        //  return FALSE;

        GST_DEBUG ("got size %d x %d framerate %d, initing compression lib",
                filter->width, filter->height, filter->framerate);
    }

    if (!init_encoder(filter))
    {
        gst_object_unref(filter);
        return FALSE;
    }
    gst_object_unref(filter);
    return TRUE; 
}

GstCaps *
gst_r263enc_src_getcaps (GstPad * pad)
{
    GstR263Enc *filter;

    filter = GST_R263ENC (gst_pad_get_parent (pad));
    g_return_val_if_fail (filter != NULL, NULL);

    if (filter->rtp_support)
    {
        gst_object_unref(filter);
        return gst_caps_new_simple ("application/x-rtp",
                "payload", G_TYPE_INT, GST_RTP_PAYLOAD_H263,
                "media", G_TYPE_STRING, "video",
                "clock_rate", G_TYPE_INT, GST_RTP_H263_CLOCKRATE,
                NULL);
    }
    else
    {
        gst_object_unref(filter);
        return gst_caps_new_simple ("video/x-h263", NULL);
    }
}

static void
gst_r263enc_base_init (GstR263EncClass *klass)
{
    static GstElementDetails plugin_details = {
        "h.263 video encoder",
        "Codec/Encoder/Video",
        "Using libr263 by Roalt Aalmoes, http://www.huygens.org",
        "Zhanglei Wang <filamoon@hotmail.com>, Philippe Khalaf <burger@speedy.org>"
    };
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

    gst_element_class_add_pad_template (element_class,
            gst_static_pad_template_get (&sink_factory));
    gst_element_class_add_pad_template (element_class,
            gst_static_pad_template_get (&src_factory));
    gst_element_class_set_details (element_class, &plugin_details);
}

/* initialize the plugin's class */
static void
gst_r263enc_class_init (GstR263EncClass *klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    gobject_class = (GObjectClass*) klass;
    gstelement_class = (GstElementClass*) klass;

    parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

    gobject_class->set_property = gst_r263enc_set_property;
    gobject_class->get_property = gst_r263enc_get_property;

    g_object_class_install_property (gobject_class, ARG_SILENT,
            g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
                FALSE, G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class, ARG_RTP,
            g_param_spec_boolean ("rtp-support", "rtp support", "Turn on RTP support ?",
                FALSE, G_PARAM_READWRITE));

    /* gstelement_class->change_state = gst_r263enc_change_state; */
    gobject_class->finalize = gst_r263enc_finalize;

    GST_DEBUG_CATEGORY_INIT (r263enc_debug, "r263enc", 0, "R263 Encoder and RTP payloader");
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_r263enc_init (GstR263Enc *filter)
{
    GstElementClass *klass = GST_ELEMENT_GET_CLASS (filter);

    filter->sinkpad = gst_pad_new_from_template (
            gst_element_class_get_pad_template (klass, "sink"), "sink");
    gst_pad_set_setcaps_function (filter->sinkpad, gst_r263enc_setcaps);
//    gst_pad_set_getcaps_function (filter->sinkpad, gst_r263enc_sink_getcaps);

    filter->srcpad = gst_pad_new_from_template (
            gst_element_class_get_pad_template (klass, "src"), "src");
    gst_pad_set_setcaps_function (filter->srcpad, gst_r263enc_setcaps);
    gst_pad_set_getcaps_function (filter->srcpad, gst_r263enc_src_getcaps);

    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

    gst_pad_set_chain_function (filter->sinkpad, gst_r263enc_chain);

    filter->silent = FALSE;
    filter->rtp_support = FALSE;
    filter->enc_inited = FALSE;
    filter->is_qcif = FALSE;
}

static void gst_r263enc_finalize (GObject * object)
{
    GstR263Enc *filter;

    g_return_if_fail (GST_IS_R263ENC (object));
    filter = GST_R263ENC (object);

    uninit_encoder (filter);

    if (G_OBJECT_CLASS (parent_class)->finalize)
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_r263enc_chain (GstPad *pad, GstBuffer *in)
{
    GstR263Enc *r;
    GstBuffer *out_buf;
    GstBuffer *rtp_buf = NULL;
    gint offset;
    gint header_int;
    gchar * header = (gchar*) &header_int;
    gint i;
    static gboolean first = TRUE;

    g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);
    g_return_val_if_fail (GST_BUFFER_DATA(in) != NULL, GST_FLOW_ERROR);

    r = GST_R263ENC (GST_OBJECT_PARENT (pad));
    g_return_val_if_fail (GST_IS_R263ENC (r), GST_FLOW_ERROR);

    if (!r->enc_inited)
    {
        GST_DEBUG("Encoder not initiated! exiting chain");
        return GST_FLOW_ERROR;
    }

    if (r->is_qcif)
    {
        //ReadQCIFBuffer((const unsigned char*)GST_BUFFER_DATA(in), r->src_int_frame);
        GST_DEBUG("QCIF dosent work yet!");
        return GST_FLOW_ERROR;
    }
    else
    {
        ReadBuffer((const unsigned char*)GST_BUFFER_DATA(in), r->src_int_frame, r->width, r->height);
    }

    out_buf = gst_buffer_new_and_alloc(1024*40);
    stream_buffer = (char*)GST_BUFFER_DATA(out_buf);
    stream_ptr = r->rtp_support? 4 : 0;

    if ( ( (r->frame_no++) % r->gop_length) == 0 )
    {
        r->cparams.inter = CPARAM_INTRA;
        CompressToH263(&r->cparams, &r->bits);
    }
    else
    {
        r->cparams.inter = CPARAM_INTER;
        if(r->cparams.advanced_method)
            FindMotion(&r->cparams,2,2);
        CompressToH263(&r->cparams, &r->bits);
    }

    GST_BUFFER_SIZE(out_buf) = stream_ptr;
    GST_DEBUG("Got buffer, QCIF is %d size is %d", r->is_qcif, GST_BUFFER_SIZE(out_buf));
    /* payload header mode A */
    if (r->rtp_support)
    {
        header[0] = 0;
        /* header[0] |= (sbit << 3 | ebit); */ /* always byte-aligned */
        //1: SRC(3) I U S A R
        header[1] = 0x40 | ( (r->cparams.inter == CPARAM_INTER) << 4 );   //SRC=010, QCIF
        //2: R R R DBQ(2) TRB(3)
        header[2] = 0;    //R must be 0, we don't use B frames, so DBQ=00, TRB=000
        //3: TR(8)
        header[3] = (char) r->frame_no; //(((int64_t)s->picture_number * 30 * s->avctx->frame_rate_base) /

        GST_DEBUG("Got frame size is %d, rtp_count is : %d, rtp_offsetlast is : %d, frame : %d\n", 
        		stream_ptr, r->bits.rtp_count, r->bits.rtp_offset[r->bits.rtp_count-1],
        		r->frame_no);

        offset = 0;
        //gst_util_dump_mem (GST_BUFFER_DATA(out_buf), 32);
        //for(i=1; i<r->bits.rtp_count-1; i++)
        for(i=1; i<r->bits.rtp_count; i++)
        {
            *(int*)(stream_buffer+offset) = header_int;
            int size = r->bits.rtp_offset[i] - r->bits.rtp_offset[i-1];
            rtp_buf = gst_rtp_buffer_new_allocate (size, 0, 0);
            memcpy ((guint8 *)gst_rtp_buffer_get_payload (rtp_buf), GST_BUFFER_DATA(out_buf)+offset, size);
            if (first)
            {
                GST_DEBUG("Setting caps on first buffer");
                gst_buffer_set_caps (rtp_buf, gst_pad_get_caps(r->srcpad));
                first = FALSE;
            }
            gst_rtp_buffer_set_payload_type (rtp_buf, GST_RTP_PAYLOAD_H263);
            gst_rtp_buffer_set_marker (rtp_buf, FALSE);
            // clock rate is 90 kHz for this payload
            //rtp_buf->timestampinc = 90000 / r->framerate;
            GstClockTime timestamp = GST_BUFFER_TIMESTAMP (in);
            guint32 ts = 0;
            if (GST_CLOCK_TIME_IS_VALID (timestamp))
            {
                ts = timestamp * GST_RTP_H263_CLOCKRATE / GST_SECOND;
                gst_rtp_buffer_set_timestamp (rtp_buf, ts);
            }
            GST_DEBUG("Calculated timestamp is %u", ts);

            GST_DEBUG("%d : One part is %d, from %d %d %d", i, size, offset, r->bits.rtp_offset[i-1], r->bits.rtp_offset[i]);

            if(i+1 >= r->bits.rtp_count)
            {
                gst_rtp_buffer_set_marker (rtp_buf, TRUE);
            }

            gst_pad_push (r->srcpad, GST_BUFFER(rtp_buf));

            offset = r->bits.rtp_offset[i];
        }
        gst_buffer_unref(out_buf);
    }
    else
    {
        if (first)
        {
            GST_DEBUG("Setting caps on first buffer");
            gst_buffer_set_caps (out_buf, gst_pad_get_caps(r->srcpad));
            first = FALSE;
        }
        //gst_util_dump_mem (GST_BUFFER_DATA(out_buf), 24);
        gst_pad_push (r->srcpad, out_buf);
    }
    gst_buffer_unref(in);
    return GST_FLOW_OK;
}

/*
static GstStateChangeReturn
gst_r263enc_change_state (GstElement *element, GstStateChange transition);
{
    GST_DEBUG("custom change state");
    GstR263Enc *filter;

    g_return_if_fail (GST_IS_R263ENC (element));
    filter = GST_R263ENC (element);

    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
            break;
        case GST_STATE_CHANGE_READY_TO_PAUSED:
            init_encoder(filter);
            break;
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
            init_encoder(filter);
            break;
        case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
            uninit_encoder(filter);
            break;
        case GST_STATE_CHANGE_PAUSED_TO_READY:
            break;
        case GST_STATE_CHANGE_READY_TO_NULL:
            break;
    }

    return parent_class->change_state (element, transition);
}*/

static void
gst_r263enc_set_property (GObject *object, guint prop_id,
        const GValue *value, GParamSpec *pspec)
{
    GstR263Enc *filter;

    g_return_if_fail (GST_IS_R263ENC (object));
    filter = GST_R263ENC (object);

    switch (prop_id)
    {
        case ARG_SILENT:
            filter->silent = g_value_get_boolean (value);
            break;
        case ARG_RTP:
            filter->rtp_support = g_value_get_boolean (value);
           break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gst_r263enc_get_property (GObject *object, guint prop_id,
        GValue *value, GParamSpec *pspec)
{
    GstR263Enc *filter;

    g_return_if_fail (GST_IS_R263ENC (object));
    filter = GST_R263ENC (object);

    switch (prop_id) {
        case ARG_SILENT:
            g_value_set_boolean (value, filter->silent);
            break;
        case ARG_RTP:
            g_value_set_boolean (value, filter->rtp_support);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}
