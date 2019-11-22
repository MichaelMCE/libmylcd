
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


#ifndef _SYNC_H_
#define _SYNC_H_

#include "pth_sync.h"
#include "win_sync.h"

void resetSignal (TTHRDSYNCCTRL *s);
void setSignal (TTHRDSYNCCTRL *s);
int waitForSignal (TTHRDSYNCCTRL *s, int time);
int createEvent (TTHRDSYNCCTRL *s);
void deleteEvent (TTHRDSYNCCTRL *s);

int lock (TTHRDSYNCCTRL *s);
void unlock (TTHRDSYNCCTRL *s);
int lock_create (TTHRDSYNCCTRL *s);
void lock_delete (TTHRDSYNCCTRL *s);

#ifdef __DEBUG__
#define newThread(a, b, c) _newThread(a, b, c, __func__)
#define joinThread(a) _joinThread (a, __func__)
#else
#define newThread(a, b, c) _newThread(a, b, c, "")
#define joinThread(a) _joinThread (a, "")
#endif

int _newThread (TTHRDSYNCCTRL *s, void *func, void *arg, const char *name);
void _joinThread (TTHRDSYNCCTRL *s, const char *funcName);

void closeThreadHandle (TTHRDSYNCCTRL *s);





#endif
