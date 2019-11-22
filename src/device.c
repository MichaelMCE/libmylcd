
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
#include "frame.h"
#include "device.h"
#include "lstring.h"
#include "display.h"
#include "lmath.h"
#include "pixel.h"
#include "copy.h"

#include "sync.h"
#include "misc.h"

static TPORTDRIVER *portNameToDevice (TREGISTEREDDRIVERS *rd, const char *port);
static TDISPLAYDRIVER *displayNameToDevice (TREGISTEREDDRIVERS *rd, const char *display);
static TREGDRV *enumeratePortDriverBegin (TREGISTEREDDRIVERS *rd);
static TREGDRV *enumerateDisplayDriverBegin (TREGISTEREDDRIVERS *rd);

static lDISPLAY displayDriverNameToID (THWD *hw, const char *display);
static lDISPLAY portDriverNameToID (THWD *hw, const char *display);
static void initDefaultCaps (THWD *hw);


TREGISTEREDDRIVERS *createDeviceTree ()
{
	TREGISTEREDDRIVERS *rd = l_calloc(1, sizeof(TREGISTEREDDRIVERS));
	if (rd){
		rd->dd = (TDISPLAYDRIVER**)l_calloc(1, sizeof(TDISPLAYDRIVER*));
		rd->pd = (TPORTDRIVER**)l_calloc(1, sizeof(TPORTDRIVER*));
		rd->dtotal = 0;
		rd->ptotal = 0;
	}
	return rd;
}

void freeDeviceTree (TREGISTEREDDRIVERS *rd)
{
	int i = rd->dtotal;
	while(i--){
		if (rd->dd[i]){
			if (rd->dd[i]->opt)
				l_free(rd->dd[i]->opt);
			if (rd->dd[i])
				l_free(rd->dd[i]);
		}
	}
	if (rd->dd)
		l_free(rd->dd);
		
	i = rd->ptotal;
	while(i--){
		if (rd->pd[i]){
			if (rd->pd[i]->opt)
				l_free(rd->pd[i]->opt);
			if (rd->pd[i])
				l_free(rd->pd[i]);
		}
	}
	if (rd->pd)
		l_free(rd->pd);
	l_free(rd);
}

TREGDRV *enumerateDriversBegin (THWD *hw, int edrv)
{
	TREGDRV *drv = NULL;

	if (edrv == LDRV_DISPLAY){
		drv = enumerateDisplayDriverBegin(hw->dtree);
		drv->hw = hw;
	}else if (edrv == LDRV_PORT){
		drv = enumeratePortDriverBegin(hw->dtree);
		drv->hw = hw;
	}
	return drv;
}

void enumerateDriverEnd (TREGDRV *d)
{	
	l_free(d);
}

int enumerateDriverNext (TREGDRV *d)
{
	int ret = 0;
	TREGISTEREDDRIVERS *rd = (TREGISTEREDDRIVERS*)d->hw->dtree;
		
	if (d->type == LDRV_DISPLAY){
		if (d->index < rd->dtotal-1){
			d->index++;
			strncpy(d->name, rd->dd[d->index]->name, lMaxDriverNameLength+1);
			strncpy(d->comment, rd->dd[d->index]->comment, lMaxDriverCommentLength+1);
			ret = 1;
		}
	}else if (d->type == LDRV_PORT){
		if (d->index < rd->ptotal-1){
			d->index++;
			strncpy(d->name, rd->pd[d->index]->name, lMaxDriverNameLength+1);
			strncpy(d->comment, rd->pd[d->index]->comment, lMaxDriverCommentLength+1);
			ret = 1;
		}	
	}
	return ret;
}

lDISPLAY driverNameToID (THWD *hw, const char *name, int ldrv_type)
{
	lDISPLAY ret = 0;
	if (ldrv_type == LDRV_DISPLAY)
		ret = displayDriverNameToID(hw, name);
	else if (ldrv_type == LDRV_PORT)
		ret = portDriverNameToID(hw, name);
	return ret;
}

