
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

#if (__BUILD_USB13700DLL__)

#include "../memory.h"
#include "../frame.h"
#include "../device.h"
#include "../pixel.h"
#include "../copy.h"
#include "../convert.h"
#include "../lstring.h"
#include "../misc.h"
#include "../pixel.h"
#include "../usb13700/display_lib_USB13700.h"
#include "usb13700dll.h"



static INLINE void clearDisplay (TDRIVER *drv);
static INLINE int quadrefresh (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm);
static INLINE int _quadrefresh (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm);
static void usb13700dll_queueCommand (DisplayInfo *u13700di, ubyte cmd);
static void usb13700dll_queueByte (DisplayInfo *u13700di, ubyte data);
static void usb13700dll_queueBytes (DisplayInfo *u13700di, ubyte *data, unsigned int size);
static int usb13700dll_updateArea (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int usb13700dll_update (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm);
static DisplayInfo *usb13700dll_getStruct (TDRIVER *drv);
static INLINE void usb13700dll_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);
static void usb13700dll_queueExecute (DisplayInfo *u13700di);
static void usb13700dll_writeCommand (DisplayInfo *u13700di, ubyte cmd);
static void create_rbittable ();

int usb13700dll_OpenDisplay (TDRIVER *drv);
int usb13700dll_CloseDisplay (TDRIVER *drv);
int usb13700dll_ClearDisplay (TDRIVER *drv);
int usb13700dll_Refresh (TDRIVER *drv, TFRAME *frm);
int usb13700dll_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int usb13700dll_getOption (TDRIVER *drv, int option, intptr_t *value);
int usb13700dll_setOption (TDRIVER *drv, int option, intptr_t *value);


void initiateUSB13700dll (TDRIVER *drv);
void shutdownUSB13700dll (TDRIVER *drv);


static ubyte rbittable[256];

int initUSB13700dll (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name,"USB13700:DLL");
	l_strcpy(dd.comment,"USB13700");
	dd.open = usb13700dll_OpenDisplay;
	dd.close = usb13700dll_CloseDisplay;
	dd.clear = usb13700dll_ClearDisplay;
	dd.refresh = usb13700dll_Refresh;
	dd.refreshArea = usb13700dll_RefreshArea;
	dd.setOption = usb13700dll_setOption;
	dd.getOption = usb13700dll_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;

	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_U13700D_STRUCT, 0);

	create_rbittable();
	return (dnumber > 0);
}

void closeUSB13700dll (TREGISTEREDDRIVERS *rd)
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

int usb13700dll_getOption (TDRIVER *drv, int option, intptr_t *value)
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

int usb13700dll_setOption (TDRIVER *drv, int option, intptr_t *value)
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

#if 0
void test (DisplayInfo *u13700di)
{
	printf("DisplayLib_SetDisplayLibOption()\n");
	DisplayLib_SetDisplayLibOption(u13700di, DISPLAYLIB_INVERTMODE, DISPLAYLIB_TRUE);

	unsigned int x=0, y=0;
	printf("DisplayLib_VersionInfo()...\n");
	DisplayLib_VersionInfo(&x, &y);
	printf("DisplayLib_VersionInfo: fw:%i, ver:%i\n", x, y);

	printf("DisplayLib_SetBacklight() 1/on to 0x77\n");
	DisplayLib_SetBacklight(u13700di, 1, 0x77);

	printf("DisplayLib_SetBacklight() 0/off\n");
	DisplayLib_SetBacklight(u13700di, 0, 0);

	printf("DisplayLib_WriteText() 'hello' at x:1 y:2\n");
	DisplayLib_WriteText(u13700di, "hello", 5, 0x01, 0x02);

	x=0, y=0;
	printf("DisplayLib_GetTouchScreenSample()...\n");
	DisplayLib_GetTouchScreenSample(u13700di, &x, &y);
	printf("DisplayLib_GetTouchScreenSample: x:%i, y:%i\n", x, y);

	//printf("DisplayLib_SetStartupBitmap()\n");
	//DisplayLib_SetStartupBitmap(u13700di, "racer.bmp");

}
#endif

