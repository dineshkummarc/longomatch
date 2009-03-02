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

#ifndef __MSBUFFERBASE_H__
#define __MSBUFFERBASE_H__

#include <glib.h>

#ifndef G_OS_WIN32
#include <winbase.h>
#else
#include <windows.h>
#endif

#define MS_BUFFER_BASE(obj) \
  ((MSBufferBase *) obj)

typedef struct {
  gpointer field_0;
  gpointer field_4;
  gpointer field_8;
  gpointer field_C;
  gpointer get_content_id;
  gpointer field_14;
} MSBufferBaseVTable;

typedef struct {
  MSBufferBaseVTable * vtable;
  gint field_4;
  gchar field_8[24];
  gint field_20;
  gint total_size;
  gint field_28;
  gint has_payload;
  gchar field_30[24];
  gint id;
  gpointer payload;
  gint payload_size;
  gint field_54;
  gint64 timestamp;
  gint field_60;
  gint field_64;
  gint field_68;
  gint field_6C;
  volatile glong ref_count;
  gint field_74;
  gint field_78;
  gint field_7C;
} MSBufferBase;

void ms_buffer_base_class_init (gpointer init_impl);

MSBufferBase * WINAPI ms_buffer_base_init (MSBufferBase * buffer_base);

#endif /* __MSBUFFERBASE_H__ */
