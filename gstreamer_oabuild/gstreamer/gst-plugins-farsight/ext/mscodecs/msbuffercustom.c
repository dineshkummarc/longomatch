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

#include "msbuffercustom.h"

MSBufferCustom *
ms_buffer_custom_new (guint8 * buf, guint buf_size)
{
  MSBufferCustom * self;

  self = g_new0 (MSBufferCustom, 1);
  ms_buffer_base_init (&self->parent);

  self->parent.id = 0x20522520; /* unique ID identifying our class */
  self->parent.payload = buf;
  self->parent.payload_size = buf_size;

  return self;
}
