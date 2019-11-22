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
#include "apilock.h"
#include "pixel.h"
#include "utils.h"
#include "frame.h"
#include "copy.h"
#include "draw.h"
#include "misc.h"


void *createFrameTree ()
{
	return framecNew();
}


void dumpFrameCacheDetail (void *flist)
{
#ifdef __DEBUG__
	TFRAMECACHE *frmc = (TFRAMECACHE*)flist;
	mylog("frames created: %i\nframes deleted: %i\n", frmc->frameCount, frmc->frameDeleteCount);
#endif
}


void freeFrameTree (void *flist)
{
	TFRAMECACHE *frmc = (TFRAMECACHE*)flist;
	dumpFrameCacheDetail(frmc);
#ifdef __DEBUG__
	mylog("frames rescued: %i\n", frameGetTotal(frmc));
#endif
	framecDelete(frmc);
}

static int registerFrame (TFRAMECACHE *frmc, TFRAME *frame, const int gid)
{
	if (frameAdd(frmc, frame, gid)){
#ifdef __DEBUG__
		frmc->frameCount++;
#endif
		return 1;
	}
	return 0;
}

void unregisterFrameGroup (THWD *hw, const int gid)
{
	frameDeleteAllByGroup((TFRAMECACHE*)hw->flist, gid);
}

TFRAME *newFrame (THWD *hw, const int width, const int height, ubyte bpp)
{
	if (bpp > LFRM_BPP_32A){
		bpp = LFRM_BPP_32A;
		mylog("libmylcd: newFrame(): invalid BPP specified, defaulting to LFRM_BPP_32A\n");
	}
	TFRAME *frame = _newFrame(hw, width, height, 1, bpp);
	//if (frame) clearFrame(frame);
	return frame;
}

int resizeFrame (TFRAME *frm, int width, int height, const int mode)
{
	width = MAX(MINFRAMESIZEW, width);
	height = MAX(MINFRAMESIZEH, height);
	if (frm->width == width && frm->height == height){
		//if (!mode) clearFrame(frm);
		return 2;
	}
	return _resizeFrame(frm, width, height, mode);
}

int _resizeFrame (TFRAME *frm, const int width, const int height, const int keepdata)
{
	TFRAME *tmp = _newFrame(frm->hw, width, height, frm->groupId, frm->bpp);
	if (tmp == NULL) return 0;

	if (keepdata){
		if (frm->frameSize == tmp->frameSize){
			void *top = tmp->pixels;
			void *fromp = frm->pixels;
			size_t framesize = frm->frameSize;
			l_memcpy(top, fromp, framesize);
			//l_memcpy(tmp->pixels, frm->pixels, frm->frameSize);
		}else{
#if 0
			copyArea(frm,tmp,0,0,0,0,MIN(frm->width-1,tmp->width-1), MIN(frm->height-1,tmp->height-1));
#else
			const int pitch = MIN(frm->pitch, tmp->pitch);
			const int height = MIN(frm->height, tmp->height);
		
			for (int y = 0; y < height; y++){
				void *fpixels = lGetPixelAddress(frm, 0, y);
				void *tpixels = lGetPixelAddress(tmp, 0, y);
				l_memcpy(tpixels, fpixels, pitch);
			}
#endif
		}
	}else{
		//clearFrame(tmp);
	}

	frm->height = tmp->height;
	frm->width = tmp->width;
	frm->bpp = tmp->bpp;
	frm->pitch = tmp->pitch;
	frm->frameSize = tmp->frameSize;

	ubyte *pixels = frm->pixels;
	frm->pixels = tmp->pixels;
	tmp->pixels = pixels;
	deleteFrame(tmp);
	return 1;
}

void deleteFrame (TFRAME *frame)
{
	frameDeleteDelink((TFRAMECACHE*)frame->hw->flist, frame);
}

void deleteFrames (THWD *hw, const int groupId)
{
	unregisterFrameGroup(hw, groupId);
}

int clearFrameClr (TFRAME *frm, int colour)
{
#if 0
	return drawRectangleFilled(frm, 0, 0, frm->width-1, frm->height-1, colour);
#else

	int i = frm->width * frm->height;

	switch(frm->bpp){
	  case LFRM_BPP_32:
	  case LFRM_BPP_32A:{
		uint32_t *pixels = l_getPixelAddress(frm, 0, 0);
		while(i--) pixels[i] = colour;
	  }
	  break;
	  case LFRM_BPP_24:{
	  	colour &= 0xFFFFFF;
		TRGB *pixels = l_getPixelAddress(frm, 0, 0);
		while(i--) pixels[i] = *(TRGB*)&colour;
	  }
	  break;
#if (!__BUILD_2432BITONLY_SUPPORT__)
	  case LFRM_BPP_15:
	  case LFRM_BPP_16:{
	  	colour &= 0xFFFF;
		uint16_t *pixels = l_getPixelAddress(frm, 0, 0);
		while(i--) pixels[i] = colour;		
	  }
	  break;
	  case LFRM_BPP_12:{
	  	colour &= 0xFFF;
		uint16_t *pixels = l_getPixelAddress(frm, 0, 0);
		while(i--) pixels[i] = colour;		
	  }
	  break;
	  case LFRM_BPP_8:{
	  	colour &= 0xFF;
		uint8_t *pixels = l_getPixelAddress(frm, 0, 0);
		while(i--) pixels[i] = colour;		
	  }
	  break;
	  case LFRM_BPP_1:{
	  	i /= 8;		/* 8 pixels per byte*/
	  	colour = ((colour>0)&0x01)*255;
		uint8_t *pixels = l_getPixelAddress(frm, 0, 0);
		while(i--) pixels[i] = colour;		
	  }	  
	  break;
#endif
	  default:
	  	return drawRectangleFilled(frm, 0, 0, frm->width-1, frm->height-1, colour);
	};

	return 1;
	
#endif
}

