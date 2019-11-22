
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

#if (__BUILD_SCROLL_SUPPORT__)


#include "memory.h"
#include "frame.h"
#include "display.h"
#include "utils.h"
#include "pixel.h"
#include "copy.h"
#include "scroll.h"


static INLINE void moveLeft (const TLSCROLLEX *const p, const int mode);
static INLINE void moveRight(const TLSCROLLEX *s, const int mode);
static INLINE void moveUp   (const TLSCROLLEX *s, const int mode);
static INLINE void moveDown (const TLSCROLLEX *s, const int mode);
static INLINE void copyLeft (const TLSCROLLEX *const s);
static INLINE void copyRight(const TLSCROLLEX *const s);
static INLINE void copyUp   (const TLSCROLLEX *const s);
static INLINE void copyDown (const TLSCROLLEX *const s);
static INLINE void minRect  (TLPOINTEX  *const m, const TLSCROLLEX *const s);
static INLINE void scrollUp (TLSCROLLEX *const s);
static INLINE void scrollDown (TLSCROLLEX *const s);
static INLINE void scrollLeft (TLSCROLLEX *const s);
static INLINE void scrollRight (TLSCROLLEX *const s);


#ifdef __DEBUG__
static TLSCROLLEX *storeSCROLL (TSCROLLCONTAINER *src, TLSCROLLEX *scroll);
static int removeSCROLL (TSCROLLCONTAINER *src, TLSCROLLEX *scroll);
#endif



int initScroll (THWD *hw)
{
#ifdef __DEBUG__
	TSCROLLCONTAINER *scr = (TSCROLLCONTAINER*)l_malloc(sizeof(TSCROLLCONTAINER));
	scr->list = NULL;
	scr->total = 0;
	hw->scrtree = (TSCROLLCONTAINER*)scr;
#endif
	return 1;
}

void closeScroll (THWD *hw)
{
#ifdef __DEBUG__
	TSCROLLCONTAINER *scr = (TSCROLLCONTAINER *)hw->scrtree;
	
	if (scr->list){
		unsigned int i;
		for (i=0; i < scr->total; i++){
			if (scr->list[i])
				deleteScroll(scr->list[i]);
		}
		l_free(scr->list);
	}
	l_free(scr);
#endif
}

int updateScroll (TLSCROLLEX *s)
{
	if (s->dir == SCR_LEFT)
		scrollLeft(s);
	else if (s->dir == SCR_RIGHT)
		scrollRight(s);
	else if (s->dir == SCR_UP)
		scrollUp(s);
	else if (s->dir == SCR_DOWN)
		scrollDown(s);

	if (s->flags&SCR_AUTOREFRESH)
		refreshFrameArea(s->clientFrm, s->desRect->x1, s->desRect->y1, s->desRect->x2, s->desRect->y2);
	return 1;
}

TLSCROLLEX *newScroll (TFRAME *src, TFRAME *client)
{
	TLSCROLLEX *s = (TLSCROLLEX*)l_calloc(1, sizeof(TLSCROLLEX));
	if (!s)
		return NULL;
	
	s->srcRect = (TLPOINTEX*)l_calloc(1, sizeof(TLPOINTEX));
	if (!s->srcRect){
		l_free(s);
		return NULL;
	}

	s->desRect = (TLPOINTEX*)l_calloc(1, sizeof(TLPOINTEX));
	if (!s->desRect){
		l_free(s->srcRect);
		l_free(s);
		return NULL;
	}

	if (!(s->desFrm = cloneFrame(client))){
		l_free(s->desRect);
		l_free(s->srcRect);
		l_free(s);
		return NULL;
	}

	s->clientFrm = client;
	s->srcFrm = src;
	s->desRect->x1 = 0;
	s->desRect->y1 = 0;
	s->desRect->x2 = s->desFrm->width-1;
	s->desRect->y2 = s->desFrm->height-1;
	s->srcRect->x1 = 0;
	s->srcRect->y1 = 0;
	s->srcRect->x2 = src->width-1;
	s->srcRect->y2 = src->height-1;
	s->dir = SCR_LEFT;				// default right to left
	s->loopGapWidth = 10;			// number of blank pixels between scroll repeats
	s->startPos = 0;
	s->pos = 0;
	s->flags = SCR_LOOP;// | SCR_AUTOREFRESH;
	s->hw = src->hw;
#ifdef __DEBUG__	
	storeSCROLL(s->hw->scrtree, s);
#endif
	return s;
}

