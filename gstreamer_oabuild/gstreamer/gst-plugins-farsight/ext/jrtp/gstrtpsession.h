/*
 * Farsight
 * GStreamer RTP Session element using JRTPlib
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
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

#ifndef __GST_RTPSESSION_H__
#define __GST_RTPSESSION_H__

#include <gst/gst.h>
#include "jrtplib_c.h"

G_BEGIN_DECLS

/* #define's don't like whitespacey bits */
#define GST_TYPE_RTPSESSION \
  (gst_gst_rtpsession_get_type())
#define GST_RTPSESSION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTPSESSION,GstRTPSession))
#define GST_RTPSESSION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTPSESSION,GstRTPSession))
#define GST_IS_RTPSESSION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTPSESSION))
#define GST_IS_RTPSESSION_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTPSESSION))

typedef struct _GstRTPSession      GstRTPSession;
typedef struct _GstRTPSessionClass GstRTPSessionClass;

struct _GstRTPSession
{
  GstElement element;

  GstPad *rtpsinkpad, *rtcpsinkpad, *rtpsrcpad, *rtcpsrcpad, *datasinkpad, *datasrcpad;

  // This holds our RTPSession
  jrtpsession_t _sess;
  rtpgstv4transmissionparams_t _params;

  guint         _localportbase;
  gchar*        _destaddrs;
  guint         _clockrate;

  guint8        _defaultpt;
  guint32       _defaulttsinc;
  gboolean      _defaultmark;
  
  gboolean silent;
};

struct _GstRTPSessionClass 
{
  GstElementClass parent_class;
};

GType gst_gst_rtpsession_get_type (void);

G_END_DECLS

#endif /* __GST_RTPSESSION_H__ */
