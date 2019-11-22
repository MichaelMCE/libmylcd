/* LIBUSB-WIN32, Generic Windows USB Library
 * Copyright (c) 2002-2005 Stephan Meyer <ste_meyer@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _USBD480_LIB_DYN_H_
#define _USBD480_LIB_DYN_H_


#include <windows.h>

#undef DLL_DECLARE
#define DLL_DECLARE(ret, api, name, args) \
  typedef ret (api * __dll_##name##_t)args; __dll_##name##_t _##name

#undef DLL_LOAD
#define DLL_LOAD(dll, name, ret_on_failure)		\
  do {											\
  HANDLE h = GetModuleHandle(dll);				\
  if(!h)										\
    h = LoadLibrary(dll);						\
  if(!h) {										\
    if (ret_on_failure){						\
      dll_load_error(#name, dll, -1);			\
      return -1;								\
    }											\
    else break; }								\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@0")))	\
    break;                                              				\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@4")))	\
    break;                                                    			\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@8")))	\
    break;                                                    			\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@12")))	\
    break;                                                    			\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@16")))	\
    break;                                                    			\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@20")))	\
    break;                                                    			\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@24")))	\
    break;                                                    			\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@28")))	\
    break;                                                    			\
  if((_##name = (__dll_##name##_t)GetProcAddress(h, "_" #name "@32")))	\
    break;										\
  if (ret_on_failure){							\
    dll_load_error(#name, dll, -2);				\
    return -2;									\
   }											\
  } while(0)




DLL_DECLARE(int, , USBD480_GetNumberOfDisplays, (void));
DLL_DECLARE(int, , USBD480_GetDisplayConfiguration, (uint32_t index, DisplayInfo *di));
DLL_DECLARE(int, , USBD480_Open, (DisplayInfo *di, uint32_t flags));
DLL_DECLARE(int, , USBD480_Close, (DisplayInfo *di));
DLL_DECLARE(int, , USBD480_DrawFullScreen, (DisplayInfo *di, uint8_t *fb));
DLL_DECLARE(int, , USBD480_DrawFullScreenRGBA32, (DisplayInfo *di, uint32_t *fb));
DLL_DECLARE(int, , USBD480_DrawFullScreenBGRA32, (DisplayInfo *di, uint32_t *fb));
DLL_DECLARE(int, , USBD480_SetBrightness, (DisplayInfo *di, uint32_t brightness));
DLL_DECLARE(int, , USBD480_SetTouchMode, (DisplayInfo *di, uint32_t mode));
DLL_DECLARE(int, , USBD480_SetAddress, (DisplayInfo *di, uint32_t address));
DLL_DECLARE(int, , USBD480_SetFrameStartAddress, (DisplayInfo *di, uint32_t address));
DLL_DECLARE(int, , USBD480_DrawFromBuffer, (DisplayInfo *di, uint8_t *fb, uint32_t size));
DLL_DECLARE(int, , USBD480_GetTouchReport, (DisplayInfo *di, /*TouchReport*/ void *touch));
//DLL_DECLARE(int, , USBD480_GetTouchPosition, (DisplayInfo *di, uint32_t *x, uint32_t *y));


#endif