#ifdef __DEBUG__
static TLSCROLLEX *storeSCROLL (TSCROLLCONTAINER *scr, TLSCROLLEX *scroll)
{
	if (scroll == NULL)
		return NULL;
	
	if (scr->list == NULL){
		scr->list = (TLSCROLLEX**)l_calloc(1, sizeof(TLSCROLLEX*));
		if (scr->list == NULL) return NULL;
		scr->total = 1;
	}
	unsigned int i;
	for (i=0; i < scr->total; i++){
		if (scr->list[i] == NULL){
			scr->list[i] = scroll;
			return scroll;
		}
	}
	if ((scr->list=(TLSCROLLEX**)l_realloc(scr->list,++scr->total*sizeof(TLSCROLLEX*))))
		scr->list[scr->total-1] = scroll;
		
	return scroll;
}

static int removeSCROLL (TSCROLLCONTAINER *scr, TLSCROLLEX *scroll)
{
	int found = 0;
	if (scr->list){
		unsigned int i;
		for (i=0; i < scr->total; i++){
			if (scroll == scr->list[i]){
				scr->list[i] = NULL;
				found++;
			}
		}
	}
	return found;
}
#endif

void deleteScroll (TLSCROLLEX *scroll)
{
#ifdef __DEBUG__
	removeSCROLL(scroll->hw->scrtree, scroll);
#endif
	deleteFrame(scroll->desFrm);
	l_free(scroll->srcRect);
	l_free(scroll->desRect);
	l_free(scroll);
	return ;

}

static INLINE void scrollLeft (TLSCROLLEX *const s)
{
	if (s->pos < (1+s->srcRect->x2 - s->srcRect->x1)){
		moveLeft(s, LMOV_BIN);
		copyLeft(s);
		scrCopyToClient(s);
		s->pos++;
	}else{
		moveLeft(s, LMOV_CLEAR);
		scrCopyToClient(s);
		if (s->flags&SCR_LOOP){
			if (++s->pos > (1+s->srcRect->x2 - s->srcRect->x1)+s->loopGapWidth)
				s->pos = s->startPos;
		}
	}
}

static INLINE void scrollRight (TLSCROLLEX *const s)
{
	if (s->pos < (1+s->srcRect->x2 - s->srcRect->x1)){
		moveRight(s, LMOV_BIN);
		copyRight(s);
		scrCopyToClient(s);
		s->pos++;
	}else{
		moveRight(s, LMOV_CLEAR);
		scrCopyToClient(s);
		if (s->flags&SCR_LOOP){
			if (++s->pos > (1+s->srcRect->x2 - s->srcRect->x1)+s->loopGapWidth)
				s->pos = s->startPos;
		}
	}
}

static INLINE void scrollUp (TLSCROLLEX *const s)
{
	if (s->pos < (1+s->srcRect->y2 - s->srcRect->y1)){
		moveUp(s, LMOV_BIN);
		copyUp(s);
		scrCopyToClient(s);
		s->pos++;
	}else{
		moveUp(s, LMOV_CLEAR);
		scrCopyToClient(s);
		if (s->flags&SCR_LOOP){
			if (++s->pos > (1+s->srcRect->y2 - s->srcRect->y1)+s->loopGapWidth)
				s->pos = s->startPos;
		}
	}
}

