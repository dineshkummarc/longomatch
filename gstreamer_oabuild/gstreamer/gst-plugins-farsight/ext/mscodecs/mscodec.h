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

#ifndef __MSCODEC_H__
#define __MSCODEC_H__

#include <glib.h>
#ifdef G_OS_WIN32
#include <windows.h>
#else
#include <winbase.h>
#endif

G_BEGIN_DECLS

typedef enum
{
  MS_CODEC_ID_RTA8  = 6,
  MS_CODEC_ID_RTA16 = 7
} MSCodecId;

typedef struct _MSEncoder MSEncoder;
typedef struct _MSDecoder MSDecoder;

typedef gint (WINAPI * MSEncoderCreateFunc)  (MSEncoder ** enc);
typedef gint (WINAPI * MSEncoderDestroyFunc) (MSEncoder * enc);
typedef gint (WINAPI * MSDecoderCreateFunc)  (MSDecoder ** dec);
typedef gint (WINAPI * MSDecoderDestroyFunc) (MSDecoder * dec);

#ifdef _MSC_VER
#pragma pack(push, 1)
#else

#endif

typedef struct {
  gint unknown_0;
  gint id;
  gint pt;
  const gchar * name;
  gint rate;
  gint y;
  gint z;
  MSEncoderCreateFunc  create_encoder;
  MSEncoderDestroyFunc destroy_encoder;
  MSDecoderCreateFunc  create_decoder;
  MSDecoderDestroyFunc destroy_decoder;
} MSCodec;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

gint WINAPI ms_encoder_init (MSEncoder * encoder);
gint WINAPI ms_encoder_encode (MSEncoder * encoder, guint8 * in_buf, guint in_size, guint8 * out_buf, guint * out_size);
gint WINAPI ms_encoder_set_bitrate (MSEncoder * encoder, guint bitrate);
gint WINAPI ms_encoder_set_lossrate (MSEncoder * encoder, gdouble percent_loss);

gint WINAPI ms_decoder_init (MSDecoder * decoder);
gint WINAPI ms_decoder_decode (MSDecoder * decoder, guint8 * in_buf, guint in_size, guint8 * out_buf, guint * out_size);

G_END_DECLS

#endif /* __MSCODEC_H__ */
