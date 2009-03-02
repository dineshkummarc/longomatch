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

#ifndef __MSBUFFERRTPEXTHEADER_H__
#define __MSBUFFERRTPEXTHEADER_H__

#include <glib.h>

#ifndef G_OS_WIN32
#include <winbase.h>
#else
#include <windows.h>
#endif

#include "msbufferbase.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct {
  gdouble packet_ntp_timestamp;
  gint seq_no;
  gint field_C;
  gint64 timestamp;
  gint field_18;
  gint field_1C;
  gint unknown_enum_value;
  gint field_24;
  gint codec_id;
  gint ssrc;
  gint marker_bit_set;
  gint unknown_bool_for_enum_set;
  gint field_38;
  gint is_dtmf;
  gint field_40;
  gint field_44;
  gint field_48;
  gint field_4C;
  gint field_50;
  gint field_54;
  gint csrc_count;
} MSBufferRtpExtHeaderData;

typedef struct {
  MSBufferBase parent;
  MSBufferRtpExtHeaderData header_data;
  guint8 field_DC[56];
  gint field_114;
} MSBufferRtpExtHeader;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

void ms_buffer_rtp_ext_header_class_init (gpointer new_impl);

MSBufferRtpExtHeader * WINAPI ms_buffer_rtp_ext_header_new ();

#endif /* __MSBUFFERRTPEXTHEADER_H__ */
