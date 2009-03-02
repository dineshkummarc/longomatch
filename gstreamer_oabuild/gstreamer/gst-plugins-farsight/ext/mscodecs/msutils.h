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

#ifndef __MSUTILS_H__
#define __MSUTILS_H__

#define INVOKE_CFUNC(func_ptr) \
  __asm { \
    /* call the function through the function pointer */ \
    __asm jmp [func_ptr] \
  }

#define INVOKE_MFUNC(func_ptr) \
  __asm { \
    /* fill in ecx with 'this' pointer */ \
    __asm mov ecx, [esp + 4] \
    \
    /* put return address where 'this' pointer was */ \
    __asm pop edx \
    __asm mov [esp], edx \
    \
    /* call the function through the function pointer */ \
    __asm jmp [func_ptr] \
  }

#define INVOKE_VFUNC(func_offset) \
  __asm { \
    /* fill in ecx with 'this' pointer */ \
    __asm mov ecx, [esp + 4] \
    \
    /* put return address where 'this' pointer was */ \
    __asm pop edx \
    __asm mov [esp], edx \
    \
    /* call the function in the vtable */ \
    __asm mov edx, [ecx + 0] \
    __asm mov edx, [edx + func_offset] \
    __asm jmp edx \
  }

#endif /* __MSUTILS_H__ */
