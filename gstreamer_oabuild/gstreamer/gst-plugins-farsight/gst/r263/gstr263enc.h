/* GStreamer
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org> 
 * Copyright (C) 1999-2005 Wang, Zhanglei <filamoon@hotmail.com> 
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

#ifndef __GST_R263ENC_H__
#define __GST_R263ENC_H__

#include <gst/gst.h>

#include <libr263/libr263.h>
G_BEGIN_DECLS

/* #define's don't like whitespacey bits */
#define GST_TYPE_R263ENC (gst_r263enc_get_type())
#define GST_R263ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_R263ENC,GstR263Enc))
#define GST_R263ENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_R263ENC,GstR263Enc))
#define GST_IS_R263ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_R263ENC))
#define GST_IS_R263ENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_R263ENC))

typedef struct _GstR263Enc      GstR263Enc;
typedef struct _GstR263EncClass GstR263EncClass;

GType gst_r263enc_get_type (void);

struct _GstR263Enc
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gboolean silent;
  gboolean rtp_support;
  gint width, height;
  gdouble framerate;
  gboolean enc_inited;
  gboolean is_qcif;
    
  /* libr263 parameters */
  CParam cparams;
  Bits bits;
  gint * EncodeBlocks;
  gint gop_length;
  guint frame_no;
  //struct qcif * qcif_frame;
  unsigned int * src_int_frame;
};

struct _GstR263EncClass
{
  GstElementClass parent_class;
};

GType gst_plugin_r263enc_get_type (void);

G_END_DECLS

#endif /* __GST_R263ENC_H__ */