int usb13700dll_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			if (drv->dd->back){
				DisplayInfo *u13700di = (DisplayInfo*)l_calloc(1, sizeof(DisplayInfo));
				if (u13700di){
					usb13700dll_setOption(drv, lOPT_U13700D_STRUCT, (intptr_t*)u13700di);
					mylog("DisplayLib_GetNumberOfDisplays()\n");
					int tDisplays = DisplayLib_GetNumberOfDisplays();
					if (tDisplays > 0){
						mylog("DisplayLib_GetDisplayConfiguration(): %i\n",tDisplays);
						if (DisplayLib_GetDisplayConfiguration(drv->dd->port-1, u13700di) == DISPLAYLIB_OK){
							mylog("usb13700dll_OpenDisplay: %i, -%s-, -%s-, %d, %d\n",\
							  tDisplays, u13700di->Name, u13700di->Username, u13700di->Width, u13700di->Height);

							mylog("DisplayLib_OpenDisplay()\n");
							if (DisplayLib_OpenDisplay(u13700di, drv->dd->port-1) == DISPLAYLIB_OK){
								usb13700dll_writeCommand(u13700di, 0x4F);	// cursor moves down
								drv->dd->status = LDRV_READY;
								drv->dd->clear(drv);
								return 1;
							}else{
								mylog("usb13700dll_OpenDisplay: USB13700 #%i unavailable\n", drv->dd->port);
							}
						}else{
							mylog("usb13700dll_OpenDisplay: USB13700 #%i not found or unavailable\n", drv->dd->port);
						}
					}else{
						mylog("usb13700dll_OpenDisplay: USB13700 not found\n");
					}

					l_free(u13700di);
					usb13700dll_setOption(drv, lOPT_U13700D_STRUCT, 0);
				}
			}
		}
	}
	return 0;
}


int usb13700dll_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;

			DisplayInfo *u13700di = usb13700dll_getStruct(drv);
			if (u13700di){
				usb13700dll_writeCommand(u13700di, 0x4C);	// cursor moves right
				mylog("DisplayLib_CloseDisplay()\n");
				DisplayLib_CloseDisplay(u13700di);
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

int usb13700dll_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			clearDisplay(drv);
			clearFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

static DisplayInfo *usb13700dll_getStruct (TDRIVER *drv)
{
	DisplayInfo *u13700di = NULL;
	drv->dd->getOption(drv, lOPT_U13700D_STRUCT, (void*)&u13700di);
	return u13700di;
}

int usb13700dll_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (drv == NULL || frm == NULL){
		return 0;

	}else if (drv->dd->status == LDRV_READY){
		DisplayInfo *u13700di = usb13700dll_getStruct(drv);
		if (!u13700di) return 0;

		if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
			if (quadrefresh(u13700di, drv, frm))
				usb13700dll_queueExecute(u13700di);
			return 1;
		}else{
			usb13700dll_update(u13700di, drv, frm);
			return 1;
		}
	}
	return 0;
}

int usb13700dll_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
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
		DisplayInfo *u13700di = usb13700dll_getStruct(drv);
		if (u13700di){
			DisplayLib_ClearS1D13700Queue(u13700di);
			return usb13700dll_updateArea(u13700di, drv, frm, x1, y1, x2, y2);
		}
	}
	return 0;
}

