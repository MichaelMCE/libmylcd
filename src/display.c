
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

#include "mylcd.h"
#include "memory.h"
#include "utils.h"
#include "sync.h"
#include "device.h"
#include "frame.h"
#include "pixel.h"
#include "copy.h"
#include "display.h"
#include "convert.h"
#include "lmath.h"
#include "misc.h"

static TDRIVER *frameToDevice1 (TFRAME *frm);
static TDRIVER *frameRectToDevice(TFRAME *frm, TRECT *rect);
static void refreshFrameAsync (TASFCONTROL *ctrl, TFRAME *frame, int swap);

static TDRIVER *verifyRectBoundToDevice (TFRAME *frm, TRECT *srcRect, int index);
static TASFCONTROL *createAsyncControl (TASFCONTROL *ctrl);
static TASFCONTROL *allocAsyncControl (TDRIVER *drv);



int initDisplay (THWD *hw)
{
	hw->render->framesSubmitted = 0;
	hw->render->framesRendered = 0;
	return 1;
}

void closeDisplay (THWD *hw)
{
#ifdef __DEBUG__
	mylog("frames submitted: %i\n", hw->render->framesSubmitted);
	mylog("frames rendered: %i\n", hw->render->framesRendered);
#endif
}

int clearDisplay (THWD *hw)
{
	int ret = 0;
	unsigned int i = hw->devcount;
	while(i--){
		if (hw->devlist[i]){
			TDISPLAYDRIVER *dd = hw->devlist[i]->dd;
			if (dd->status == LDRV_READY)
				ret = dd->clear(hw->devlist[i]);
		}
	}
	return ret;
}

