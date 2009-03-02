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

#include "msbufferrtpextheader.h"
#include "msutils.h"

gpointer _ms_buffer_rtp_ext_header_new_impl = NULL;

void
ms_buffer_rtp_ext_header_class_init (gpointer new_impl)
{
  _ms_buffer_rtp_ext_header_new_impl = new_impl;
}

#ifdef G_OS_WIN32

__declspec(naked) MSBufferRtpExtHeader * WINAPI
ms_buffer_rtp_ext_header_new ()
{
  INVOKE_CFUNC (_ms_buffer_rtp_ext_header_new_impl);
}

#endif

