
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
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <windows.h>

#include "mylcd.h"
#include "memory.h"
#include "framec.h"
#include "frame.h"



static inline int _frameLock (TFRAMECACHE *frmc)
{
	//return (WaitForSingleObject(frmc->hMutex, INFINITE) == WAIT_OBJECT_0);
	EnterCriticalSection((CRITICAL_SECTION*)frmc->cs);
	return 1;
}

static inline void _frameUnlock (TFRAMECACHE *frmc)
{
	//ReleaseMutex(frmc->hMutex);
	LeaveCriticalSection((CRITICAL_SECTION*)frmc->cs);
}

static inline TFRAMERECORD *_frameGetRoot (TFRAMECACHE *frmc)
{
	return frmc->first;
}

// will return NULL if tree is empty otherwise will return a valid record
static inline TFRAMERECORD *_frameGetFirst (TFRAMECACHE *frmc)
{
	return _frameGetRoot(frmc);
}

static inline int _frameInsertFirst (TFRAMECACHE *frmc, void *item)
{
	TFRAMERECORD *newrec = (TFRAMERECORD*)l_malloc(sizeof(TFRAMERECORD));
	if (!newrec){
		//mylog("frameInsertFirst(): out of memory\n");
		return -1;
	}
	
	newrec->prev = NULL;
	newrec->item = item;
	
	TFRAMERECORD *rec = _frameGetFirst(frmc);
	if (rec){
		newrec->next = rec;
		rec->prev = newrec;
	}else{
		newrec->next = NULL;
	}
	frmc->first = newrec;
	return 1;
}

static inline void *_frameCreateItem (TFRAMECACHE *frmc, TFRAME *frame, const int gid)
{
	frame->groupId = gid;
	return frame;
	/*
	TFRAMEITEM *item = (TFRAMEITEM*)l_malloc(sizeof(TFRAMEITEM));
	if (item){
		item->frame = frame;
		item->groupId = gid;
	}
	return item;*/
}

static inline int _frameAdd (TFRAMECACHE *frmc, TFRAME *frame, const int gid)
{
	void *item = _frameCreateItem(frmc, frame, gid);
	if (item)
		return _frameInsertFirst(frmc, item);
	else
		return 0;
}


int frameAdd (TFRAMECACHE *frmc, TFRAME *frame, const int gid)
{
	int ret = 0;
	if (_frameLock(frmc)){
		ret = _frameAdd(frmc, frame, gid);
		_frameUnlock(frmc);
	}
	return ret;	
}

static inline int _frameGetTotal (TFRAMECACHE *frmc)
{
	int total = 0;

	TFRAMERECORD *rec = _frameGetFirst(frmc);
	if (rec){
		total++;
		while((rec=rec->next))
			total++;
	}
	return total;
}

int frameGetTotal (TFRAMECACHE *frmc)
{
	int ret = 0;
	if (_frameLock(frmc)){
		ret = _frameGetTotal(frmc);
		_frameUnlock(frmc);
	}
	return ret;	
}

static inline TFRAMERECORD *_frameGetRec (TFRAMECACHE *frmc, const TFRAME *frame)
{
	TFRAMERECORD *rec = _frameGetRoot(frmc);
	while(rec){
		if (rec->item == frame)
			return rec;
		rec = rec->next;
	}
	return NULL;
}

static inline void _freeFrame (TFRAMECACHE *frmc, TFRAME *frm)
{
	if (frm){
#ifdef __DEBUG__
		frmc->frameDeleteCount++;
#endif
		l_free(frm->pixels);
		l_free(frm->pops);
		l_free(frm);
	}
}

static inline void _frameDeleteItem (TFRAMECACHE *frmc, void *item)
{
	_freeFrame(frmc, (TFRAME*)item);
	//_freeFrame(frmc, item->frame);
	//l_free(item);
}

static inline void _frameDeleteRecord (TFRAMECACHE *frmc, TFRAMERECORD *rec)
{
	_frameDeleteItem(frmc, rec->item);
	l_free(rec);
}


static inline void _frameDeleteDelinkRec (TFRAMECACHE *frmc, TFRAMERECORD *rec)
{
	if (rec){
		if (frmc->first == rec){
			frmc->first = rec->next;
			if (rec->next)
				rec->next->prev = NULL;
		}else{
			if (rec->next)
				rec->next->prev = rec->prev;
			if (rec->prev)
				rec->prev->next = rec->next;
		}
		_frameDeleteRecord(frmc, rec);
	}else{
		//mylog("unable to delink record\n");
	}
}

static inline void _frameDeleteDelink (TFRAMECACHE *frmc, TFRAME *frame)
{
	_frameDeleteDelinkRec(frmc, _frameGetRec(frmc, frame));
}

void frameDeleteDelink (TFRAMECACHE *frmc, TFRAME *frame)
{
	if (_frameLock(frmc)){
		_frameDeleteDelink(frmc, frame);
		_frameUnlock(frmc);
	}
}

static inline void _frameDeleteAllByGroup (TFRAMECACHE *frmc, const int gid)
{
	TFRAMERECORD *rec = _frameGetRoot(frmc);
	if (rec){
		TFRAMERECORD *next;
		do{
			next = rec->next;
			if (((TFRAME*)rec->item)->groupId == gid)
				_frameDeleteDelinkRec(frmc, rec);
		}while((rec=next));
	}
}

void frameDeleteAllByGroup (TFRAMECACHE *frmc, const int gid)
{
	if (_frameLock(frmc)){
		_frameDeleteAllByGroup(frmc, gid);
		_frameUnlock(frmc);
	}
}

static inline void _frameDeleteAll (TFRAMECACHE *frmc)
{
	TFRAMERECORD *rec = _frameGetRoot(frmc);
	if (rec){
		TFRAMERECORD *next;
		do{
			next = rec->next;
			//printf("frame %p, %ix%i, %i\n", rec->item->frame, rec->item->frame->width, rec->item->frame->height, rec->item->frame->bpp);
			_frameDeleteRecord(frmc, rec);
		}while((rec=next));
	}
	frmc->first = NULL;
}

void framecDelete (TFRAMECACHE *frmc)
{
	_frameLock(frmc);
	_frameDeleteAll(frmc);
	//CloseHandle(frmc->hMutex);
	DeleteCriticalSection((CRITICAL_SECTION*)frmc->cs);
	l_free(frmc->cs);
	l_free(frmc);
}

TFRAMECACHE *framecNew ()
{
	TFRAMECACHE *frmc = l_calloc(1, sizeof(TFRAMECACHE));
	if (frmc){
		//frmc->hMutex = CreateMutex(NULL, FALSE, NULL);
		frmc->cs = l_calloc(1, sizeof(CRITICAL_SECTION));
		if (frmc->cs){
			InitializeCriticalSectionAndSpinCount(frmc->cs, 8192);
			frmc->first = NULL;
			frmc->frameCount = 0;
			frmc->frameRescueCount = 0;
			frmc->frameDeleteCount = 0;
			return frmc;
		}
		l_free(frmc);
	}
	return NULL;
}

