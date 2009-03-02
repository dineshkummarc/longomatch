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

#ifndef __MSBUFFERSTREAM_H__
#define __MSBUFFERSTREAM_H__

#include <glib.h>

#ifndef G_OS_WIN32
#include <winbase.h>
#else
#include <windows.h>
#endif

#include "msbufferbase.h"

enum
{
  MS_BUFFER_INDEX_RTP_EXT_HEADER = 2,
  MS_BUFFER_INDEX_DECODED_MEDIA = 10,
  MS_BUFFER_INDEX_ENCODED_MEDIA = 16,
};

typedef struct _MSBufferStream MSBufferStream;

void ms_buffer_stream_class_init (gpointer new_impl, gpointer add_buffer_impl, gpointer update_offset_and_size_impl);

MSBufferStream * WINAPI ms_buffer_stream_new ();
MSBufferBase * WINAPI ms_buffer_stream_add_buffer (MSBufferStream * stream, gint index, MSBufferBase * buffer);
gpointer WINAPI ms_buffer_stream_update_offset_and_size (MSBufferStream * stream, gint index, gint new_offset, gint new_size);

#endif /* __MSBUFFERSTREAM_H__ */