TDRIVER *displayIDToDriver (THWD *hw, lDISPLAY did)
{
	TDRIVER *drv = NULL;
	if (did && did <= hw->devcount){
		if (hw->devlist[--did])
			drv = hw->devlist[did];
	}
	return drv;
}

int setCapabilities (THWD *hw, const unsigned int flag, const int value)
{
	if (flag && (flag < CAPS_TOTAL))
		return (hw->caps[flag] = value);
	else
		return -1;
}

int getCapabilities (THWD *hw, const unsigned int flag)
{
	if (flag && (flag < CAPS_TOTAL))
		return (hw->caps[flag]);
	else
		return -1;
}

lDISPLAY selectDevice (THWD *hw, const char *displayd, const char *portd, const int width, const int height, int bpp, const int data, TRECT *rect)
{
	
	TPORTDRIVER *pd = NULL;
	TDISPLAYDRIVER *dd = NULL;

	if (bpp < LFRM_BPP_1 || bpp > LFRM_BPP_32A){
		bpp = LFRM_BPP_32A;
		mylog("libmylcd: invalid BPP specified, defaulting to LFRM_BPP_32A\n");
	}
						
	TPORTDRIVER *portdrv = portNameToDevice(hw->dtree, portd);
	if (!portdrv){
		mylog("libmylcd: '%s' unknown or not compiled in to library.\n", portd);
		return 0;
	}
	
	TDISPLAYDRIVER *displaydrv = displayNameToDevice(hw->dtree, displayd);
	if (!displaydrv){
		mylog("libmylcd: '%s' unknown or not compiled in to library.\n", displayd);
		return 0;
	}

	int idev = hw->devcount;
	hw->devlist = (TDRIVER**)l_realloc(hw->devlist, (idev+1) * sizeof(TDRIVER**));
	if (hw->devlist==NULL){
		mylog("libmylcd: l_realloc() returned NULL\n");
		return 0;
	}

	if ((hw->devlist[idev]=(TDRIVER*)l_calloc(1,sizeof(TDRIVER)))){
		if ((pd=(TPORTDRIVER*)l_calloc(1,sizeof(TPORTDRIVER)))){
			if ((dd=(TDISPLAYDRIVER*)l_calloc(1,sizeof(TDISPLAYDRIVER)))){
				l_memcpy(pd, portdrv, sizeof(TPORTDRIVER));
				l_memcpy(dd, displaydrv, sizeof(TDISPLAYDRIVER));

				dd->opt = (void*)l_calloc(displaydrv->optTotal+1, sizeof(intptr_t*));
				for (int i = 0; i < displaydrv->optTotal; i++)
					dd->opt[i] = displaydrv->opt[i];
					
				pd->opt = (void*)l_calloc(portdrv->optTotal+1, sizeof(intptr_t*));
				for (int i = 0; i < portdrv->optTotal; i++)
					pd->opt[i] = portdrv->opt[i];

				dd->hw = hw;
				dd->port = data;
				dd->WIDTH = width;
				dd->HEIGHT = height;
				dd->width = l_abs(MIN(width, 1+rect->right - rect->left));
				dd->height = l_abs(MIN(height, 1+rect->btm - rect->top));
				dd->bpp = bpp;
				dd->clr = 0;
				dd->currentColumn = 0;
				dd->currentRow = 0;
				dd->tmpGrpId = (intptr_t)dd;

#if (__BUILD_2432BITONLY_SUPPORT__)
				dd->temp[bpp] = _newFrame(hw, dd->width, dd->height, dd->tmpGrpId, bpp);
#else
				for (int i = 0; i < LFRM_TYPES; i++)
					dd->temp[i] = _newFrame(hw, dd->width, dd->height, dd->tmpGrpId, i);
#endif
					
				if (dd->temp[bpp]){
					if (pd->open(pd, dd->port)){
						hw->devlist[idev]->rect = rect;
						hw->devlist[idev]->pd = pd;
						hw->devlist[idev]->dd = dd;
						if (dd->open(hw->devlist[idev])){
							mylog("display:%i\t%s (%ix%i)\n\t\t%s\n",idev+1,dd->comment, dd->width, dd->height, pd->comment);
							hw->devcount++;
							return idev+1;
						}else{
							mylog("libmylcd: could not start device: '%s' (%ix%i)\n",dd->comment, dd->width, dd->height);
							//pd->close(pd);
						}
					}else{
						mylog("libmylcd: could not open driver: '%s'\n", pd->comment);
					}
					unregisterFrameGroup(hw, dd->tmpGrpId);
				}
				l_free(dd->opt);
				l_free(pd->opt);
			}
			l_free(hw->devlist[idev]);
			hw->devlist[idev] = NULL;
		}
		if (pd){
			l_free(pd);
			pd = NULL;
		}
		if (dd){
			l_free(dd);
			dd = NULL;
		}
	}
	return 0;
}

