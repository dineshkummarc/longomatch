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

#ifndef __GST_MSDEC_H__
#define __GST_MSDEC_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#include "mscodec.h"

G_BEGIN_DECLS

#define GST_TYPE_MSDEC \
  (gst_msdec_get_type ())
#define GST_MSDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_MSDEC, GstMSDec))
#define GST_MSDEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_MSDEC, GstMSDecClass))
#define GST_IS_MSDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_MSDEC))
#define GST_IS_MSDEC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_MSDEC))
#define GST_MSDEC_CLASS_GET_PARAMS(klass) \
  ((GstMSDecClassParams *) g_type_get_qdata (G_OBJECT_CLASS_TYPE (klass), \
      GST_MSDEC_PARAMS_QDATA))

typedef struct _GstMSDecClassParams GstMSDecClassParams;
typedef struct _GstMSDecClass GstMSDecClass;
typedef struct _GstMSDec GstMSDec;

struct _GstMSDecClassParams
{
  const MSCodec * codec;
  const gchar * mimetype;
};

struct _GstMSDecClass
{
  GstBaseTransformClass parent_class;
};

struct _GstMSDec
{
  GstBaseTransform parent;

  MSDecoder * decoder;
};

GType gst_msdec_get_type (void);

void gst_msdec_register (GstPlugin * plugin,
                         const gchar * element_name,
                         const gchar * mimetype,
                         const MSCodec * codec);

G_END_DECLS

#endif /* __GST_MSDEC_H__ */
