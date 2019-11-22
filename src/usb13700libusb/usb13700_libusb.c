
 // USB13700 @ www.lcdinfo.com

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

#if (__BUILD_USB13700LIBUSB__)

#include "../memory.h"
#include "../frame.h"
#include "../device.h"
#include "../pixel.h"
#include "../copy.h"
#include "../convert.h"
#include "../lstring.h"
#include "../pixel.h"
#include "../misc.h"
#include "usb13700_libusb.h"
#include "libusb13700/libusb13700.h"



static INLINE void clearDisplay (TDRIVER *drv);
static INLINE int quadrefresh (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm);
static void usb13700lu_queueCommand (DisplayInfo *u13700di, ubyte cmd);
void usb13700lu_queueByte (DisplayInfo *u13700di, ubyte data);
static void usb13700lu_queueBytes (DisplayInfo *u13700di, ubyte *data, unsigned int size);
static int usb13700lu_updateArea (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int usb13700lu_update (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm);
static DisplayInfo *usb13700lu_getStruct (TDRIVER *drv);
static INLINE void usb13700lu_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);
static void usb13700lu_queueExecute (DisplayInfo *u13700di);
static void usb13700lu_writeCommand (DisplayInfo *u13700di, ubyte cmd);

int usb13700lu_OpenDisplay (TDRIVER *drv);
int usb13700lu_CloseDisplay (TDRIVER *drv);
int usb13700lu_ClearDisplay (TDRIVER *drv);
int usb13700lu_Refresh (TDRIVER *drv, TFRAME *frm);
int usb13700lu_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int usb13700lu_getOption (TDRIVER *drv, int option, intptr_t *value);
int usb13700lu_setOption (TDRIVER *drv, int option, intptr_t *value);

void initiateUSB13700dll (TDRIVER *drv);
void shutdownUSB13700dll (TDRIVER *drv);
static void create_rbittable ();

static ubyte rbittable[256];


int initLIBUSB13700 (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name,"USB13700:LIBUSB");
	l_strcpy(dd.comment,"USB13700");
	dd.open = usb13700lu_OpenDisplay;
	dd.close = usb13700lu_CloseDisplay;
	dd.clear = usb13700lu_ClearDisplay;
	dd.refresh = usb13700lu_Refresh;
	dd.refreshArea = usb13700lu_RefreshArea;
	dd.setOption = usb13700lu_setOption;
	dd.getOption = usb13700lu_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;

	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_U13700D_STRUCT, 0);

	create_rbittable();
	return (dnumber > 0);
}

void closeLIBUSB13700 (TREGISTEREDDRIVERS *rd)
{
	return;
}

static void create_rbittable ()
{
	int i = 256;
	while(i--){
		rbittable[i] = (i&0x80)>>7;
		rbittable[i] |= (i&0x40)>>5;
		rbittable[i] |= (i&0x20)>>3;
		rbittable[i] |= (i&0x10)>>1;
		rbittable[i] |= (i&0x08)<<1;
		rbittable[i] |= (i&0x04)<<3;
		rbittable[i] |= (i&0x02)<<5;
		rbittable[i] |= (i&0x01)<<7;
	}
}

int usb13700lu_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		if (option < drv->dd->optTotal){
			intptr_t *opt = drv->dd->opt;
			*value = opt[option];
			return 1;
		}
	}
	return 0;
}

int usb13700lu_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = drv->dd->opt;

	if (option == lOPT_U13700D_STRUCT){
		opt[lOPT_U13700D_STRUCT] = (intptr_t)value;

	}else{
		return 0;
	}
	return 1;
}

int usb13700lu_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			if (drv->dd->back){
				DisplayInfo *u13700di = (DisplayInfo*)l_calloc(1, sizeof(DisplayInfo));
				if (u13700di){
					usb13700lu_setOption(drv, lOPT_U13700D_STRUCT, (intptr_t*)u13700di);
					mylog("libusb13700_GetNumberOfDisplays()\n");
					int tDisplays = libusb13700_GetNumberOfDisplays();
					if (tDisplays > 0){
						mylog("libusb13700_GetDisplayConfiguration(): %i\n",tDisplays);
						if (libusb13700_GetDisplayConfiguration(drv->dd->port, u13700di) == DISPLAYLIB_OK){
							mylog("libusb13700_OpenDisplay: %i, -%s-, -%s-, %d, %d\n",\
							  tDisplays, u13700di->Name, u13700di->Username, u13700di->Width, u13700di->Height);

							mylog("libusb13700_OpenDisplay()\n");
							if (libusb13700_OpenDisplay(u13700di, drv->dd->port) == DISPLAYLIB_OK){

								usb13700lu_writeCommand(u13700di, 0x4F);	// cursor moves down
								drv->dd->status = LDRV_READY;
								drv->dd->clear(drv);
								return 1;
							}else{
								mylog("libusb13700_OpenDisplay: USB13700 #%i unavailable\n", drv->dd->port);
							}
						}else{
							mylog("libusb13700_OpenDisplay: USB13700 #%i not found or unavailable\n", drv->dd->port);
						}
					}else{
						mylog("libusb13700_OpenDisplay: USB13700 not found\n");
					}
					lDeleteFrame(drv->dd->back);
					l_free(u13700di);
					usb13700lu_setOption(drv, lOPT_U13700D_STRUCT, NULL);
				}
			}
		}
	}
	return 0;
}


