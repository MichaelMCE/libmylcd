
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


#include "mylcd.h"
#include "sync.h"
#include "misc.h"


void resetSignal (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_resetSignal(s);
#else
	return win_resetSignal(s);
#endif
}

void setSignal (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_setSignal(s);
#else
	return win_setSignal(s);
#endif
}

int waitForSignal (TTHRDSYNCCTRL *s, int time)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_waitForSignal(s, time);
#else
	return win_waitForSignal(s, time);
#endif
}

int createEvent (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_createEvent(s);
#else
	return win_createEvent(s);
#endif
}

void deleteEvent (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_deleteEvent(s);
#else
	return win_deleteEvent(s);
#endif
}

/***************************************************/
/***************************************************/
/***************************************************/

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

/***************************************************/
/***************************************************/
/***************************************************/

int _newThread (TTHRDSYNCCTRL *s, void *func, void *arg, const char *funcName)
{
	mylog("%s(): ", funcName);
	
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_newThread(s, func, arg);
#else
	return win_newThread(s, func, arg);
#endif
}

void closeThreadHandle (TTHRDSYNCCTRL *s)
{
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_closeThreadHandle(s);
#else
	return win_closeThreadHandle(s);
#endif
}

void _joinThread (TTHRDSYNCCTRL *s, const char *funcName)
{
	mylog("%s(): ", funcName);
	
#if (__BUILD_PTHREADS_SUPPORT__)
	return pth_joinThread(s);
#else
	return win_joinThread(s);
#endif
}

