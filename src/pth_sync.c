
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

#if (__BUILD_PTHREADS_SUPPORT__)

#include "pth_sync.h"



void pth_resetSignal (TTHRDSYNCCTRL *s)
{
	pthread_cond_broadcast(&s->hSignal);
}

void pth_setSignal (TTHRDSYNCCTRL *s)
{
	pthread_cond_signal(&s->hSignal);
}

int pth_waitForSignal (TTHRDSYNCCTRL *s, int _time)
{
	struct timeval tv;
	struct timezone tz;
	struct timespec timeout;

#ifdef _WIN64
	mingw_gettimeofday(&tv, &tz);		// fix me
#else
	gettimeofday(&tv, &tz);	
#endif

	timeout.tv_nsec = 0;
	timeout.tv_sec = tv.tv_sec + _time;
	return pthread_cond_timedwait(&s->hSignal, &s->hSignalMutex, &timeout);

}

int pth_createEvent (TTHRDSYNCCTRL *s)
{
	s->hSignal = PTHREAD_COND_INITIALIZER;
	s->hSignalMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_init(&s->hSignal, NULL);
	pthread_mutex_init(&s->hSignalMutex, NULL);
	pthread_mutex_lock(&s->hSignalMutex);

	return 1;
}

void pth_deleteEvent (TTHRDSYNCCTRL *s)
{
	pthread_cond_destroy(&s->hSignal);
	pthread_mutex_destroy(&s->hSignalMutex);
}

/***************************************************/
/***************************************************/
/***************************************************/

int pth_lock (TTHRDSYNCCTRL *s)
{
	pthread_mutex_lock(&s->hLock);
	return 1;
}

void pth_unlock (TTHRDSYNCCTRL *s)
{
	pthread_mutex_unlock(&s->hLock);
}

int pth_lock_create (TTHRDSYNCCTRL *s)
{
	s->hLock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&s->hLock, NULL);
	return 1;
}

void pth_lock_delete (TTHRDSYNCCTRL *s)
{
	pthread_mutex_destroy(&s->hLock);
}

/***************************************************/
/***************************************************/
/***************************************************/
	
int pth_newThread (TTHRDSYNCCTRL *s, void *func, void *arg)
{
	pthread_create(&s->hThread, NULL, func, arg);
	return 1;
}

void pth_closeThreadHandle (TTHRDSYNCCTRL *s)
{

}

void pth_joinThread (TTHRDSYNCCTRL *s)
{
	pthread_join(s->hThread, NULL);
}


#endif