static lDISPLAY displayDriverNameToID (THWD *hw, const char *display)
{
	for (unsigned int i = 0; i < hw->devcount; i++){
		if (hw->devlist[i]){
			if (hw->devlist[i]->dd){
				if (hw->devlist[i]->dd->status != LDRV_CLOSED){
					if (!l_strcasecmp(hw->devlist[i]->dd->name, display))
						return i+1;
				}
			}
		}
	}
	return 0;
}

static lDISPLAY portDriverNameToID (THWD *hw, const char *display)
{
	for (unsigned int i = 0; i < hw->devcount; i++){
		if (hw->devlist[i]){
			if (hw->devlist[i]->pd){
				if (hw->devlist[i]->pd->status != LDRV_CLOSED){
					if (!l_strcasecmp(hw->devlist[i]->pd->name, display))
						return i+1;
				}
			}
		}
	}
	return 0;
}

static TDISPLAYDRIVER *displayNameToDevice (TREGISTEREDDRIVERS *rd, const char *display)
{
	for (unsigned int i = 0; i < rd->dtotal; i++){
		if (rd->dd[i]){
			if (!l_strcasecmp(rd->dd[i]->name, display))
				return rd->dd[i];
		}
	}
	return NULL;
}

static TPORTDRIVER *portNameToDevice (TREGISTEREDDRIVERS *rd, const char *port)
{
	for (unsigned int i = 0; i < rd->ptotal; i++){
		if (rd->pd[i]){
			if (!l_strcasecmp(rd->pd[i]->name, port))
				return rd->pd[i];
		}
	}
	return NULL;
}

static TREGDRV *enumerateDisplayDriverBegin (TREGISTEREDDRIVERS *rd)
{
	if (rd->dtotal){
		TREGDRV *d = (TREGDRV*)l_calloc(1,sizeof(TREGDRV));
		if (d){
			d->index = 0;
			d->total = rd->dtotal;
			d->type = LDRV_DISPLAY;
			l_strncpy(d->name, rd->dd[d->index]->name, lMaxDriverNameLength+1);
			l_strncpy(d->comment, rd->dd[d->index]->comment, lMaxDriverCommentLength+1);
			return d;
		}
	}
	return NULL;
}

static TREGDRV *enumeratePortDriverBegin (TREGISTEREDDRIVERS *rd)
{

	if (rd->ptotal){
		TREGDRV *d = (TREGDRV *)l_calloc(1,sizeof(TREGDRV));
		if (d){
			d->index = 0;
			d->total = rd->ptotal;
			d->type = LDRV_PORT;
			l_strncpy(d->name, rd->pd[d->index]->name, lMaxDriverNameLength+1);
			l_strncpy(d->comment, rd->pd[d->index]->comment, lMaxDriverCommentLength+1);
			return d;
		}
	}
	return NULL;
}

