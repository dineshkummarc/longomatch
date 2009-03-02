/*
 * Copyright 2007 Ole André Vadla Ravnås <ole.andre.ravnas@tandberg.com>
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

#ifndef __GST_AUDIO_CHUNK_H__
#define __GST_AUDIO_CHUNK_H__

#include <gst/gst.h>
#include <gst/base/gstadapter.h>

G_BEGIN_DECLS

#define GST_TYPE_AUDIO_CHUNK \
  (gst_audio_chunk_get_type ())
#define GST_AUDIO_CHUNK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_AUDIO_CHUNK, GstAudioChunk))
#define GST_AUDIO_CHUNK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_AUDIO_CHUNK, GstAudioChunkClass))
#define GST_IS_AUDIO_CHUNK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_AUDIO_CHUNK))
#define GST_IS_AUDIO_CHUNK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_AUDIO_CHUNK))

typedef struct _GstAudioChunk      GstAudioChunk;
typedef struct _GstAudioChunkClass GstAudioChunkClass;

struct _GstAudioChunk
{
  GstElement element;

  GstPad * sink_pad;
  GstPad * src_pad;

  gint samples_per_buffer;

  GstAdapter * adapter;
  guint64 offset;
  GstClockTime time;
};

struct _GstAudioChunkClass
{
  GstElementClass parent_class;
};

GType gst_audio_chunk_get_type (void);

G_END_DECLS

#endif /* __GST_AUDIO_CHUNK_H__ */
