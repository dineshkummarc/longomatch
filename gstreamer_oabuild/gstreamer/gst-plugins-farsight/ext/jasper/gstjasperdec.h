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

#ifndef __GST_JASPERDEC_H__
#define __GST_JASPERDEC_H__

#include <gst/gst.h>
#include <jasper/jasper.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_JASPERDEC         (gst_jasperdec_get_type())
#define GST_JASPERDEC(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_JASPERDEC,GstJasperDec))
#define GST_JASPERDEC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_JASPERDEC,GstJasperDec))
#define GST_IS_JASPERDEC(obj)      (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_JASPERDEC))
#define GST_IS_JASPERDEC_CLASS(obj)(G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_JASPERDEC))

typedef struct _GstJasperDec GstJasperDec;
typedef struct _GstJasperDecClass GstJasperDecClass;

struct _GstJasperDec
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gint width;
  gint height;
  gint bpp;
};

struct _GstJasperDecClass
{
  GstElementClass parent_class;
};

GType gst_jasperdec_get_type(void);

gboolean gst_jasperdec_plugin_init (GstPlugin * plugin);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GST_JASPERDEC_H__ */