int registerDisplayDriver (TREGISTEREDDRIVERS *rd, TDISPLAYDRIVER *dd)
{
	
	int i = rd->dtotal; 
	if ((rd->dd = (TDISPLAYDRIVER **)l_realloc(rd->dd, (1+i) * sizeof(TDISPLAYDRIVER *)))){
		rd->dd[i] = (TDISPLAYDRIVER*)l_calloc(1, sizeof(TDISPLAYDRIVER));
		if (rd->dd[i]){
			l_strncpy(rd->dd[i]->name, dd->name, lMaxDriverNameLength+1);
			l_strncpy(rd->dd[i]->comment, dd->comment, lMaxDriverCommentLength+1);
				
			rd->dd[i]->optTotal = dd->optTotal;
			if (rd->dd[i]->optTotal){
				rd->dd[i]->opt = (void*)l_calloc(dd->optTotal, sizeof(intptr_t*));
				if (rd->dd[i]->opt == NULL){
					l_free(rd->dd[i]);
					rd->dd[i] = NULL;
					return 0;
				}
			}
			rd->dd[i]->open = dd->open;
			rd->dd[i]->close = dd->close;
			rd->dd[i]->clear = dd->clear;
			rd->dd[i]->refresh = dd->refresh;
			rd->dd[i]->refreshArea = dd->refreshArea;
			rd->dd[i]->getOption = dd->getOption;
			rd->dd[i]->setOption = dd->setOption;
			rd->dtotal++;
			return i+1;
		}
	}
	return 0;
}

int registerPortDriver (TREGISTEREDDRIVERS *rd, TPORTDRIVER *pd)
{
	
	int i = rd->ptotal;
	if ((rd->pd = (TPORTDRIVER **)l_realloc(rd->pd, (1+i) * sizeof(TPORTDRIVER*)))){
		rd->pd[i] = (TPORTDRIVER*)l_calloc(1,sizeof(TPORTDRIVER));
		if (rd->pd[i]){
			l_strncpy(rd->pd[i]->name, pd->name, lMaxDriverNameLength+1);
			l_strncpy(rd->pd[i]->comment, pd->comment, lMaxDriverCommentLength+1);

			rd->pd[i]->optTotal = pd->optTotal;
			if (rd->pd[i]->optTotal){
				rd->pd[i]->opt = (void*)l_calloc(pd->optTotal, sizeof(intptr_t*));
				if (rd->pd[i]->opt == NULL){
					l_free(rd->pd[i]);
					rd->pd[i] = NULL;
					return 0;
				}
			}

			rd->pd[i]->status = pd->status;
			rd->pd[i]->open = pd->open;
			rd->pd[i]->close = pd->close;
			rd->pd[i]->write8 = pd->write8;
			rd->pd[i]->write16 = pd->write16;
			rd->pd[i]->write32 = pd->write32;
			rd->pd[i]->flush = pd->flush;
			rd->pd[i]->read = pd->read;
			rd->pd[i]->writeBuffer = pd->writeBuffer;
			rd->pd[i]->getOption = pd->getOption;
			rd->pd[i]->setOption = pd->setOption;
			rd->ptotal++;
			return i+1;
		}
	}
	return 0;
}

static void closeDDriver (TDRIVER *drv)
{
	if (drv->ctrl){
		removeAsyncControl((TASFCONTROL*)drv->ctrl);
		drv->ctrl = NULL;
	}
	if (drv->dd->status != LDRV_CLOSED)
		drv->dd->close(drv);
	drv->dd->status = LDRV_CLOSED;
}

static void closePDriver (TDRIVER *drv)
{
	if (drv->pd->status != LDRV_CLOSED)
		drv->pd->close(drv->pd);
	drv->pd->status = LDRV_CLOSED;
}

int closeDeviceId (THWD *hw, const lDISPLAY did)
{
	int ret = 0;
	TDRIVER *drv = displayIDToDriver(hw, did);
	if (drv){
		closeDDriver(drv);
		closePDriver(drv);
		
		if (drv->dd->status == LDRV_CLOSED){
			unregisterFrameGroup(hw, drv->dd->tmpGrpId);
			
			if (drv->dd->opt)
				l_free(drv->dd->opt);
			l_free(drv->dd);
			drv->dd = NULL;
			
			if (drv->pd->opt)
				l_free(drv->pd->opt);
			if (drv->pd){
				l_free(drv->pd);
				drv->pd = NULL;
			}
			
			l_free(drv);
			hw->devlist[did-1] = NULL; // 'drv' points to 'hw->devlist[did-1]'
			ret = 1;
		}
	}
	return ret;
}

