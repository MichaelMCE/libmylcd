
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

#if (!__BUILD_PTHREADS_SUPPORT__)

#include <process.h>
#include "win_sync.h"
#include "misc.h"

void win_resetSignal (TTHRDSYNCCTRL *s)
{
	ResetEvent(s->hSignal);
}

void win_setSignal (TTHRDSYNCCTRL *s)
{
	SetEvent(s->hSignal);
}

int win_waitForSignal (TTHRDSYNCCTRL *s, const int mstime)
{
	return WaitForSingleObject(s->hSignal, mstime);
}

int win_createEvent (TTHRDSYNCCTRL *s)
{
	s->hSignal = CreateEvent(NULL, 0, 0, NULL);
	return (intptr_t)s->hSignal;
}

void win_deleteEvent (TTHRDSYNCCTRL *s)
{
	CloseHandle(s->hSignal);
}

/***************************************************/
/***************************************************/
/***************************************************/

int win_lock (TTHRDSYNCCTRL *s)
{
	//return (s->hMutex && WaitForSingleObject(s->hMutex, INFINITE) == WAIT_OBJECT_0);
	EnterCriticalSection(&s->cs);
	return (s != NULL);
}

void win_unlock (TTHRDSYNCCTRL *s)
{
	//if (s->hMutex)
		//ReleaseMutex(s->hMutex);
	LeaveCriticalSection(&s->cs);
}

int win_lock_create (TTHRDSYNCCTRL *s)
{
	//s->hMutex = CreateMutex(NULL, FALSE, NULL);
	//return (s->hMutex != NULL);
	InitializeCriticalSectionAndSpinCount(&s->cs, 8196);
	return 8196;
}

void win_lock_delete (TTHRDSYNCCTRL *s)
{
	//if (s->hMutex){
	//	CloseHandle(s->hMutex);
	//	s->hMutex = NULL;
	//}
	DeleteCriticalSection(&s->cs);
}

/***************************************************/
/***************************************************/
/***************************************************/
	
int win_newThread (TTHRDSYNCCTRL *s, void *func, void *arg)
{
	unsigned int threadID;
	s->hThread = _beginthreadex(NULL, 0, func, arg, 0, &threadID);
	mylog("newThread: %i\n", s->hThread);
	return (int)s->hThread;
}

void win_closeThreadHandle (TTHRDSYNCCTRL *s)
{
	mylog("closeThreadHandle: %i\n", s->hThread);
	CloseHandle((HANDLE)s->hThread);
}

void win_joinThread (TTHRDSYNCCTRL *s)
{
	mylog("joinThread: %i\n", s->hThread);
	if (WaitForSingleObject((HANDLE)s->hThread, INFINITE) != WAIT_OBJECT_0){
		mylog("joinThread: object timed out: %i\n", s->hThread);
	}
}

#endif

