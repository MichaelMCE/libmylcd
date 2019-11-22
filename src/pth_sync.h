
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


#ifndef _PTH_SYNC_H_
#define _PTH_SYNC_H_

#if (__BUILD_PTHREADS_SUPPORT__)

#include <sys/time.h>
#include <pthread.h>

#ifdef _WIN64
#ifndef _TIMEZONE_DEFINED /* also in time.h */
#define _TIMEZONE_DEFINED
struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

  extern int __cdecl mingw_gettimeofday (struct timeval *p, struct timezone *z);
#endif
#endif


typedef struct{
	pthread_cond_t		hSignal;		// event signal
	pthread_mutex_t		hSignalMutex;	// event signal mutex
	pthread_mutex_t		hLock;
	pthread_t			hThread;
}TTHRDSYNCCTRL;


void pth_resetSignal (TTHRDSYNCCTRL *s);
void pth_setSignal (TTHRDSYNCCTRL *s);
int pth_waitForSignal (TTHRDSYNCCTRL *s, int time);
int pth_createEvent (TTHRDSYNCCTRL *s);
void pth_deleteEvent (TTHRDSYNCCTRL *s);
int pth_lock (TTHRDSYNCCTRL *s);
void pth_unlock (TTHRDSYNCCTRL *s);
int pth_lock_create (TTHRDSYNCCTRL *s);
void pth_lock_delete (TTHRDSYNCCTRL *s);
int pth_newThread (TTHRDSYNCCTRL *s, void *func, void *arg);
void pth_closeThreadHandle (TTHRDSYNCCTRL *s);
void pth_joinThread (TTHRDSYNCCTRL *s);

#endif


#endif
