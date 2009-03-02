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

#include "msaudiohealer.h"
#include "msutils.h"

#ifdef G_OS_WIN32

__declspec(naked) HRESULT WINAPI
ms_audio_healer_start (MSAudioHealer * healer,
                       gint reserved)
{
  INVOKE_VFUNC (4);
}

__declspec(naked) HRESULT WINAPI
ms_audio_healer_push_samples (MSAudioHealer * healer,
                              MSBufferStream * stream,
                              gint reserved)
{
  INVOKE_VFUNC (28);
}

__declspec(naked) HRESULT WINAPI
ms_audio_healer_pull_samples (MSAudioHealer * healer,
                              MSBufferStream * stream,
                              gint reserved,
                              gint num_millisec)
{
  INVOKE_VFUNC (32);
}

#endif

