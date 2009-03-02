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

#ifndef __GST_RTP_RTAUDIO_PAY_H__
#define __GST_RTP_RTAUDIO_PAY_H__

#include <gst/gst.h>
#include <gst/rtp/gstbasertppayload.h>

G_BEGIN_DECLS

#define GST_TYPE_RTP_RTAUDIO_PAY \
  (gst_rtp_rtaudio_pay_get_type ())
#define GST_RTP_RTAUDIO_PAY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_RTP_RTAUDIO_PAY, GstRtpRTAudioPay))
#define GST_RTP_RTAUDIO_PAY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_RTP_RTAUDIO_PAY, GstRtpRTAudioPayClass))
#define GST_IS_RTP_RTAUDIO_PAY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_RTP_RTAUDIO_PAY))
#define GST_IS_RTP_RTAUDIO_PAY_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_RTP_RTAUDIO_PAY))

typedef struct _GstRtpRTAudioPayClass GstRtpRTAudioPayClass;
typedef struct _GstRtpRTAudioPay GstRtpRTAudioPay;

struct _GstRtpRTAudioPayClass
{
  GstBaseRTPPayloadClass parent_class;
};

struct _GstRtpRTAudioPay
{
  GstBaseRTPPayload parent;
};

GType gst_rtp_rtaudio_pay_get_type (void);

G_END_DECLS

#endif /* __GST_RTP_RTAUDIO_PAY_H__ */
