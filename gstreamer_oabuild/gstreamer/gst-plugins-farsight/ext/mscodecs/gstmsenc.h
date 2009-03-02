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

#ifndef __GST_MSENC_H__
#define __GST_MSENC_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#include "mscodec.h"

G_BEGIN_DECLS

#define GST_TYPE_MSENC \
  (gst_msenc_get_type ())
#define GST_MSENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_MSENC, GstMSEnc))
#define GST_MSENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_MSENC, GstMSEncClass))
#define GST_IS_MSENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_MSENC))
#define GST_IS_MSENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_MSENC))
#define GST_MSENC_CLASS_GET_PARAMS(klass) \
  ((GstMSEncClassParams *) g_type_get_qdata (G_OBJECT_CLASS_TYPE (klass), \
      GST_MSENC_PARAMS_QDATA))

typedef struct _GstMSEncClassParams GstMSEncClassParams;
typedef struct _GstMSEncClass GstMSEncClass;
typedef struct _GstMSEnc GstMSEnc;

struct _GstMSEncClassParams
{
  const MSCodec * codec;
  const gchar * mimetype;
  gint bitrate_offset;
  gboolean supports_lossrate;
};

struct _GstMSEncClass
{
  GstBaseTransformClass parent_class;
};

struct _GstMSEnc
{
  GstBaseTransform parent;

  MSEncoder * encoder;
  gdouble lossrate;
};

GType gst_msenc_get_type (void);

void gst_msenc_register (GstPlugin * plugin,
                         const gchar * element_name,
                         const gchar * mimetype,
                         gint bitrate_offset,
                         gboolean supports_lossrate,
                         const MSCodec * codec);

G_END_DECLS

#endif /* __GST_MSENC_H__ */
