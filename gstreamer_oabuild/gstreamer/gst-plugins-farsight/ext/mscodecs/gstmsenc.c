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

/*
 * TODO:
 * - Use GstAdapter and take into account what the encoder prefers as input.
 * - Actually implement transform_size().
 * - Support setting bitrate for codecs supporting it.
 */

#include "gstmsenc.h"

#include <string.h>

#ifdef G_OS_WIN32
#include <windows.h>
#else
#include <winbase.h>
#include <ldt_keeper.h>
#endif

GST_DEBUG_CATEGORY_EXTERN (mscodecs_debug);
#define GST_CAT_DEFAULT (mscodecs_debug)

#define GST_MSENC_PARAMS_QDATA g_quark_from_static_string ("msenc-params")

enum
{
  PROP_0,
  PROP_BITRATE,
  PROP_LOSSRATE
};

static GstBaseTransformClass * parent_class = NULL;

static void gst_msenc_finalize (GObject * object);
static void gst_msenc_get_property (GObject * object,
                                    guint prop_id,
                                    GValue * value,
                                    GParamSpec * pspec);
static void gst_msenc_set_property (GObject * object,
                                    guint prop_id,
                                    const GValue * value,
                                    GParamSpec * pspec);

static gboolean gst_msenc_transform_size (GstBaseTransform * base,
                                          GstPadDirection direction,
                                          GstCaps * caps,
                                          guint size,
                                          GstCaps * other_caps,
                                          guint * othersize);
static GstCaps * gst_msenc_transform_caps (GstBaseTransform * base,
                                           GstPadDirection direction,
                                           GstCaps * caps);
static GstFlowReturn gst_msenc_transform (GstBaseTransform * base,
                                          GstBuffer * inbuf,
                                          GstBuffer * outbuf);

static void
gst_msenc_base_init (GstMSEncClass * klass)
{
  GstMSEncClassParams * params = GST_MSENC_CLASS_GET_PARAMS (klass);
  GstElementClass * element_class = GST_ELEMENT_CLASS (klass);
  GstPadTemplate * sink_template, * src_template;
  GstElementDetails details;

  sink_template = gst_pad_template_new ("sink",
      GST_PAD_SINK,
      GST_PAD_ALWAYS,
      gst_caps_new_simple ("audio/x-raw-int",
          "width", G_TYPE_INT, 16,
          "depth", G_TYPE_INT, 16,
          "endianness", G_TYPE_INT, G_BYTE_ORDER,
          "signed", G_TYPE_BOOLEAN, TRUE,
          "rate", G_TYPE_INT, params->codec->rate,
          "channels", G_TYPE_INT, 1,
          NULL));
  src_template = gst_pad_template_new ("src",
      GST_PAD_SRC,
      GST_PAD_ALWAYS,
      gst_caps_new_simple (params->mimetype,
          "rate", G_TYPE_INT, params->codec->rate,
          "channels", G_TYPE_INT, 1,
          NULL));

  gst_element_class_add_pad_template (element_class, sink_template);
  gst_element_class_add_pad_template (element_class, src_template);

  details.longname = g_strdup_printf ("%s encoder", params->codec->name);
  details.klass = g_strdup_printf ("Codec/Encoder/Audio");
  details.description = g_strdup_printf ("%s encoder", params->codec->name);
  details.author = "Ole André Vadla Ravnås <oleavr@gmail.com>";
  gst_element_class_set_details (element_class, &details);
  g_free (details.longname);
  g_free (details.klass);
  g_free (details.description);
}

