
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

#ifndef __MSAUDIOHEALER_H__
#define __MSAUDIOHEALER_H__

#include "msbufferstream.h"

#include <glib.h>

#ifndef G_OS_WIN32
#include <winbase.h>
#else
#include <windows.h>
#endif

typedef struct _MSAudioHealer MSAudioHealer;

HRESULT WINAPI ms_audio_healer_start (MSAudioHealer * healer, gint reserved);
HRESULT WINAPI ms_audio_healer_push_samples (MSAudioHealer * healer, MSBufferStream * stream, gint reserved);
HRESULT WINAPI ms_audio_healer_pull_samples (MSAudioHealer * healer, MSBufferStream * stream, gint reserved, gint num_millisec);

#endif /* __MSAUDIOHEALER_H__ */
