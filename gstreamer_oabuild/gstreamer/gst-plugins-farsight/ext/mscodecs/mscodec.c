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

#include "mscodec.h"
#include "msutils.h"

#ifdef G_OS_WIN32

__declspec(naked) gint WINAPI
ms_encoder_init (MSEncoder * encoder)
{
  INVOKE_VFUNC (0);
}

__declspec(naked) gint WINAPI
ms_encoder_encode (MSEncoder * encoder,
                   guint8 * in_buf, guint in_size,
                   guint8 * out_buf, guint * out_size)
{
  INVOKE_VFUNC (4);
}

__declspec(naked) gint WINAPI
ms_encoder_set_bitrate (MSEncoder * encoder,
                        guint bitrate)
{
  INVOKE_VFUNC (16);
}

__declspec(naked) gint WINAPI
ms_encoder_set_lossrate (MSEncoder * encoder,
                         gdouble percent_loss)
{
  INVOKE_VFUNC (20);
}

__declspec(naked) gint WINAPI
ms_decoder_init (MSDecoder * decoder)
{
  INVOKE_VFUNC (0);
}

__declspec(naked) gint WINAPI
ms_decoder_decode (MSDecoder * decoder,
                   guint8 * in_buf, guint in_size,
                   guint8 * out_buf, guint * out_size)
{
  INVOKE_VFUNC (4);
}

#endif