int clearFrame (TFRAME *frm)
{
	const uint32_t backi = getBackgroundColour(frm->hw);
	const char *back = (char*)&backi;

	if (frm->bpp == LFRM_BPP_32A){
		uint32_t *pixels = (uint32_t*)frm->pixels;
		int i = frm->width*frm->height;
		const int colour = /*0xFF000000|*/(backi&0xFFFFFF);
		while(i--) pixels[i] = colour;

	}else if (back[0] == back[1] && back[0] == back[2] && back[0] == back[3]){
		l_memset(frm->pixels, back[0], frm->frameSize);
	}else{
		clearFrameClr(frm, backi);
	}
	return 1;
}

TFRAME *_newFrame (THWD *hw, const int width, const int height, const int gid, const ubyte bpp)
{
	TFRAME *frm = (TFRAME*)l_malloc(sizeof(TFRAME));
	if (frm == NULL)
		return NULL;
	
	frm->pops = (TPIXELPRIMITVES*)l_malloc(sizeof(TPIXELPRIMITVES));
	if (frm->pops == NULL){
		l_free(frm);
		return NULL;
	}
	frm->hw = hw;
	frm->height = MAX(MINFRAMESIZEH, height);
	frm->width = MAX(MINFRAMESIZEW, width);
	frm->bpp = bpp;
	frm->pitch = calcPitch(frm->width, frm->bpp);
	frm->frameSize = calcFrameSize(frm->width, frm->height, frm->bpp);
	frm->style = LSP_SET;
	frm->udata_ptr = NULL;
	frm->udata_int = 0;
	assignPixelPrimitives(frm->pops, frm->bpp);
	
	if ((frm->pixels=(ubyte*)l_calloc(1, frm->frameSize + sizeof(int)))){
		if (registerFrame(hw->flist, frm, gid))
			return frm;
		l_free(frm->pixels);
	}
	l_free(frm->pops);
	l_free(frm);
	return NULL;
}

TFRAME *cloneFrame (TFRAME *src)
{
	TFRAME *des = _newFrame(src->hw, src->width, src->height, 1, src->bpp);
	if (des){
		l_memcpy(des->pixels, src->pixels, src->frameSize);
		des->style = src->style;
		des->udata_ptr = src->udata_ptr;
		des->udata_int = src->udata_int;
		return des;
	}else{
		return NULL;
	}
}

TFRAME *cloneFrameEx (TFRAME *current, TFRAME *src)
{
	if (current == NULL){
		return cloneFrame(src);
	}else if (src->width == current->width && src->height == current->height && src->bpp == current->bpp){
		l_memcpy(current->pixels, src->pixels, src->frameSize);
		current->style = src->style;
		current->udata_ptr = src->udata_ptr;
		current->udata_int = src->udata_int;
		return current;
	}else{
		deleteFrame(current);
		return cloneFrame(src);
	}
}

// return number of bytes required per row
int calcPitch (const int width, const ubyte bpp)
{
	switch (bpp){
#if (!__BUILD_2432BITONLY_SUPPORT__)
	  case LFRM_BPP_1:
		if (!(width/8))
			return 1;
		else if (width&7)
			return (width/8)+1;
		else
			return (width/8);
	  case LFRM_BPP_8: return width;
	  case LFRM_BPP_12: return width<<1;
	  case LFRM_BPP_15: return width<<1;
#endif
	  case LFRM_BPP_16: return width<<1;
	  case LFRM_BPP_24: return width*3;
	  case LFRM_BPP_32: return width<<2;
	  case LFRM_BPP_32A: return width<<2;
	}

	mylog("calcPitch(): invalid bpp. %i %i\n", width, bpp);
	return 0;
}

static int padTo4 (const int width)
{
	if (width&3)
		return ((width>>2)<<2) + 4;
	else 
		return width;
}

int calcFrameSize (const int width, const int height, const ubyte bpp)
{
	//printf("calcFrameSize(): %i %i %i\n", width, height, bpp);
	return height*padTo4(calcPitch(width, bpp));
}

