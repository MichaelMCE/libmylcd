
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


#ifndef _PTHREADS_DYN_H_
#define _PTHREADS_DYN_H_


#include <windows.h>
#include <pthread.h>

#define PTHREAD_DLL_NAME "pthreadGC2.dll"

#undef DLL_DECLARE
#define DLL_DECLARE(ret, api, name, args) \
  typedef ret (*__dll_##name##_t)args; static __dll_##name##_t _##name

#undef DLL_LOAD
#define DLL_LOAD(dll, name, ret_on_failure)                 			\
  do {                                             						\
  /*printf("loading %s %s\n", #name, dll);*/							\
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



DLL_DECLARE(int, PTW32_CDECL, pthread_create, (pthread_t *tid, const pthread_attr_t *attr, void *(*start) (void *), void *arg));
DLL_DECLARE(int, PTW32_CDECL, pthread_mutex_init, (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr));
DLL_DECLARE(int, PTW32_CDECL, pthread_join, (pthread_t thread, void **value_ptr));
DLL_DECLARE(int, PTW32_CDECL, pthread_mutex_lock, (pthread_mutex_t *mutex));
DLL_DECLARE(int, PTW32_CDECL, pthread_mutex_unlock, (pthread_mutex_t *mutex));
DLL_DECLARE(int, PTW32_CDECL, pthread_mutex_destroy, (pthread_mutex_t *mutex));
DLL_DECLARE(void,PTW32_CDECL, pthread_exit, (void *value_ptr));
DLL_DECLARE(int, PTW32_CDECL, pthread_cond_init, (pthread_cond_t *cond, const pthread_condattr_t *attr));
DLL_DECLARE(int, PTW32_CDECL, pthread_cond_signal, (pthread_cond_t *cond));
DLL_DECLARE(int, PTW32_CDECL, pthread_cond_broadcast, (pthread_cond_t *cond));
DLL_DECLARE(int, PTW32_CDECL, pthread_cond_wait, (pthread_cond_t *cond, pthread_mutex_t *mutex));
DLL_DECLARE(int, PTW32_CDECL, pthread_cond_timedwait, (pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime));
DLL_DECLARE(int, PTW32_CDECL, pthread_cond_destroy, (pthread_cond_t *cond));



#endif

