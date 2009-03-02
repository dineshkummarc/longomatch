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


#ifndef __GST_RTPDVPAYLOAD_H__
#define __GST_RTPDVPAYLOAD_H__

#include <gst/gst.h>
#include <gst/rtp/gstbasertppayload.h>

G_BEGIN_DECLS

typedef struct _GstDVPayload GstDVPayload;
typedef struct _GstDVPayloadClass GstDVPayloadClass;

#define GST_TYPE_RTPDVPAYLOAD \
  (gst_dvpayload_get_type())
#define GST_RTPDVPAYLOAD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTPDVPAYLOAD,GstDVPayload))
#define GST_RTPDVPAYLOAD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTPDVPAYLOAD,GstDVPayload))
#define GST_IS_RTPDVPAYLOAD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTPDVPAYLOAD))
#define GST_IS_RTPDVPAYLOAD_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTPDVPAYLOAD))

struct _GstDVPayload
{
  GstBaseRTPPayload payload;
};

struct _GstDVPayloadClass
{
  GstBaseRTPPayloadClass parent_class;
};

gboolean gst_dvpayload_plugin_init (GstPlugin * plugin);

G_END_DECLS

#endif /* __GST_RTPDVPAYLOAD_H__ */