/*
write ep = 2
read ep = x82
get usb13700 config: ep2: FE 1 A, returns with on ep82: 1 x14 w1 w2 h1 h2 u s b 1 3 7 0 0
get backlight config :ep2: FE 5 A. replies with 7 bytes
cmd = ep:2 FE 7 A nn
data byte = ep:2 FE 9 A nn
stream: ep2: FE 8 A LSB MSB nn nn nn nn			(total = 16bit value, LSB first. eg: 28 00 = 0x0028)
full frame update: ep2: FE 2 A SIZE_LSB SIZE_MSB nn nn nn nn: (total = 16bit value, frame size (320x240 = 9600), LSB first. eg: 9600 = 0x8025, size should not include 5 byte header)
set backlight: ep2: FE 4 A state level: state = 1/on 0/off, level = 0-255. replies on ep82: 4 x14 1
write text: ep2: FE A A len1 len2 x y t e x t: len/length of text, x/y = character position in 8x8 blocks: eg 'FE A A 5 0 1 2 h e l l o'
get touch screen sample: ep2: FE C A: returns on ep82: 'C 14 x x y y' x/y = 16bit values


set startup bitmap: 2 writes then 1 read
write 1: FE 6 A size1 size2: size = length of write2
write 2: w w h h size1 size2 nn nn nn nn.  eg (40 1 F0 0 57 25) = 320 240 9559
read: 6 x14 1

80 25
128 37 = 9600


DisplayLib_ExPortConfig() port0:1  port1:0x03
write: ep:2 size:7 (FE F A 1 3 1 3 )
read: ep:82: 512 bytes: ( F 14 1 ..)

DisplayLib_ExPortIOConfig() port0Dir:0x03 port1Dir:0
write() ep:2 size:7 (FE 10 A 3 0 3 0 )
read() ep:82 size:512 ( 10 14 1 0 0


DisplayLib_ExPortSPIConfig(): 8 0 1 1 0 33 0
write() ep:2 size:9 (FE 13 A C8 0 21 0 0 1 )
read() ep:82 size:512 ( 13 14 1 0

DisplayLib_ExPortIOSet() 3 3 0 0
write() ep:2 size:7 (FE 11 A 3 3 0 0 )

DisplayLib_ExPortSPISend() size: 0x12/18
write() ep:2 size:23 (FE 14 A 12 0 CA 0 C 1 20 1 C 1 1 1 BB 0 1 1 D1 0 94 0 )
DisplayLib_ExPortSPISend() size: 0xC61E/50718
write() ep:2 size:50723 (FE 14 A 1E C6 25 0 75 0 2 1 8 ..)

*/


int usb13700dll_update (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm)
{
	TFRAME *src = NULL;
	if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
		//usb13700dll_copyArea(frm, drv->dd->back, 0, 0, 0, 0, frm->width-1, frm->height-1);
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

	//libusb13700_WriteS1D13700ClearQueue(u13700di);
	usb13700dll_queueCommand(u13700di, 0x4C);		// cursor moves right
	usb13700dll_queueCommand(u13700di, 0x46);		// set memory position
	usb13700dll_queueByte(u13700di, USB13700GFXLAYERADDR&0xFF);
	usb13700dll_queueByte(u13700di, USB13700GFXLAYERADDR>>8);
	usb13700dll_queueCommand(u13700di, 0x42);		// write to memory
	usb13700dll_queueBytes(u13700di, buffer, drv->dd->back->frameSize);
	usb13700dll_queueCommand(u13700di, 0x4F);		// cursor moves down
	usb13700dll_queueExecute(u13700di);
	l_free(buffer);
	return 1;
}

int usb13700dll_updateArea (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	// sync back buffer with front
	if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
		//usb13700dll_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);
		copyArea_fast8(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);
	}

	x2++; y2++;
	ubyte width[frm->height];
	const int addr_y1_pitch = USB13700GFXLAYERADDR+(y1*frm->pitch);
	unsigned short addr;
	int y, i=0;
	int x;

	for (x = x1; x < x2; x+=8){
		usb13700dll_queueCommand(u13700di, 0x46);			// set memory position
		addr = addr_y1_pitch+(x>>3);
		usb13700dll_queueBytes(u13700di, (void*)&addr, 2);	// set 2 byte address
		usb13700dll_queueCommand(u13700di, 0x42);			// write to memory
		i = 0;
		for (y = y1; y < y2; y++)
			width[i++] = rbittable[*(ubyte*)getPixel_addr_NB(frm, x, y)];
		usb13700dll_queueBytes(u13700di, width, i);
	}
	return 1;
}