static void
gst_msenc_class_init (GstMSEncClass * klass)
{
  GObjectClass * gobject_class;
  GstBaseTransformClass * gstbasetransform_class;
  GstMSEncClassParams * params = GST_MSENC_CLASS_GET_PARAMS (klass);

  gobject_class = (GObjectClass *) klass;
  gstbasetransform_class = (GstBaseTransformClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gst_msenc_finalize;

  gstbasetransform_class->transform_size = gst_msenc_transform_size;
  gstbasetransform_class->transform_caps = gst_msenc_transform_caps;
  gstbasetransform_class->transform = gst_msenc_transform;

  if (params->bitrate_offset >= 0 || params->supports_lossrate)
  {
    gobject_class->get_property = gst_msenc_get_property;
    gobject_class->set_property = gst_msenc_set_property;

    if (params->bitrate_offset >= 0)
    {
      g_object_class_install_property (gobject_class, PROP_BITRATE,
          g_param_spec_uint ("bitrate", "Bitrate",
              "The bitrate used for encoding",
              0, G_MAXUINT, 0, G_PARAM_READWRITE));
    }

    if (params->supports_lossrate)
    {
      g_object_class_install_property (gobject_class, PROP_LOSSRATE,
          g_param_spec_double ("lossrate", "Lossrate",
              "The percent packet loss for controlling FEC",
              0.0, 100.0, 0.0, G_PARAM_READWRITE));
    }
  }
}

static void
gst_msenc_init (GstMSEnc * enc)
{
  GstMSEncClass * klass = (GstMSEncClass *) G_OBJECT_GET_CLASS (enc);
  const MSCodec * codec = GST_MSENC_CLASS_GET_PARAMS (klass)->codec;
  gint ret;

  enc->lossrate = 0.0;

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  ret = codec->create_encoder (&enc->encoder);
  if (ret != 0)
  {
    GST_ERROR_OBJECT (enc, "Failed to create %s encoder", codec->name);
    return;
  }

  ret = ms_encoder_init (enc->encoder);
  if (ret != 0)
  {
    GST_ERROR_OBJECT (enc, "Failed to initialize %s encoder: %d",
        codec->name, ret);
    return;
  }

  GST_DEBUG_OBJECT (enc, "%s encoder initialized successfully",
      codec->name);
}

static void
gst_msenc_finalize (GObject * object)
{
  GstMSEnc * enc = (GstMSEnc *) object;
  GstMSEncClass * klass = (GstMSEncClass *) G_OBJECT_GET_CLASS (object);
  const MSCodec * codec = GST_MSENC_CLASS_GET_PARAMS (klass)->codec;
  gint ret;

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  ret = codec->destroy_encoder (enc->encoder);
  if (ret != 0)
  {
    GST_WARNING_OBJECT (enc, "Failed to destroy %s encoder",
        codec->name);
  }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_msenc_get_property (GObject * object,
                        guint prop_id,
                        GValue * value,
                        GParamSpec * pspec)
{
  GstMSEnc * self = (GstMSEnc *) object;

  switch (prop_id)
  {
    case PROP_BITRATE:
    {
      GstMSEncClass * klass = (GstMSEncClass *) G_OBJECT_GET_CLASS (object);
      GstMSEncClassParams * params = GST_MSENC_CLASS_GET_PARAMS (klass);
      guint bitrate;

      GST_OBJECT_LOCK (self);
      bitrate =
        *((guint *) ((guint8 *) self->encoder + params->bitrate_offset));
      GST_OBJECT_UNLOCK (self);

      g_value_set_uint (value, bitrate);
      break;
    }
    case PROP_LOSSRATE:
    {
      GST_OBJECT_LOCK (self);
      g_value_set_double (value, self->lossrate);
      GST_OBJECT_UNLOCK (self);
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_msenc_set_property (GObject * object,
                        guint prop_id,
                        const GValue * value,
                        GParamSpec * pspec)
{
  GstMSEnc * self = (GstMSEnc *) object;

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  switch (prop_id)
  {
    case PROP_BITRATE:
      GST_OBJECT_LOCK (self);
      ms_encoder_set_bitrate (self->encoder, g_value_get_uint (value));
      GST_OBJECT_UNLOCK (self);
      break;
    case PROP_LOSSRATE:
      GST_OBJECT_LOCK (self);
      self->lossrate = g_value_get_double (value);
      ms_encoder_set_lossrate (self->encoder, self->lossrate);
      GST_OBJECT_UNLOCK (self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_msenc_transform_size (GstBaseTransform * base,
                          GstPadDirection direction,
                          GstCaps * caps,
                          guint size,
                          GstCaps * other_caps,
                          guint * other_size)
{
  GstMSEncClass * klass = (GstMSEncClass *) G_OBJECT_GET_CLASS (base);
  GstMSEncClassParams * params = GST_MSENC_CLASS_GET_PARAMS (klass);
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
  else if (strcmp (in_name, "audio/x-raw-int") == 0 &&
      strcmp (out_name, params->mimetype) == 0)
  {
    *other_size = size;
  }
  else
  {
    return FALSE;
  }

  return TRUE;
}

static GstCaps *
gst_msenc_transform_caps (GstBaseTransform * base,
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
gst_msenc_transform (GstBaseTransform * base,
                     GstBuffer * inbuf,
                     GstBuffer * outbuf)
{
  GstMSEnc * self = (GstMSEnc *) base;
  guint out_size = GST_BUFFER_SIZE (outbuf);
  gint ret;

#ifndef G_OS_WIN32
  Check_FS_Segment ();
#endif

  GST_DEBUG_OBJECT (self, "calling ms_encoder_encode with buffer of size %d",
      GST_BUFFER_SIZE (inbuf));

  ret = ms_encoder_encode (self->encoder,
      GST_BUFFER_DATA (inbuf), GST_BUFFER_SIZE (inbuf),
      GST_BUFFER_DATA (outbuf), &out_size);

  GST_DEBUG_OBJECT (self, "called ms_encoder_encode, ret = 0x%08x", ret);

  if (ret != 0)
  {
    GST_ELEMENT_ERROR (self, STREAM, ENCODE,
      (("ms_encoder_encode returned 0x%08x"), ret),
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
gst_msenc_register (GstPlugin * plugin,
                    const gchar * element_name,
                    const gchar * mimetype,
                    gint bitrate_offset,
                    gboolean supports_lossrate,
                    const MSCodec * codec)
{
  GTypeInfo type_info = {
    sizeof (GstMSEncClass),
    (GBaseInitFunc) gst_msenc_base_init,
    NULL,
    (GClassInitFunc) gst_msenc_class_init,
    NULL,
    NULL,
    sizeof (GstMSEncClass),
    0,
    (GInstanceInitFunc) gst_msenc_init,
  };
  gchar * type_name;
  GType type;
  GstMSEncClassParams * params;
  gboolean result;

  type_name = g_strdup_printf ("msenc_%s", element_name);

  type = g_type_register_static (GST_TYPE_BASE_TRANSFORM, type_name,
      &type_info, 0);

  params = g_new0 (GstMSEncClassParams, 1);
  params->codec = codec;
  params->mimetype = mimetype;
  params->bitrate_offset = bitrate_offset;
  params->supports_lossrate = supports_lossrate;
  g_type_set_qdata (type, GST_MSENC_PARAMS_QDATA, params);

  result = gst_element_register (plugin, type_name, GST_RANK_NONE, type);
  if (!result)
  {
    GST_ERROR_OBJECT (plugin, "failed to register element %s", type_name);
  }

  g_free (type_name);
}