int usb13700lu_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;

			DisplayInfo *u13700di = usb13700lu_getStruct(drv);
			if (u13700di){
				usb13700lu_writeCommand(u13700di, 0x4C);	// cursor moves right
				mylog("libusb13700_CloseDisplay()\n");
				libusb13700_CloseDisplay(u13700di);
				drv->dd->setOption(drv, lOPT_U13700D_STRUCT, 0);
				l_free(u13700di);
			}
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

int usb13700lu_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			clearDisplay(drv);
			if (drv->dd->back)
				clearFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

static DisplayInfo *usb13700lu_getStruct (TDRIVER *drv)
{
	DisplayInfo *u13700di = NULL;
	drv->dd->getOption(drv, lOPT_U13700D_STRUCT, (intptr_t*)&u13700di);
	return u13700di;
}

int usb13700lu_Refresh (TDRIVER *drv, TFRAME *frm)
{

	if (drv == NULL || frm == NULL){
		return 0;

	}else if (drv->dd->status == LDRV_READY){
		DisplayInfo *u13700di = usb13700lu_getStruct(drv);
		if (!u13700di) return 0;

		if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
			if (quadrefresh(u13700di, drv, frm)){
				usb13700lu_queueExecute(u13700di);
			}
			return 1;
		}else{
			usb13700lu_update(u13700di, drv, frm);
			return 1;
		}
	}
	return 0;
}

int usb13700lu_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (frm == NULL)
		return 0;
	if (x2<x1 || y2<y1)
		return 0;

	x2 = MIN(MIN(frm->width, drv->dd->width), x2);
	y2 = MIN(MIN(frm->height, drv->dd->height), y2);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);

	if (drv->dd->status == LDRV_READY){
		DisplayInfo *u13700di = usb13700lu_getStruct(drv);
		if (u13700di)
			return usb13700lu_updateArea(u13700di, drv, frm, x1, y1, x2, y2);
	}
	return 0;
}

int usb13700lu_update (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm)
{
	TFRAME *src = NULL;
	if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
		//usb13700lu_copyArea(frm, drv->dd->back, 0, 0, 0, 0, frm->width-1, frm->height-1);
		copyArea_fast8(frm, drv->dd->back, 0, 0, 0, 0, frm->width-1, frm->height-1);
		src = drv->dd->back;
	}else{
		src = frm;
	}

	ubyte *buffer = l_malloc(src->frameSize);
	if (buffer == NULL) return 0;

	int i = src->frameSize;
	while (i--)
		buffer[i] = rbittable[src->pixels[i]];

	libusb13700_WriteS1D13700ClearQueue(u13700di);
	usb13700lu_queueCommand(u13700di, 0x4C);		// cursor moves right
	usb13700lu_queueCommand(u13700di, 0x46);		// set memory position
	const unsigned short addr = USB13700GFXLAYERADDR;
	usb13700lu_queueBytes(u13700di, (void*)&addr, 2);	// set 2 byte address
	usb13700lu_queueCommand(u13700di, 0x42);		// write to memory
	usb13700lu_queueBytes(u13700di, buffer, drv->dd->back->frameSize);
	usb13700lu_queueCommand(u13700di, 0x4F);		// cursor moves down
	usb13700lu_queueExecute(u13700di);

	l_free(buffer);
	return 1;
}

int usb13700lu_updateArea (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	// sync back buffer with front
	if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
		//usb13700lu_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);
		copyArea_fast8(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);
	}

	x2++; y2++;
	ubyte width[frm->height];
	const int addr_y1_pitch = USB13700GFXLAYERADDR+(y1*frm->pitch);
	unsigned short addr;
	int y, i=0;
	int x;

	for (x = x1; x < x2; x+=8){
		usb13700lu_queueCommand(u13700di, 0x46);			// set memory position
		addr = addr_y1_pitch+(x>>3);
		usb13700lu_queueBytes(u13700di, (void*)&addr, 2);	// set 2 byte address
		usb13700lu_queueCommand(u13700di, 0x42);			// write to memory
		i = 0;
		for (y = y1; y < y2; y++)
			width[i++] = rbittable[*(ubyte*)getPixel_addr_NB(frm, x, y)];
		usb13700lu_queueBytes(u13700di, width, i);
	}
	return 1;
}