static INLINE void clearDisplay (TDRIVER *drv)
{
	if (drv){
		DisplayInfo *u13700di = usb13700dll_getStruct(drv);
		if (u13700di){
			ubyte *buffer = (ubyte*)l_malloc(u13700di->Height * u13700di->Width);
			if (buffer){
				// clear gfx layer
				l_memset(buffer, drv->dd->clr, u13700di->Height * u13700di->Width);
				usb13700dll_writeCommand(u13700di, 0x4C);	// cursor moves right
				mylog("DisplayLib_DrawScreen()\n");
				if (DisplayLib_DrawScreen(u13700di, buffer) != DISPLAYLIB_OK)
					mylog("clearDisplay(): Unable to refresh\n");

				// clear text layer
				unsigned short addr = USB13700TXTLAYERADDR;
				l_memset(buffer, 0,  2*u13700di->Height * (u13700di->Width/8));
				usb13700dll_queueCommand(u13700di, 0x4C);			// cursor moves right
				usb13700dll_queueCommand(u13700di, 0x46);			// set memory position
				usb13700dll_queueBytes(u13700di, (void*)&addr, 2);	// set 2 byte address
				usb13700dll_queueCommand(u13700di, 0x42);			// write to memory
				usb13700dll_queueBytes(u13700di, buffer, (u13700di->Height/8) * (u13700di->Width/8));
				usb13700dll_queueCommand(u13700di, 0x4F);			// cursor moves down
				usb13700dll_queueExecute(u13700di);
				l_free(buffer);
			}
		}
	}
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y)
{
	int j, i;
	int xPLUS8 = MIN(x+u13700_quadBoxW, a->width);
	int yPLUS8 = MIN(y+u13700_quadBoxH, a->height);

	//x--;
	//for (i = xPLUS8; i > x; i--){
	for (i = x; i<xPLUS8; i++){
		for (j=y; j<yPLUS8; j++){
			if (getPixel_NB(a, i, j) ^ getPixel_NB(b, i, j))
				return 0;
		}
	}
	return 1;
}

int _usb13700dll_updateArea (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm, ubyte *pixels, int x, int y, int w, int h)
{
	// sync back buffer with front
	if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
		//usb13700dll_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);
		copyArea_fast8(frm, drv->dd->back, x, y, x, y, x+(w-1), y+(h-1));
	}
	DisplayLib_DrawRectFromScrBuf(u13700di, pixels, x, y, w, h);
	return 1;
}

static INLINE int _quadrefresh (DisplayInfo *u13700di, TDRIVER *drv, TFRAME *frm)
{
	int x2 = MIN(frm->width, drv->dd->width);
	int y2 = MIN(frm->height, drv->dd->height);
	int x,y;
	TFRAME *back = drv->dd->back;

	ubyte pixels[frm->width * frm->height];
	frame1To8BPP(frm, pixels, 0, 1);

	for (y=0; y<y2; y += u13700_quadBoxH){
		for (x=0; x<x2; x += u13700_quadBoxW){
			if (!cmpquad(frm, back, x, y)){
				_usb13700dll_updateArea(u13700di, drv, frm, pixels, x, y, u13700_quadBoxW, u13700_quadBoxH);
			}
		}
	}
	return 0;
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

	//libusb13700_WriteS1D13700ClearQueue(u13700di);

	for (y=y1;y<y2; y += u13700_quadBoxH){
		for (x=x1;x<x2; x += u13700_quadBoxW){
			if (!cmpquad(frm, back, x, y)){
				usb13700dll_updateArea(u13700di, drv, frm, x, y, x+(u13700_quadBoxW-1), y+(u13700_quadBoxH-1));
				if (++updates > mean){ /*140 is ~9600*/
					usb13700dll_update(u13700di, drv, frm);
					return 0;
				}
			}
		}
	}
	return updates;
}

static void usb13700dll_queueByte (DisplayInfo *u13700di, ubyte data)
{
	mylog("DisplayLib_WriteS1D13700DataByte() queue, data:%i\n",data);
	DisplayLib_WriteS1D13700DataByte(u13700di, data, 1);
}

static void usb13700dll_queueCommand (DisplayInfo *u13700di, ubyte cmd)
{
	mylog("DisplayLib_WriteS1D13700Command() queue, cmd:%X\n",cmd);
	DisplayLib_WriteS1D13700Command(u13700di, cmd, 1);
}

static void usb13700dll_queueBytes (DisplayInfo *u13700di, ubyte *data, unsigned int size)
{
	mylog("DisplayLib_WriteS1D13700Data() queue, size:%i\n", size);
	DisplayLib_WriteS1D13700Data(u13700di, data, size, 1);
}

static void usb13700dll_queueExecute (DisplayInfo *u13700di)
{
	mylog("DisplayLib_WriteS1D13700ExecuteQueue()\n");
	DisplayLib_WriteS1D13700ExecuteQueue(u13700di);
}

static void usb13700dll_writeCommand (DisplayInfo *u13700di, ubyte cmd)
{
	mylog("DisplayLib_WriteS1D13700Command() cmd:%X\n", cmd);
	DisplayLib_WriteS1D13700Command(u13700di, cmd, 0);
}

static INLINE void usb13700dll_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
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

int initUSB13700dll(void *rd){return 1;}
void closeUSB13700dll(void *rd){return;}

#endif

