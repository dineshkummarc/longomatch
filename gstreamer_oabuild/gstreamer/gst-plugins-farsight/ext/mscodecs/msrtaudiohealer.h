
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

#ifndef __MSRTAUDIOHEALER_H__
#define __MSRTAUDIOHEALER_H__

#include "msaudiohealer.h"

typedef gpointer (WINAPI * MSRTAudioHealerMallocFunc) (gsize size);
typedef void (WINAPI * MSRTAudioHealerFreeFunc) (gpointer memory);

void ms_rtaudio_healer_class_init (gpointer new_impl, gpointer free_impl);

HRESULT WINAPI ms_rtaudio_healer_new (MSAudioHealer ** healer,
    MSRTAudioHealerMallocFunc malloc_impl, MSRTAudioHealerFreeFunc free_impl);
HRESULT WINAPI ms_rtaudio_healer_free (MSAudioHealer * healer);

#endif /* __MSRTAUDIOHEALER_H__ */