static INLINE void clearDisplay (TDRIVER *drv)
{
	if (drv){
		DisplayInfo *u13700di = usb13700lu_getStruct(drv);
		if (u13700di){
			ubyte *buffer = (ubyte*)l_malloc(u13700di->Height * u13700di->Width);
			if (buffer){
				// clear gfx layer
				l_memset(buffer, drv->dd->clr, u13700di->Height * u13700di->Width);
				usb13700lu_writeCommand(u13700di, 0x4C);	// cursor moves right
				mylog("libusb13700_DrawScreen()\n");
				if (libusb13700_DrawScreen(u13700di, buffer) != DISPLAYLIB_OK)
					mylog("clearDisplay(): Unable to refresh\n");

				// clear text layer
				unsigned short addr = USB13700TXTLAYERADDR;
				l_memset(buffer, 0,  2*u13700di->Height * (u13700di->Width/8));
				usb13700lu_queueCommand(u13700di, 0x4C);			// cursor moves right
				usb13700lu_queueCommand(u13700di, 0x46);			// set memory position
				usb13700lu_queueBytes(u13700di, (void*)&addr, 2);	// set 2 byte address
				usb13700lu_queueCommand(u13700di, 0x42);			// write to memory
				usb13700lu_queueBytes(u13700di, buffer, (u13700di->Height/8) * (u13700di->Width/8));
				usb13700lu_queueCommand(u13700di, 0x4F);			// cursor moves down
				usb13700lu_queueExecute(u13700di);
				l_free(buffer);
			}
		}
	}
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y)
{
	int j, i;
	int xPLUS8 = MIN(x+u13700_quadBoxW-1, a->width-1);
	int yPLUS8 = MIN(y+u13700_quadBoxH, a->height);

	x--;
	for (i = xPLUS8; i > x; i--){
		for (j=y; j<yPLUS8; j++){
			if (getPixel_NB(a, i, j) ^ getPixel_NB(b, i, j))
				return 0;
		}
	}
	return 1;
}

static INLINE int quadrefresh (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm)
{
	int x2 = MIN(frm->width, drv->dd->width);
	int y2 = MIN(frm->height, drv->dd->height);
	int x1 = 0, y1 = 0;
	int updates = 0;
	int x,y;
	TFRAME *back = drv->dd->back;
	const int mean = ((frm->width>>3) * frm->height)/69.0;

	libusb13700_WriteS1D13700ClearQueue(u13700di);

	for (y=y1;y<y2; y += u13700_quadBoxH){
		for (x=x1;x<x2; x += u13700_quadBoxW){
			if (!cmpquad(frm, back, x, y)){
				usb13700lu_updateArea(u13700di, drv, frm, x, y, x+(u13700_quadBoxW-1), y+(u13700_quadBoxH-1));
				if (++updates > mean){ /*140 is ~9600*/
				//	if (libusb13700_WriteS1D13700GetQueueSize(u13700di) > mean){ //9600
						usb13700lu_update(u13700di, drv, frm);
						return 0;
				//	}
				}
			}
		}
	}
	return updates;
}

void usb13700lu_queueByte (DisplayInfo *u13700di, ubyte data)
{
	mylog("libusb13700_WriteS1D13700DataByte() queue, data:%i\n",data);
	libusb13700_WriteS1D13700DataByte(u13700di, data, 1);
}

static void usb13700lu_queueCommand (DisplayInfo *u13700di, ubyte cmd)
{
	mylog("libusb13700_WriteS1D13700Command() queue, cmd:%X\n",cmd);
	libusb13700_WriteS1D13700Command(u13700di, cmd, 1);
}

static void usb13700lu_queueBytes (DisplayInfo *u13700di, ubyte *data, unsigned int size)
{
	mylog("libusb13700_WriteS1D13700Data() queue, size:%i\n", size);
	libusb13700_WriteS1D13700Data(u13700di, data, size, 1);
}

static void usb13700lu_queueExecute (DisplayInfo *u13700di)
{
	mylog("libusb13700_WriteS1D13700ExecuteQueue()\n");
	libusb13700_WriteS1D13700ExecuteQueue(u13700di);
}

static void usb13700lu_writeCommand (DisplayInfo *u13700di, ubyte cmd)
{
	mylog("libusb13700_WriteS1D13700Command() cmd:%X\n", cmd);
	libusb13700_WriteS1D13700Command(u13700di, cmd, 0);
}

static INLINE void usb13700lu_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	if ((dx>to->width-1) || (dy>to->height-1))
		return;

	int x,y;
	int row;
	int xx=dx;
	y2=MIN(y2+1, from->height);
	x2=MIN(x2+1, from->width);
	y1=MIN(y1, from->height-1);
	x1=MIN(x1, from->width-1);

	for (y = y1; y<y2; y++,dy++){
		xx = dx;
		row = y * from->pitch;
		for (x = x1; x<x2; x++,xx++)
			setPixel_NB(to, xx, dy, getPixel_NBr(from, x, row));
	}
	return;
}

#else

int initLIBUSB13700(void *rd){return 1;}
void closeLIBUSB13700(void *rd){return;}

#endif