THWD *newHWDeviceDesc ()
{
	THWD *hw = (THWD*)l_calloc(1, sizeof(THWD));
	if (hw){
		hw->devlist = (TDRIVER**)l_calloc(1, sizeof(TDRIVER**));
		hw->paths = (THWDPATHS*)l_calloc(1, sizeof(THWDPATHS));
		hw->fonts = (TFONTTREES*)l_calloc(1, sizeof(TFONTTREES));
		hw->fonts->bmp = (TFONT*)l_calloc(1, sizeof(TFONT));
		hw->fonts->bdf = (TWFONT*)l_calloc(1, sizeof(TWFONT));
		hw->cmap = (TCMAPTREE*)l_calloc(1, sizeof(TCMAPTREE));
		hw->cmap->root = (TMAPTABLE*)l_calloc(1, sizeof(TMAPTABLE));
		hw->render = (TTRENDER*)l_calloc(1, sizeof(TTRENDER));
		hw->render->loc = (TLPRINTREGION*)l_calloc(1, sizeof(TLPRINTREGION));
		hw->devcount = 0;
		initDefaultCaps(hw);
	}
	return hw;
}

void freeHWDeviceDesc (THWD *hw)
{
	unsigned int i = hw->devcount;

	// send close signal to device then free its resources
	// close display devices before port drivers
	while(i--){
		if (hw->devlist[i])
			closeDDriver(hw->devlist[i]);
	}

	i = hw->devcount;
	while(i--){
		if (hw->devlist[i]){
			closePDriver(hw->devlist[i]);
			unregisterFrameGroup(hw, hw->devlist[i]->dd->tmpGrpId);
			
			if (hw->devlist[i]->dd){
				if (hw->devlist[i]->dd->opt)
					l_free(hw->devlist[i]->dd->opt);
				l_free(hw->devlist[i]->dd);
				hw->devlist[i]->dd = NULL;
			}
			if (hw->devlist[i]->pd){
				if (hw->devlist[i]->pd->opt)
					l_free(hw->devlist[i]->pd->opt);
				l_free(hw->devlist[i]->pd);
				hw->devlist[i]->pd = NULL;
			}
			l_free(hw->devlist[i]);
			hw->devlist[i] = NULL;
		}
	}
	if (hw->render){
		if (hw->render->loc){
			l_free(hw->render->loc);
			hw->render->loc = NULL;
		}
		l_free(hw->render);
		hw->render = NULL;
	}
	if (hw->paths){
		if (hw->paths->font){
			l_free(hw->paths->font);
			hw->paths->font = NULL;
		}
		if (hw->paths->cmap){
			l_free(hw->paths->cmap);
			hw->paths->cmap = NULL;
		}
		l_free(hw->paths);
		hw->paths = NULL;
	}
	if (hw->fonts){
		if (hw->fonts->bmp){
			l_free(hw->fonts->bmp);
			hw->fonts->bmp = NULL;
		}
		if (hw->fonts->bdf){
			l_free(hw->fonts->bdf);
			hw->fonts->bdf = NULL;
		}
		l_free(hw->fonts);
		hw->fonts = NULL;
	}
	if (hw->cmap){
		if (hw->cmap->root){
			l_free(hw->cmap->root);
			hw->cmap->root = NULL;
		}
		l_free(hw->cmap);
		hw->cmap = NULL;
	}
	
	l_free(hw->devlist);
	hw->devlist = NULL;
	hw->devcount = 0;
	l_free(hw);
}

int setDefaultDisplayOption (TREGISTEREDDRIVERS *rd, lDISPLAY did, int option, intptr_t value)
{

	if (did > 0 && did <= rd->dtotal){
		if (rd->dd[--did]){
			intptr_t *opt = rd->dd[did]->opt;
			if (option < rd->dd[did]->optTotal){
				opt[option] = value;
				return 1;
			}
		}
	}
	return 0;
}

