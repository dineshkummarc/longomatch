/* GStreamer
 * Copyright (C) <2005> Philippe Khalaf <burger@speedy.org> 
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
#include <gst/gst.h>
#include "gstjasperdec.h"
#include <gst/video/video.h>

static GstElementDetails gst_jasperdec_details = {
  "JPEG2000 Decoder",
  "Codec/Decoder/Image",
  "Decode a JPEG2000 image",
  "Philippe Khalaf <burger@speedy.org>",
};

GST_DEBUG_CATEGORY (jasperdec_debug);
#define GST_CAT_DEFAULT jasperdec_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
};

static GstStaticPadTemplate jasperdec_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static GstStaticPadTemplate jasperdec_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("image/jp2, "
        "width = (int) [ 16, 4096 ], "
        "height = (int) [ 16, 4096 ], " "framerate = (fraction) [ 0.0, MAX ]")
    );

GST_BOILERPLATE (GstJasperDec, gst_jasperdec, GstElement, GST_TYPE_ELEMENT);

jas_image_t *
gst_jasperdec_make_image_from_raw_rgb(GstJasperDec *jasperdec, GstBuffer* buf);
static void gst_jasperdec_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_jasperdec_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_jasperdec_chain (GstPad * pad, GstBuffer * data);

static void
gst_jasperdec_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template
      (element_class, gst_static_pad_template_get (&jasperdec_sink_template));
  gst_element_class_add_pad_template
      (element_class, gst_static_pad_template_get (&jasperdec_src_template));
  gst_element_class_set_details (element_class, &gst_jasperdec_details);
}

static void
gst_jasperdec_class_init (GstJasperDecClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->get_property = gst_jasperdec_get_property;
  gobject_class->set_property = gst_jasperdec_set_property;

  GST_DEBUG_CATEGORY_INIT (jasperdec_debug, "jasperdec", 0, "Jpeg2000 image decoder");
}

static gboolean
gst_jasperdec_setcaps (GstPad * pad, GstCaps * caps)
{
  GstJasperDec *jasperdec;
  const GValue *fps;
  GstStructure *structure;
  GstCaps *pcaps;
  gboolean ret = TRUE;
  GstPad *opeer;

  jasperdec = GST_JASPERDEC (gst_pad_get_parent (pad));

  structure = gst_caps_get_structure (caps, 0);
  gst_structure_get_int (structure, "width", &jasperdec->width);
  gst_structure_get_int (structure, "height", &jasperdec->height);
  fps = gst_structure_get_value (structure, "framerate");
  gst_structure_get_int (structure, "bpp", &jasperdec->bpp);

  opeer = gst_pad_get_peer (jasperdec->srcpad);
  if (opeer) {
    pcaps = gst_caps_new_simple (GST_VIDEO_CAPS_RGB);
    structure = gst_caps_get_structure (pcaps, 0);
    gst_structure_set_value (structure, "framerate", fps);
    gst_structure_set_value (structure, "width", jasperdec->width);
    gst_structure_set_value (structure, "height", jasperdec->height);

    if (gst_pad_accept_caps (opeer, pcaps)) {
      gst_pad_set_caps (jasperdec->srcpad, pcaps);
    } else
      ret = FALSE;
    gst_caps_unref (pcaps);
    gst_object_unref (opeer);
  }
  gst_object_unref(jasperdec);
  return ret;
}

static void
gst_jasperdec_init (GstJasperDec * jasperdec, GstJasperDecClass * g_class)
{
  /* sinkpad */
  jasperdec->sinkpad = gst_pad_new_from_template
      (gst_static_pad_template_get (&jasperdec_sink_template), "sink");
  gst_pad_set_chain_function (jasperdec->sinkpad, gst_jasperdec_chain);
  gst_pad_set_setcaps_function (jasperdec->sinkpad, gst_jasperdec_setcaps);
  gst_element_add_pad (GST_ELEMENT (jasperdec), jasperdec->sinkpad);

  /* srcpad */
  jasperdec->srcpad = gst_pad_new_from_template
      (gst_static_pad_template_get (&jasperdec_src_template), "src");
  gst_element_add_pad (GST_ELEMENT (jasperdec), jasperdec->srcpad);

  /* init jasper */
  if (jas_init())
  {
      GST_DEBUG_OBJECT(jasperdec, "Failed to initialise libjasper");
  }
}

