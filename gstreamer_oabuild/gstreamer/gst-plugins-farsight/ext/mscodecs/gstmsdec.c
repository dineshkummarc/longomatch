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

#include "gstmsdec.h"

#include <string.h>

GST_DEBUG_CATEGORY_EXTERN (mscodecs_debug);
#define GST_CAT_DEFAULT (mscodecs_debug)

#define GST_MSDEC_PARAMS_QDATA g_quark_from_static_string ("msdec-params")

static GstBaseTransformClass * parent_class = NULL;

static void gst_msdec_finalize (GObject * object);

static gboolean gst_msdec_transform_size (GstBaseTransform * base,
                                          GstPadDirection direction,
                                          GstCaps * caps,
                                          guint size,
                                          GstCaps * other_caps,
                                          guint * othersize);
static GstCaps * gst_msdec_transform_caps (GstBaseTransform * base,
                                           GstPadDirection direction,
                                           GstCaps * caps);
static GstFlowReturn gst_msdec_transform (GstBaseTransform * base,
                                          GstBuffer * inbuf,
                                          GstBuffer * outbuf);

static void
gst_msdec_base_init (GstMSDecClass * klass)
{
  GstMSDecClassParams * params = GST_MSDEC_CLASS_GET_PARAMS (klass);
  GstElementClass * element_class = GST_ELEMENT_CLASS (klass);
  GstPadTemplate * sink_template, * src_template;
  GstElementDetails details;

  sink_template = gst_pad_template_new ("sink",
      GST_PAD_SINK,
      GST_PAD_ALWAYS,
      gst_caps_new_simple (params->mimetype,
          "rate", G_TYPE_INT, params->codec->rate,
          "channels", G_TYPE_INT, 1,
          NULL));
  src_template = gst_pad_template_new ("src",
      GST_PAD_SRC,
      GST_PAD_ALWAYS,
      gst_caps_new_simple ("audio/x-raw-int",
          "width", G_TYPE_INT, 16,
          "depth", G_TYPE_INT, 16,
          "endianness", G_TYPE_INT, G_BYTE_ORDER,
          "signed", G_TYPE_BOOLEAN, TRUE,
          "rate", G_TYPE_INT, params->codec->rate,
          "channels", G_TYPE_INT, 1,
          NULL));

  gst_element_class_add_pad_template (element_class, sink_template);
  gst_element_class_add_pad_template (element_class, src_template);

  details.longname = g_strdup_printf ("%s decoder", params->codec->name);
  details.klass = g_strdup_printf ("Codec/Decoder/Audio");
  details.description = g_strdup_printf ("%s decoder", params->codec->name);
  details.author = "Ole André Vadla Ravnås <oleavr@gmail.com>";
  gst_element_class_set_details (element_class, &details);
  g_free (details.longname);
  g_free (details.klass);
  g_free (details.description);
}

