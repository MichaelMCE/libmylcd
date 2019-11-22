
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.


#ifndef _LIBUSB_DYN_H_
#define _LIBUSB_DYN_H_


#undef DLL_DECLARE
#define DLL_DECLARE(ret, api, name, args) \
  typedef ret (api * __dll_##name##_t)args; static __dll_##name##_t _##name

#undef DLL_LOAD
#define DLL_LOAD(dll, name, ret_on_failure)                 			\
  do {                                             						\
  HANDLE h = (HANDLE)GetModuleHandle(dll);             					\
  if (!h)                                              					\
    h = (HANDLE)LoadLibrary(dll);                      					\
  if (!h){                                            					\
    if (ret_on_failure){                               					\
      dll_load_error(#name, dll, -1);									\
      return -1;                             							\
    }																	\
    else break; }                                             			\
  if ((_##name = (__dll_##name##_t)GetProcAddress(h, #name))) 			\
    break;                                                    			\
																		\
  if (ret_on_failure){                                         			\
    dll_load_error(#name, dll, -2);										\
    return -2;                               						  	\
   }																	\
  }while(0)



#endif