static GstFlowReturn
gst_jasperdec_chain (GstPad * pad, GstBuffer * buf)
{
  GstJasperDec *jasperdec;
  GstFlowReturn ret = GST_FLOW_OK;

  /* this will hold our output image */
  jas_image_t *image;
  /* this will hold our input buffer */
  jas_stream_t *in;

  /* our encoded image */
  GstBuffer *out_buf;

  jasperdec = GST_JASPERDEC (gst_pad_get_parent (pad));

  if (!(in = jas_stream_memopen((char *)GST_BUFFER_DATA(buf), GST_BUFFER_SIZE(buf))))
  {
      GST_DEBUG_OBJECT(jasperdec, "Could not read input buffer");
      ret = GST_FLOW_ERROR;
      goto done;
  }

  /*
  if (!(image = gst_jasperdec_make_image_from_raw_rgb(jasperdec, buf)))
  {
      GST_DEBUG_OBJECT(jasperdec, "Could not load image");
      ret = GST_FLOW_ERROR;
      goto done;
  }*/

  if (!(image = jas_image_decode (in, jas_image_strtofmt("jp2"), 0)))
  {
      GST_DEBUG_OBJECT(jasperdec, "Could not decode image");
      ret = GST_FLOW_ERROR;
      goto done;
  }

  /* out now contains the encoded image and the length */
  jas_stream_memobj_t* obj = (jas_stream_memobj_t*) out->obj_;
  out_buf = gst_buffer_new_and_alloc (obj->len_);
  GST_BUFFER_DATA (out_buf) = obj->buf_;

  jas_stream_close(out);
  jas_image_destroy(image);

  if ((ret = gst_pad_push (jasperdec->srcpad, out_buf)) != GST_FLOW_OK)
    goto done;

done:

  gst_buffer_unref (buf);
  gst_object_unref (jasperdec);
  return ret;
}

GstBuffer*
gst_jasperdec_make_raw_rgb_from_image(GstJasperDec *jasperdec, jas_image_t* image)
{
    GstBuffer *buf;
    // prepare the component parameters
    jas_image_cmptparm_t cmptparms[3];

    gint i;
    for (i = 0; i < 3; ++i) {
        cmptparms[i].tlx = 0;
        cmptparms[i].tly = 0;

        cmptparms[i].hstep = 1;
        cmptparms[i].vstep = 1;
        cmptparms[i].width = jasperdec->width;
        cmptparms[i].height = jasperdec->height;

        /* 8 x 3 components = 24 bits */
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = false;
    }

    if (!(image = jas_image_create(3, cmptparms, JAS_CLRSPC_UNKNOWN)))
    {
        GST_DEBUG_OBJECT(jasperdec, "Could not create the image");
        return FALSE;
    }

    guint8 *data = GST_BUFFER_DATA(buf);

    jas_matrix_t* m;
    if (!(m = jas_matrix_create(jasperdec->height, jasperdec->width)))
    {
        GST_DEBUG_OBJECT(jasperdec, "Could not create matrix");
        return FALSE;
    }

    jas_image_setclrspc(image, JAS_CLRSPC_SRGB);
    jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_RGB_R);
    jas_image_setcmpttype(image, 1, JAS_IMAGE_CT_RGB_G);
    jas_image_setcmpttype(image, 2, JAS_IMAGE_CT_RGB_B);

    gint xpos, ypos;
    gint height = jasperdec->height;
    gint width = jasperdec->width;

    /* R */
    for(ypos = 0; ypos < height; ypos++)
        for(xpos = 0; xpos < width; xpos++)
            jas_matrix_set(m, ypos, xpos, (int)data+(3*((ypos*width)+xpos)));
    jas_image_writecmpt(image, 0, 0, 0, width, height, m);

    /* G */
    for(ypos = 0; ypos < height; ypos++)
        for(xpos = 0; xpos < width; xpos++)
            jas_matrix_set(m, ypos, xpos, (int)data+(3*((ypos*width)+xpos))+1);
    jas_image_writecmpt(image, 1, 0, 0, width, height, m);

    /* B */
    for(ypos = 0; ypos < height; ypos++)
        for(xpos = 0; xpos < width; xpos++)
            jas_matrix_set(m, ypos, xpos, (int)data+(3*((ypos*width)+xpos))+2);
    jas_image_writecmpt(image, 2, 0, 0, width, height, m);

    jas_matrix_destroy(m);

    return image;
}

static void
gst_jasperdec_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstJasperDec *jasperdec;

  jasperdec = GST_JASPERDEC (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_jasperdec_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstJasperDec *jasperdec;

  jasperdec = GST_JASPERDEC (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

gboolean
gst_jasperdec_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "jasperdec",
      GST_RANK_NONE, GST_TYPE_JASPERDEC);
}
