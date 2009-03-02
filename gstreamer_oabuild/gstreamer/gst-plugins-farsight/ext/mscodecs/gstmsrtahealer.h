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

#ifndef __GST_MSRTAHEALER_H__
#define __GST_MSRTAHEALER_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_MS_RTA_HEALER \
  (gst_ms_rta_healer_get_type ())
#define GST_MS_RTA_HEALER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_MS_RTA_HEALER, GstMSRTAHealer))
#define GST_MS_RTA_HEALER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_MS_RTA_HEALER, GstMSRTAHealerClass))
#define GST_IS_MS_RTA_HEALER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_MS_RTA_HEALER))
#define GST_IS_MS_RTA_HEALER_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_MS_RTA_HEALER))

typedef struct _GstMSRTAHealerClass GstMSRTAHealerClass;
typedef struct _GstMSRTAHealer GstMSRTAHealer;
typedef struct _GstMSRTAHealerPrivate GstMSRTAHealerPrivate;

struct _GstMSRTAHealerClass
{
  GstElementClass parent_class;
};

struct _GstMSRTAHealer
{
  GstElement parent;
};

GType gst_ms_rta_healer_get_type (void);

G_END_DECLS

#endif /* __GST_MSRTAHEALER_H__ */