int setDefaultPortOption (TREGISTEREDDRIVERS *rd, lDISPLAY did, int option, intptr_t value)
{
	
	if (did > 0&& did <= rd->ptotal){
		if (rd->pd[--did]){
			intptr_t *opt = rd->pd[did]->opt;
			if (option < rd->pd[did]->optTotal){
				opt[option] = value;
				return 1;
			}
		}
	}
	return 0;

}

/*
int getDefaultDisplayOption (lDISPLAY dnumber, int option, int *value)
{
	if (value && dnumber && (dnumber <= rd.dtotal)){
		if (rd.dd[--dnumber]){
			int *opt = (int *)rd.dd[dnumber]->opt;
			if (option < rd.dd[dnumber]->optTotal){
				*value = opt[option];
				return 1;
			}
		}
	}
	return 0;
}
*/

static void initDefaultCaps (THWD *hw)
{
	setCapabilities(hw, CAP_HTML_NUM_REF,	__BUILD_NUM_ENTITYREF_SUPPORT__);
	setCapabilities(hw, CAP_HTML_CHAR_REF,	__BUILD_CHR_ENTITYREF_SUPPORT__);
	

	setCapabilities(hw, CAP_PNG_READ,		__BUILD_PNG_READ_SUPPORT__);
	setCapabilities(hw, CAP_PNG_WRITE,		__BUILD_PNG_WRITE_SUPPORT__);
	setCapabilities(hw, CAP_JPG_READ,		__BUILD_JPG_READ_SUPPORT__);
	setCapabilities(hw, CAP_JPG_WRITE,		__BUILD_JPG_WRITE_SUPPORT__);
	setCapabilities(hw, CAP_BMP,			__BUILD_BMP_SUPPORT__);
	setCapabilities(hw, CAP_TGA,			__BUILD_TGA_SUPPORT__);
	setCapabilities(hw, CAP_PGM,			__BUILD_PGM_SUPPORT__);
	setCapabilities(hw, CAP_GIF,			__BUILD_GIF_SUPPORT__);

	setCapabilities(hw, CAP_DRAW,			__BUILD_DRAW_SUPPORT__);
	setCapabilities(hw, CAP_ROTATE,			__BUILD_ROTATE_SUPPORT__);
	setCapabilities(hw, CAP_SCROLL,			__BUILD_SCROLL_SUPPORT__);
	setCapabilities(hw, CAP_BDF_FONT,		__BUILD_BDF_FONT_SUPPORT__);
	setCapabilities(hw, CAP_BITMAP_FONT,	__BUILD_BITMAP_FONT_SUPPORT__);
	setCapabilities(hw, CAP_FONTS,			__BUILD_FONTS_SUPPORT__);
	setCapabilities(hw, CAP_CDECODE,		__BUILD_CHRDECODE_SUPPORT__);
	setCapabilities(hw, CAP_PRINT,			__BUILD_PRINT_SUPPORT__);
	setCapabilities(hw, CAP_BIG5,			__BUILD_INTERNAL_BIG5__);
	setCapabilities(hw, CAP_SJISX0213,		__BUILD_INTERNAL_JISX0213__);
	setCapabilities(hw, CAP_HZGB2312,		__BUILD_INTERNAL_HZGB2312__);

	setCapabilities(hw, CAP_NULLDISPLAY,	__BUILD_NULLDISPLAY__);
	setCapabilities(hw, CAP_NULLPORT,		__BUILD_NULLPORT__);
	setCapabilities(hw, CAP_DDRAW,			__BUILD_DDRAW__);
	setCapabilities(hw, CAP_SDL,			__BUILD_SDL__);
	setCapabilities(hw, CAP_OPENGL,			__BUILD_OPENGL__);
	setCapabilities(hw, CAP_KS0108,			__BUILD_KS0108__);
	setCapabilities(hw, CAP_SED1565,		__BUILD_SED1565__);
	setCapabilities(hw, CAP_USB13700EXP,	__BUILD_USB13700EXP__);
	setCapabilities(hw, CAP_USB13700DLL,	__BUILD_USB13700DLL__);
	setCapabilities(hw, CAP_USB13700LIBUSB, __BUILD_USB13700LIBUSB__);
	
	setCapabilities(hw, CAP_USBD480DLL,		__BUILD_USBD480DLL__);
	setCapabilities(hw, CAP_USBD480LIBUSB,	__BUILD_USBD480LIBUSB__);
	setCapabilities(hw, CAP_USBD480LIBUSBHID,__BUILD_USBD480__);
		
	setCapabilities(hw, CAP_SBUI,			__BUILD_SBUI__);
		
	setCapabilities(hw, CAP_T6963C,			__BUILD_T6963C__);
	setCapabilities(hw, CAP_SED1335,		__BUILD_SED1335__);
	setCapabilities(hw, CAP_PCD8544,		__BUILD_PCD8544__);
	setCapabilities(hw, CAP_PCF8814,		__BUILD_PCF8814__);
	setCapabilities(hw, CAP_S1D15G14,		__BUILD_S1D15G14__);
	setCapabilities(hw, CAP_PCF8833,		__BUILD_PCF8833__);
	setCapabilities(hw, CAP_S1D15G10,		__BUILD_S1D15G10__);
	setCapabilities(hw, CAP_LEDCARD,		__BUILD_LEDCARD__);

	setCapabilities(hw, CAP_G15LIBUSB,		__BUILD_G15LIBUSB__);
	setCapabilities(hw, CAP_G15DISPLAY,		__BUILD_G15DISPLAY__);	
	setCapabilities(hw, CAP_G19DISPLAY,		__BUILD_G19DISPLAY__);	
		
	setCapabilities(hw, CAP_WINIO,			__BUILD_WINIO__);
	setCapabilities(hw, CAP_OPENPARPORT,	__BUILD_OPENPARPORT__);
	setCapabilities(hw, CAP_DLPORTIO,		__BUILD_DLPORTIO__);
	setCapabilities(hw, CAP_SERIAL,			__BUILD_SERIAL__);
	setCapabilities(hw, CAP_FT245,			__BUILD_FT245__);
	setCapabilities(hw, CAP_FTDI,			__BUILD_FTDI__);
	
	setCapabilities(hw, CAP_APISYNC,		__BUILD_APISYNC_SUPPORT__);
	setCapabilities(hw, CAP_PTHREADS,		__BUILD_PTHREADS_SUPPORT__);
	setCapabilities(hw, CAP_PIXELEXPORTS,	__BUILD_PIXELPRIMITIVEEXPORTS__);

#ifdef __DEBUG__
	  setCapabilities(hw, CAP_DEBUG, CAP_STATE_ON);
#else
	  setCapabilities(hw, CAP_DEBUG, CAP_STATE_OFF);
#endif

#ifdef __DEBUG_SHOWFILEIO__
	  setCapabilities(hw, CAP_DEBUG_FILEIO, CAP_STATE_ON);
#else
	  setCapabilities(hw, CAP_DEBUG_FILEIO, CAP_STATE_OFF);
#endif
	
#ifdef __DEBUG_MEM__
	  setCapabilities(hw, CAP_DEBUG_MEM, CAP_STATE_ON);
#else
	  setCapabilities(hw, CAP_DEBUG_MEM, CAP_STATE_OFF);
#endif	

#ifdef USE_MMX
	  setCapabilities(hw, CAP_MMX, CAP_STATE_ON);
#else
	  setCapabilities(hw, CAP_MMX, CAP_STATE_OFF);
#endif		

#ifdef USE_MMX2
	  setCapabilities(hw, CAP_MMX2, CAP_STATE_ON);
#else
	  setCapabilities(hw, CAP_MMX2, CAP_STATE_OFF);
#endif

#ifdef USE_3DNOW
	  setCapabilities(hw, CAP_3DNOW, CAP_STATE_ON);
#else
	  setCapabilities(hw, CAP_3DNOW, CAP_STATE_OFF);
#endif		

#ifdef USE_SSE
	  setCapabilities(hw, CAP_SSE, CAP_STATE_ON);
#else
	  setCapabilities(hw, CAP_SSE, CAP_STATE_OFF);
#endif		
	
}