static INLINE void scrollDown (TLSCROLLEX *const s)
{
	if (s->pos < (s->srcRect->y2 - s->srcRect->y1)){
		moveDown(s, LMOV_BIN);
		copyDown(s);
		scrCopyToClient(s);
		s->pos++;
	}else{
		moveDown(s, LMOV_CLEAR);
		scrCopyToClient(s);
		if (s->flags&SCR_LOOP){
			if (++s->pos > (s->srcRect->y2 - s->srcRect->y1)+s->loopGapWidth)
				s->pos = s->startPos;
		}
	}
}

static INLINE void moveLeft (const TLSCROLLEX *const s, const int mode)
{
	moveArea(s->desFrm, s->desRect->x1, s->desRect->y1, s->desRect->x2, s->desRect->y2, 1, mode, LMOV_LEFT);
}

static INLINE void moveRight (const TLSCROLLEX *const s, const int mode)
{
	moveArea(s->desFrm, s->desRect->x1, s->desRect->y1, s->desRect->x2, s->desRect->y2, 1, mode, LMOV_RIGHT);
}

static INLINE void moveUp (const TLSCROLLEX *const s, const int mode)
{
	moveArea(s->desFrm, s->desRect->x1, s->desRect->y1, s->desRect->x2, s->desRect->y2, 1, mode, LMOV_UP);
}

static INLINE void moveDown (const TLSCROLLEX *const s, const int mode)
{
	moveArea(s->desFrm, s->desRect->x1, s->desRect->y1, s->desRect->x2, s->desRect->y2, 1, mode, LMOV_DOWN);
}

static INLINE void copyLeft (const TLSCROLLEX *const s)
{
	TLPOINTEX m;
	minRect(&m, s);
	copyArea(s->srcFrm, s->desFrm, s->desRect->x2, s->desRect->y1, m.x1+s->pos, m.y1, m.x1+s->pos, m.y2);
}

static INLINE void copyRight (const TLSCROLLEX *const s)
{
	TLPOINTEX m;
	minRect(&m, s);
	m.y2 = s->srcRect->y2;
	copyArea(s->srcFrm, s->desFrm, s->desRect->x1, s->desRect->y1, m.x2-s->pos-1, m.y1, m.x2-s->pos, m.y2);
}

static INLINE void copyUp (const TLSCROLLEX *const s)
{
	TLPOINTEX m;
	minRect(&m, s);
	copyArea(s->srcFrm, s->desFrm, s->desRect->x1, s->desRect->y2, m.x1, m.y1+s->pos, m.x2, m.y1+s->pos+1);
}

static INLINE void copyDown (const TLSCROLLEX *const s)
{
	TLPOINTEX m;
	minRect(&m, s);
	m.y2 = s->srcRect->y2;
	copyArea(s->srcFrm, s->desFrm, s->desRect->x1, s->desRect->y1, m.x1, m.y2 - s->pos-1, m.x2, m.y2 - s->pos);
}

static int _MIN (int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

static INLINE void minRect (TLPOINTEX *const m, const TLSCROLLEX *const scroll)
{
	m->x1 = scroll->srcRect->x1;
	m->y1 = scroll->srcRect->y1;
	m->x2 = _MIN(_MIN(scroll->srcRect->x2 - scroll->srcRect->x1, scroll->desRect->x2 - scroll->desRect->x1), scroll->desFrm->width-1);
	m->y2 = _MIN(_MIN(scroll->srcRect->y2 - scroll->srcRect->y1, scroll->desRect->y2 - scroll->desRect->y1), scroll->desFrm->height-1);
}

int scrCopyToClient (const TLSCROLLEX *s)
{
	return copyArea(s->desFrm, s->clientFrm, s->desRect->x1, s->desRect->y1, s->desRect->x1, s->desRect->y1, s->desRect->x2, s->desRect->y2);
}

#else

int initScroll (THWD *hw){return 1;}
void closeScroll (THWD *hw){return;}

#endif
