
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


#ifndef _WIN_SYNC_H_
#define _WIN_SYNC_H_


#if (__BUILD_PTHREADS_SUPPORT__ == 0)

#include <windows.h>

typedef struct{
	HANDLE				hSignal;		// event signal
#if 1
	CRITICAL_SECTION	cs;
#else
	HANDLE				hMutex;
#endif
	uintptr_t			hThread;
}TTHRDSYNCCTRL;


void win_resetSignal (TTHRDSYNCCTRL *s);
void win_setSignal (TTHRDSYNCCTRL *s);
int win_waitForSignal (TTHRDSYNCCTRL *s, int time);
int win_createEvent (TTHRDSYNCCTRL *s);
void win_deleteEvent (TTHRDSYNCCTRL *s);
int win_lock (TTHRDSYNCCTRL *s);
void win_unlock (TTHRDSYNCCTRL *s);
int win_lock_create (TTHRDSYNCCTRL *s);
void win_lock_delete (TTHRDSYNCCTRL *s);
int win_newThread (TTHRDSYNCCTRL *s, void *func, void *arg);
void win_closeThreadHandle (TTHRDSYNCCTRL *s);
void win_joinThread (TTHRDSYNCCTRL *s);


#endif

#endif