int update (const THWD *hw, const void *buffer, const size_t bufferLen)
{
	int ret = 0;
	unsigned int i = hw->devcount;

	while(i--){
		if (hw->devlist[i]){
			TDISPLAYDRIVER *dd = hw->devlist[i]->dd;

			if (dd->status == LDRV_READY){
				if (bufferLen <= dd->temp[dd->bpp]->frameSize){
					l_memcpy(dd->temp[dd->bpp]->pixels, buffer, bufferLen);
					ret = dd->refresh(hw->devlist[i], dd->temp[dd->bpp]);
#ifdef __DEBUG__
					if (ret > 0) hw->render->framesRendered++;
#endif
				}else{
					mylog("update(): invalid input buffer length. is:%i, expecting:%i\n",\
					  (int)bufferLen, (int)dd->temp[dd->bpp]->frameSize);
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int refreshAsync (TFRAME *frm, const int swap)
{
	TDRIVER *drv = frameToDevice1(frm);
	if (drv){
		if (drv->ctrl == NULL){
			if (allocAsyncControl(drv))
				createAsyncControl((TASFCONTROL*)drv->ctrl);
			else
				return -1;
		}
		refreshFrameAsync((TASFCONTROL*)drv->ctrl, frm, swap);
		return 1;
	}
	return 0;

}

int setDisplayOption (THWD *hw, lDISPLAY did, int option, intptr_t *value)
{
	int ret = 0;
	if (hw){
		TDRIVER *drv = displayIDToDriver(hw, did);
		if (drv){
			if (drv->dd){
				if (drv->dd->setOption){
					if (drv->dd->optTotal){
						if (drv->dd->opt)
							ret = drv->dd->setOption(drv, option, value);
					}
				}
			}
		}
	}
	return ret;
}

int getDisplayOption (THWD *hw, lDISPLAY did, int option, intptr_t *value)
{
	int ret = 0;
	if (hw && value){
		TDRIVER *drv = displayIDToDriver(hw, did);
		if (drv){
			if (drv->dd){
				if (drv->dd->getOption){
					if (drv->dd->optTotal){
						if (drv->dd->opt)
							ret = drv->dd->getOption(drv,option,value);
					}
				}
			}
		}
	}
	return ret;
}

int pauseDisplay (THWD *hw, lDISPLAY did)
{
	int ret = 0;
	TDRIVER *drv = displayIDToDriver(hw, did);
	if (drv){
		if (drv->dd->status == LDRV_READY){
			drv->dd->status = LDRV_DISENGAGED;
			ret = 1;
		}
	}
	return ret;
}

int resumeDisplay (THWD *hw, lDISPLAY did)
{
	int ret = 0;
	TDRIVER *drv = displayIDToDriver(hw, did);
	if (drv){
		if (drv->dd->status == LDRV_DISENGAGED){
			drv->dd->status = LDRV_READY;
			ret = 1;
		}
	}
	return ret;
}

int refreshFrameArea (TFRAME *frm, int left, int top, int right, int btm)
{

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right > frm->width-1) right = frm->width-1;
	if (btm > frm->height-1) btm = frm->height-1;

	TRECT rect;
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.btm = btm;
	TDRIVER *drv = NULL;

	for (unsigned int i = 0; i < frm->hw->devcount; i++){
		if ((drv=verifyRectBoundToDevice(frm, &rect, i))){
			//convertFrameArea(frm, drv->dd->temp[frm->bpp], 0, 0, drv->rect->left, drv->rect->top, drv->rect->right, drv->rect->btm);
			
			if (drv->dd->refreshArea(drv, frm, left, top, right, btm)){
			//if (drv->dd->refreshArea(drv, drv->dd->temp[frm->bpp], l_abs(drv->rect->left - left), l_abs(top - drv->rect->top),
			//  l_abs(drv->rect->right - left)+1, l_abs((top - drv->rect->top)+ (btm - top)))){
#ifdef __DEBUG__
				frm->hw->render->framesRendered++;
#endif
			}
		}
	}
	return 1;
}

static TFRAME *swapframe (TFRAME *current, TFRAME *src, const int mode)
{
	if (current == NULL){
		return cloneFrame(src);
	}else if (src->width == current->width && src->height == current->height && src->bpp == current->bpp){
		if (!mode){
			l_memcpy(current->pixels, src->pixels, src->frameSize);
		}else{
			void *pixels = current->pixels;
			current->pixels = src->pixels;
			src->pixels = pixels;
		}
		current->style = src->style;
		current->udata_ptr = src->udata_ptr;
		current->udata_int = src->udata_int;
		return current;
	}else{
		deleteFrame(current);
		return cloneFrame(src);
	}
}

static void refreshFrameAsync (TASFCONTROL *ctrl, TFRAME *frame, const int mode)
{
	lock(&ctrl->tsc);
	ctrl->frameAsync1 = swapframe(ctrl->frameAsync1, frame, mode);
	unlock(&ctrl->tsc);
	setSignal(&ctrl->tsc);
}

int refreshFrame (TFRAME *frm)
{
	TRECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = frm->width-1;
	rect.btm = frm->height-1;
	TDRIVER *drv = NULL;
	int ret = 0;

	for (unsigned int i = 0; i < frm->hw->devcount; i++){
		if ((drv=verifyRectBoundToDevice(frm, &rect, i))){
			TDISPLAYDRIVER *dd = drv->dd;
			convertFrameArea(frm, dd->temp[frm->bpp], 0, 0, drv->rect->left, drv->rect->top, drv->rect->right, drv->rect->btm);
			ret = drv->dd->refresh(drv, dd->temp[frm->bpp]);
#ifdef __DEBUG__
			if (ret > 0) frm->hw->render->framesRendered++;
#endif
		}
	}
	return ret;
}

unsigned int __stdcall frame_dispatcher (TASFCONTROL *ctrl)
{
	//printf("frame_dispatcher %i\n", (int)GetCurrentThreadId());

	mylog("frame_dispatcher(): entered, locking\n");
	lock(&ctrl->exitLock);
	mylog("frame_dispatcher(): got lock, entering loop\n");

	while(ctrl->thrdState){
		if (!waitForSignal(&ctrl->tsc, INFINITE)){
			if (!ctrl->thrdState) break;
			resetSignal(&ctrl->tsc);
			lock(&ctrl->tsc);
			ctrl->frameAsync2 = cloneFrameEx(ctrl->frameAsync2, ctrl->frameAsync1);
			unlock(&ctrl->tsc);
			if (ctrl->thrdState)
				refreshFrame(ctrl->frameAsync2);
		}
	}
	mylog("frame_dispatcher(): unlocking for exit..\n");
	unlock(&ctrl->exitLock);
	mylog("frame_dispatcher(): unlocked, exited\n");
	return 1;
}

void removeAsyncControl (TASFCONTROL *ctrl)
{
	//if (ctrl){
		ctrl->thrdState = 0;
		setSignal(&ctrl->tsc);
		mylog("removeAsyncControl():acquring thread lock\n");
		lock(&ctrl->exitLock);
		unlock(&ctrl->exitLock);
		mylog("removeAsyncControl():got thread lock\n");
		lock_delete(&ctrl->exitLock);
		lock_delete(&ctrl->tsc);
		deleteEvent(&ctrl->tsc);
		deleteFrame(ctrl->frameAsync1);
		deleteFrame(ctrl->frameAsync2);
		//joinThread(&ctrl->tsc);
		closeThreadHandle(&ctrl->tsc);
		l_free(ctrl);
	//}
}

static TASFCONTROL *createAsyncControl (TASFCONTROL *ctrl)
{
	if (ctrl != NULL){
		ctrl->thrdState = 1;
		createEvent(&ctrl->tsc);
		lock_create(&ctrl->tsc);
		lock_create(&ctrl->exitLock);
		newThread(&ctrl->tsc, frame_dispatcher, ctrl);
	}
	return ctrl;
}

static TASFCONTROL *allocAsyncControl (TDRIVER *drv)
{
	if (drv != NULL){
		drv->ctrl = l_calloc(1, sizeof(TASFCONTROL));
		if (drv->ctrl == NULL){
			mylog("allocSyncControl(): unable to alloc memory\n");
			return NULL;
		}else{
			return (TASFCONTROL*)drv->ctrl;
		}
	}else{
		mylog("allocSyncControl(): drv == NULL\n");
		return NULL;
	}
}

// find driver which has claim to src rect within frame
// return driver handle when source rect overlaps destination rect
// returns NULL if no overlap
static TDRIVER *frameRectToDevice (TFRAME *frm, TRECT *srcRect)
{
	TDRIVER *drv;

	for (unsigned int i = 0; i < frm->hw->devcount; i++){
		if (frm->hw->devlist[i]){
			if ((drv=verifyRectBoundToDevice(frm, srcRect, i)))
				return drv;
		}
	}
	return NULL;
}

static TDRIVER *frameToDevice1 (TFRAME *frm)
{
	TRECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = frm->width-1;
	rect.btm = frm->height-1;
	return frameRectToDevice(frm, &rect);
}

static TDRIVER *verifyRectBoundToDevice (TFRAME *frm, TRECT *srcRect, int index)
{

	if (frm->hw->devlist[index]){
		TRECT *desRect = frm->hw->devlist[index]->rect;

		// check if update area is overlapping on the left side of display rect
		if ((srcRect->left <= desRect->left) && (srcRect->right >= desRect->left)){
			if ((srcRect->top <= desRect->top) && (srcRect->btm >= desRect->top))
				return frm->hw->devlist[index];
			else if((srcRect->top >= desRect->top) && (srcRect->top <= desRect->btm))
		    	return frm->hw->devlist[index];
		}else if (((srcRect->right >= desRect->right) && (srcRect->left <= desRect->right)) || ((srcRect->right <= desRect->right) && (srcRect->right >= desRect->left))){
			if ((srcRect->top <= desRect->top) && (srcRect->btm >= desRect->top))
				return frm->hw->devlist[index];
			else if((srcRect->top >= desRect->top) && (srcRect->top <= desRect->btm))
		    	return frm->hw->devlist[index];
		}
	}
	return NULL;
}

