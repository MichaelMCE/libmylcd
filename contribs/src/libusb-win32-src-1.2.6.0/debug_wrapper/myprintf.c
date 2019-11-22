
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


#define __WIN32__ 1


#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <ctype.h>
#include <wchar.h>


#ifndef _WIN64
//#include <ansidecl.h>	/* for VA_OPEN and VA_CLOSE */ 
#endif

#ifndef	_ANSIDECL_H
#undef VA_OPEN
#undef VA_CLOSE
#undef VA_FIXEDARG

#define VA_OPEN(AP, VAR)	{ va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)		} va_end(AP); }
#define VA_FIXEDARG(AP, T, N)	struct Qdmy
#endif



typedef struct{
	//HANDLE				hSignal;		// event signal
#if 0
	CRITICAL_SECTION	hLock;
	HANDLE				hSemLock;
#else
	volatile HANDLE		hMutex;
#endif
	//uintptr_t			hThread;
}TTHRDSYNCCTRL;


int lock (TTHRDSYNCCTRL *s);
void unlock (TTHRDSYNCCTRL *s);
int lock_create (TTHRDSYNCCTRL *s);
void lock_delete (TTHRDSYNCCTRL *s);


#ifdef __WIN32__

static int syncCreated = 0;
static TTHRDSYNCCTRL sync;

#endif


int win_lock (TTHRDSYNCCTRL *s)
{
	return (s->hMutex && WaitForSingleObject(s->hMutex, INFINITE) == WAIT_OBJECT_0);
}

void win_unlock (TTHRDSYNCCTRL *s)
{
	if (s->hMutex)
		ReleaseMutex(s->hMutex);
}

int win_lock_create (TTHRDSYNCCTRL *s)
{
	s->hMutex = CreateMutex(NULL, FALSE, NULL);
	return (s->hMutex != NULL);
}

void win_lock_delete (TTHRDSYNCCTRL *s)
{
	if (s->hMutex){
		CloseHandle(s->hMutex);
		s->hMutex = NULL;
	}
}


int lock (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_lock(s);
#else
	return win_lock(s);
#endif
}

void unlock (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_unlock(s);
#else
	return win_unlock(s);
#endif
}

int lock_create (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_lock_create(s);
#else
	return win_lock_create(s);
#endif
}

void lock_delete (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_lock_delete(s);
#else
	return win_lock_delete(s);
#endif
}


void my_lock ()
{
	lock(&sync);
}

void my_unlock ()
{
	unlock(&sync);
}


int initDebugPrint ()
{
	if (!syncCreated){
		syncCreated = 1;
		lock_create(&sync);
	}
	return 1;
}

void closeDebugPrint ()
{
//	lock(&sync);
//	lock_delete(&sync);
}


int my_printf (const char *str, ...)
{
	int ret = -1;
#ifdef __WIN32__
	if (lock(&sync)){
		VA_OPEN(ap, str);
		ret = vprintf(str, ap);
		VA_CLOSE(ap);
		unlock(&sync);
	}
	
#else
	VA_OPEN(ap, str);
	ret = vprintf(str, ap);
	VA_CLOSE(ap);
#endif

	fflush(stdout);
	return ret;
}

int my_wprintf (const wchar_t *wstr, ...)
{
	int ret = -1;
#ifdef __WIN32__
	if (lock(&sync)){
		VA_OPEN(ap, wstr);
		ret = vwprintf(wstr, ap);
		VA_CLOSE(ap);
		unlock(&sync);
	}
	
#else
	VA_OPEN(ap, wstr);
	ret = vwprintf(wstr, ap);
	VA_CLOSE(ap);
#endif

	fflush(stdout);
	return ret;
}

