
// libmylcd
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

#include "pthreads_dyn.h"

static int pthreadInitStatus = 0;



static void dll_load_error (const char *fn, const char *dll, const int err)
{
	if (err == -1)
		printf("libmylcd: '%s' not found\n", dll);
	else if (err == -2)
		printf("libmylcd: symbol '%s' not found in '%s'\n", fn, dll);
}

int pthreadInit ()
{
	const char *dll = PTHREAD_DLL_NAME;
	
	if (pthreadInitStatus)
		return 1;

	DLL_LOAD(dll, pthread_create, 1);
	DLL_LOAD(dll, pthread_mutex_init, 1);
	DLL_LOAD(dll, pthread_join, 1);
	DLL_LOAD(dll, pthread_mutex_lock, 1);
	DLL_LOAD(dll, pthread_mutex_unlock, 1);
	DLL_LOAD(dll, pthread_mutex_destroy, 1);
	DLL_LOAD(dll, pthread_exit, 1);
	DLL_LOAD(dll, pthread_cond_init, 1);
	DLL_LOAD(dll, pthread_cond_signal, 1);
	DLL_LOAD(dll, pthread_cond_broadcast, 1);
	DLL_LOAD(dll, pthread_cond_wait, 1);
	DLL_LOAD(dll, pthread_cond_timedwait, 1);
	DLL_LOAD(dll, pthread_cond_destroy, 1);	
	pthreadInitStatus = 1;
	return 1;

}

int pthread_cond_destroy (pthread_cond_t *cond)
{
	if (_pthread_cond_destroy)
		return _pthread_cond_destroy(cond);
	else
		return -1;
}

int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	if (_pthread_cond_wait)
		return _pthread_cond_wait(cond, mutex);
	else
		return -1;
}

int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
	if (_pthread_cond_timedwait)
		return _pthread_cond_timedwait(cond, mutex, abstime);
	else
		return -1;
}

int pthread_cond_broadcast (pthread_cond_t *cond)
{
	if (_pthread_cond_broadcast)
		return _pthread_cond_broadcast(cond);
	else
		return -1;
}

int pthread_cond_signal (pthread_cond_t *cond)
{
	if (_pthread_cond_signal)
		return _pthread_cond_signal(cond);
	else
		return -1;
}

int pthread_cond_init (pthread_cond_t *cond, const pthread_condattr_t *attr)
{
	if (!pthreadInitStatus){
		if (!pthreadInit())
			return -1;
	}	
	
//	if (_pthread_cond_init)
		return _pthread_cond_init(cond, attr);
//	else
//		return -1;
}

int pthread_create (pthread_t *tid, const pthread_attr_t *attr, void *(*start) (void *), void *arg)
{
	if (!pthreadInitStatus){
		if (!pthreadInit())
			return -1;
	}	
	
//	if (_pthread_create)
		return _pthread_create(tid, attr, start, arg);
//	else
//		return -1;
}

int pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	if (!pthreadInitStatus){
		if (!pthreadInit())
			return -1;
	}	
	
//	if (_pthread_mutex_init)
		return _pthread_mutex_init(mutex, attr);
//	else
//		return -1;
}

void pthread_exit (void *value_ptr)
{
	if (_pthread_exit)
		_pthread_exit(value_ptr);
}

int pthread_join (pthread_t thread, void **value_ptr)
{
	if (_pthread_join)
		return _pthread_join(thread, value_ptr);
	else
		return -1;
}

int pthread_mutex_lock (pthread_mutex_t *mutex)
{
	if (_pthread_mutex_lock)
		return _pthread_mutex_lock(mutex);
	else
		return -1;
}

int pthread_mutex_unlock (pthread_mutex_t *mutex)
{
	if (_pthread_mutex_unlock)
		return _pthread_mutex_unlock(mutex);
	else
		return -1;
}

int pthread_mutex_destroy (pthread_mutex_t *mutex)
{
	if (_pthread_mutex_destroy)
		return _pthread_mutex_destroy(mutex);
	else
		return -1;
}

#endif