static void
gst_msdec_class_init (GstMSDecClass * klass)
{
  GObjectClass * gobject_class;
  GstBaseTransformClass * gstbasetransform_class;

  gobject_class = (GObjectClass *) klass;
  gstbasetransform_class = (GstBaseTransformClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gst_msdec_finalize;

  gstbasetransform_class->transform_size = gst_msdec_transform_size;
  gstbasetransform_class->transform_caps = gst_msdec_transform_caps;
  gstbasetransform_class->transform = gst_msdec_transform;
}

static void
gst_msdec_init (GstMSDec * dec)
{
  GstMSDecClass * klass = (GstMSDecClass *) G_OBJECT_GET_CLASS (dec);
  const MSCodec * codec = GST_MSDEC_CLASS_GET_PARAMS (klass)->codec;
  gint ret;

  ret = codec->create_decoder (&dec->decoder);
  if (ret != 0)
  {
    GST_ERROR_OBJECT (dec, "Failed to create %s decoder", codec->name);
    return;
  }

  ret = ms_decoder_init (dec->decoder);
  if (ret != 0)
  {
    GST_ERROR_OBJECT (dec, "Failed to initialize %s decoder: %d", codec->name,
        ret);
    return;
  }

  GST_DEBUG_OBJECT (dec, "%s decoder initialized successfully", codec->name);
}

static void
gst_msdec_finalize (GObject * object)
{
  GstMSDec * dec = (GstMSDec *) object;
  GstMSDecClass * klass = (GstMSDecClass *) G_OBJECT_GET_CLASS (object);
  const MSCodec * codec = GST_MSDEC_CLASS_GET_PARAMS (klass)->codec;
  gint ret;

  ret = codec->destroy_decoder (dec->decoder);
  if (ret != 0)
  {
    GST_WARNING_OBJECT (dec, "Failed to destroy %s decoder", codec->name);
  }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_msdec_transform_size (GstBaseTransform * base,
                          GstPadDirection direction,
                          GstCaps * caps,
                          guint size,
                          GstCaps * other_caps,
                          guint * other_size)
{
  GstMSDecClass * klass = (GstMSDecClass *) G_OBJECT_GET_CLASS (base);
  GstMSDecClassParams * params = GST_MSDEC_CLASS_GET_PARAMS (klass);
  GstStructure * structure;
  const gchar * in_name, * out_name;

  if (caps == NULL || other_caps == NULL)
    return FALSE;

  structure = gst_caps_get_structure (caps, 0);
  in_name = gst_structure_get_name (structure);
  structure = gst_caps_get_structure (other_caps, 0);
  out_name = gst_structure_get_name (structure);

  if (in_name == NULL || out_name == NULL)
  {
    return FALSE;
  }
  else if (strcmp (in_name, params->mimetype) == 0 &&
    strcmp (out_name, "audio/x-raw-int") == 0)
  {
    *other_size = 1000; /* FIXME: not good, but enough for a 60 ms chunk */
  }
  else
  {
    return FALSE;
  }

  return TRUE;
}

static GstCaps *
gst_msdec_transform_caps (GstBaseTransform * base,
                          GstPadDirection direction,
                          GstCaps * caps)
{
  GstElementClass * element_class = GST_ELEMENT_CLASS (
      G_OBJECT_GET_CLASS (base));
  GstPadTemplate * tpl;

  if (direction == GST_PAD_SRC)
    tpl = gst_element_class_get_pad_template (element_class, "sink");
  else
    tpl = gst_element_class_get_pad_template (element_class, "src");

  return gst_caps_ref (gst_pad_template_get_caps (tpl));
}

static GstFlowReturn
gst_msdec_transform (GstBaseTransform * base,
                     GstBuffer * inbuf,
                     GstBuffer * outbuf)
{
  GstMSDec * self = (GstMSDec *) base;
  guint out_size = GST_BUFFER_SIZE (outbuf);
  gint ret;

  GST_DEBUG_OBJECT (self, "calling ms_decoder_decode with buffer of size %d",
      GST_BUFFER_SIZE (inbuf));

  ret = ms_decoder_decode (self->decoder,
      GST_BUFFER_DATA (inbuf), GST_BUFFER_SIZE (inbuf),
      GST_BUFFER_DATA (outbuf), &out_size);

  GST_DEBUG_OBJECT (self, "called ms_decoder_decode, ret = 0x%08x", ret);

  if (ret != 0)
  {
    GST_ELEMENT_ERROR (self, STREAM, ENCODE,
      (("ms_decoder_decode returned 0x%08x"), ret),
      NULL);
    return GST_FLOW_ERROR;
  }

  GST_BUFFER_SIZE (outbuf) = out_size;
  GST_BUFFER_OFFSET (outbuf) = GST_BUFFER_OFFSET (inbuf);
  GST_BUFFER_TIMESTAMP (outbuf) = GST_BUFFER_TIMESTAMP (inbuf);
  GST_BUFFER_DURATION (outbuf) = GST_BUFFER_DURATION (inbuf);

  return GST_FLOW_OK;
}

void
gst_msdec_register (GstPlugin * plugin,
                    const gchar * element_name,
                    const gchar * mimetype,
                    const MSCodec * codec)
{
  GTypeInfo type_info = {
    sizeof (GstMSDecClass),
    (GBaseInitFunc) gst_msdec_base_init,
    NULL,
    (GClassInitFunc) gst_msdec_class_init,
    NULL,
    NULL,
    sizeof (GstMSDecClass),
    0,
    (GInstanceInitFunc) gst_msdec_init,
  };
  gchar * type_name;
  GType type;
  GstMSDecClassParams * params;
  gboolean result;

  type_name = g_strdup_printf ("msdec_%s", element_name);

  type = g_type_register_static (GST_TYPE_BASE_TRANSFORM, type_name,
      &type_info, 0);

  params = g_new0 (GstMSDecClassParams, 1);
  params->codec = codec;
  params->mimetype = mimetype;
  g_type_set_qdata (type, GST_MSDEC_PARAMS_QDATA, params);

  result = gst_element_register (plugin, type_name, GST_RANK_NONE, type);
  if (!result)
  {
    GST_ERROR_OBJECT (plugin, "failed to register element %s", type_name);
  }

  g_free (type_name);
}
