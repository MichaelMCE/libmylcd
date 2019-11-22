
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


// usb13700 expansion port driver


#include "mylcd.h"

#if ((__BUILD_USB13700EXP__) && ((__BUILD_USB13700LIBUSB__) || (__BUILD_USB13700DLL__)))

#include "../memory.h"
#include "../device.h"
#include "../lstring.h"
#include "../misc.h"
#include "usb13700exp.h"


int usb13700_OpenDriver (TPORTDRIVER *pd, int port);
int usb13700_CloseDriver (TPORTDRIVER *pd);
int usb13700_WritePort8 (const int port, const ubyte data);
int usb13700_WritePort16 (const int port, const unsigned short data);
int usb13700_WritePort32 (const int port, const unsigned int data);
int usb13700_WriteBuffer (const int port, void *buffer, size_t tbytes);
int usb13700_Flush (TPORTDRIVER *pd);
ubyte usb13700_ReadPort (const int port);

static int drv_usb13700_getOption (TDRIVER *drv, int option, intptr_t *value);
static int drv_usb13700_setOption (TDRIVER *drv, int option, intptr_t *value);
static DisplayInfo *usb13700_getStruct (TPORTDRIVER *pd);

TPORTDRIVER *g_pd = NULL;

static unsigned char sendBuffer[USB13700_SPISENDBUFFERSIZE];
static unsigned int sendBufferPosition = 0;


int initUSB13700EXP (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;

	l_strcpy(pd.name,"USB13700:EXP");
	l_strcpy(pd.comment,"USB13700 Expansion port. 0x378 - SPI data, 0x378+2 - port0 ctrl");
	pd.open = usb13700_OpenDriver;
	pd.close = usb13700_CloseDriver;
	pd.write8 = usb13700_WritePort8;
	pd.write16 = usb13700_WritePort16;
	pd.write32 = usb13700_WritePort32;
	pd.writeBuffer = usb13700_WriteBuffer;
	pd.flush = usb13700_Flush;
	pd.read = usb13700_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = drv_usb13700_setOption;
	pd.getOption = drv_usb13700_getOption;
	pd.optTotal = 6;

	int dnumber = registerPortDriver(rd, &pd);
	setDefaultPortOption(rd, dnumber, lOPT_USB13700EXP_STRUCT, 0);
	setDefaultPortOption(rd, dnumber, lOPT_USB13700EXP_FRMLENGTH, 8);	// default to 9 bit SPI
	setDefaultPortOption(rd, dnumber, lOPT_USB13700EXP_CLKPOL, 0);
	setDefaultPortOption(rd, dnumber, lOPT_USB13700EXP_CLKPHASE, 0);
	setDefaultPortOption(rd, dnumber, lOPT_USB13700EXP_BITCLOCK, 0);
	setDefaultPortOption(rd, dnumber, lOPT_USB13700EXP_PRESCALER, 3);	// a number that works
	return (dnumber > 0);
}

void closeUSB13700EXP (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int SPISetup (DisplayInfo *di, uint8_t frameLength, uint8_t frameFormat, uint8_t cpol, uint8_t cpha, uint8_t bitclock, uint8_t prescaler, uint8_t sselMode)
{
	//printf("DisplayLib_ExPortSPIConfig: %i %i %i %i %i %i %i\n", frameLength, frameFormat, cpol, cpha, bitclock, prescaler, sselMode);
	return DisplayLib_ExPortSPIConfig(di, frameLength, frameFormat, cpol, cpha, bitclock, prescaler, sselMode);
}

static int pd_usb13700_setOption (TPORTDRIVER *pd, int option, intptr_t *value)
{
	if (pd){
		intptr_t *opt = pd->opt;

		switch(option){
			case lOPT_USB13700EXP_STRUCT:
				opt[option] = (intptr_t)value;
				break;
			case lOPT_USB13700EXP_FRMLENGTH:
			case lOPT_USB13700EXP_CLKPOL:
			case lOPT_USB13700EXP_CLKPHASE:
			case lOPT_USB13700EXP_BITCLOCK:
			case lOPT_USB13700EXP_PRESCALER:
			{
				opt[option] = *value;
				DisplayInfo *u13700di = usb13700_getStruct(pd);
				if (u13700di){
					SPISetup(u13700di,
					  opt[lOPT_USB13700EXP_FRMLENGTH]-1, 0,
					  opt[lOPT_USB13700EXP_CLKPOL],
					  opt[lOPT_USB13700EXP_CLKPHASE],
					  opt[lOPT_USB13700EXP_BITCLOCK],
					  opt[lOPT_USB13700EXP_PRESCALER],
					  SPI_SSEL_AUTO);
				}
				break;
			}
			default: return 0;
		}
		return 1;
	}else{
		return 0;
	}
}

static int pd_usb13700_getOption (TPORTDRIVER *pd, int option, intptr_t *value)
{
	if (pd && value){
		intptr_t *opt = pd->opt;
		*value = (intptr_t)opt[option];
		return 1;
	}else{
		return 0;
	}
}

static int drv_usb13700_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	return pd_usb13700_getOption(drv->pd, option, value);
}

static int drv_usb13700_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	return pd_usb13700_setOption(drv->pd, option, value);
}

static DisplayInfo *usb13700_getStruct (TPORTDRIVER *pd)
{
	DisplayInfo *u13700di = NULL;
	pd_usb13700_getOption(pd, lOPT_USB13700EXP_STRUCT, (intptr_t*)&u13700di);
	return u13700di;
}

static int usb13700_initiate (TPORTDRIVER *pd, int deviceIndex)
{
	DisplayInfo *u13700di = (DisplayInfo*)l_calloc(1, sizeof(DisplayInfo));
	if (u13700di){
		pd_usb13700_setOption(pd, lOPT_USB13700EXP_STRUCT, (intptr_t*)u13700di);
		int tDisplays = DisplayLib_GetNumberOfDisplays();

		//printf("tDisplays %i %i\n",tDisplays, deviceIndex);
		if (tDisplays > 0){
			//printf("DisplayLib_GetDisplayConfiguration\n");
			if (DisplayLib_GetDisplayConfiguration(deviceIndex-1, u13700di) == DISPLAYLIB_OK){
				mylog("usb13700dll_OpenDisplay: %i, -%s-, -%s-, %d, %d\n",\
				  tDisplays, u13700di->Name, u13700di->Username, u13700di->Width, u13700di->Height);
				if (DisplayLib_OpenDisplay(u13700di, deviceIndex-1) == DISPLAYLIB_OK){

					//printf(" DisplayLib_ExPortConfig() %i %i\n", EXPORT0_IO, EXPORT1_SPI_IO);
					DisplayLib_ExPortConfig(u13700di, EXPORT0_IO, EXPORT1_SPI_IO);

					//printf(" DisplayLib_ExPortIOConfig() %i %i\n", USB13700_PORT0MASK, 0x00);
					DisplayLib_ExPortIOConfig(u13700di, USB13700_PORT0MASK, 0x00);

					SPISetup(u13700di, 8, 0, 1, 1, 0, 33, SPI_SSEL_AUTO);	// default to 9bit SPI
					sendBufferPosition = 0;
					return 1;
				}else{
					printf("usb13700dll_OpenDisplay: USB13700 #%i unavailable\n", deviceIndex);
				}
			}else{
				printf("usb13700dll_OpenDisplay: USB13700 #%i not found or unavailable\n", deviceIndex);
			}
		}else{
			printf("usb13700dll_OpenDisplay: USB13700 not found\n");
		}

		l_free(u13700di);
		pd_usb13700_setOption(pd, lOPT_USB13700EXP_STRUCT, 0);
	}
	return 0;
}

static int usb13700_Close (TPORTDRIVER *pd)
{
	DisplayInfo *u13700di = usb13700_getStruct(pd);
	if (u13700di){
		DisplayLib_CloseDisplay(u13700di);
		l_free(u13700di);
		pd_usb13700_setOption(pd, lOPT_USB13700EXP_STRUCT, 0);
		return 1;
	}
	return 0;
}

int usb13700_OpenDriver (TPORTDRIVER *pd, int port)
{
	if (usb13700_initiate(pd, port)){
		pd->status = LDRV_READY;
		g_pd = pd;
		return 1;
	}else{
		pd->status = LDRV_CLOSED;
		return 0;
	}
}

int usb13700_CloseDriver (TPORTDRIVER *pd)
{
	if (pd->status != LDRV_CLOSED){
		pd->status = LDRV_CLOSED;
		g_pd = NULL;
		return usb13700_Close(pd);
	}
	return 0;
}

ubyte usb13700_ReadPort (const int port)
{
	if (!(port&0x02)){
		DisplayInfo *u13700di = usb13700_getStruct(g_pd);
		if (u13700di){
			uint32_t data = 0;
			//printf(" DisplayLib_ExPortSPIReceive()\n");
			DisplayLib_ExPortSPIReceive(u13700di, &data);
			return data&0xFF;
		}
	}else{
		DisplayInfo *u13700di = usb13700_getStruct(g_pd);
		if (u13700di){
			uint8_t tmp, data = 0;
			//printf(" DisplayLib_ExPortIOGet()\n");
			DisplayLib_ExPortIOGet(u13700di, &data, &tmp);
			return data;
		}
	}
	return 0;
}

int usb13700_WritePort8 (const int port, const ubyte data)
{
	if (!(port&0x02)){
		if (sendBufferPosition >= USB13700_SPISENDBUFFERSIZE) usb13700_Flush(g_pd);
		sendBuffer[sendBufferPosition++] = data;
		return 1;
	}else{
		DisplayInfo *u13700di = usb13700_getStruct(g_pd);
		if (u13700di){
			//printf(" DisplayLib_ExPortIOSet() %i %i %i %i\n", USB13700_PORT0MASK, data&USB13700_PORT0MASK, 0, 0);

			if (!DisplayLib_ExPortIOSet(u13700di, USB13700_PORT0MASK, data&USB13700_PORT0MASK, 0, 0))
				printf("DisplayLib_ExPortIOSet() returned 0\n");
			return 1;
		}
	}
	return 0;
}

int usb13700_WritePort16 (const int port, const unsigned short data)
{
	if (sendBufferPosition >= USB13700_SPISENDBUFFERSIZE) usb13700_Flush(g_pd);
	*(unsigned short*)(sendBuffer+sendBufferPosition) = data;
	sendBufferPosition += 2;
	return 1;
}

int usb13700_WritePort32 (const int port, const unsigned int data)
{
	if (sendBufferPosition >= USB13700_SPISENDBUFFERSIZE) usb13700_Flush(g_pd);
	*(unsigned int*)(sendBuffer+sendBufferPosition) = data;
	sendBufferPosition += 4;
	return 1;
}

int usb13700_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	while(tbytes--)
		usb13700_WritePort8(port, *(ubyte*)buffer++);
	return 0;
}

int usb13700_Flush (TPORTDRIVER *pd)
{
	DisplayInfo *u13700di = usb13700_getStruct(pd);
	if (!u13700di) return 0;

	if (sendBufferPosition){
		//printf("DisplayLib_ExPortSPISend() size: 0x%X/%i\n", sendBufferPosition, sendBufferPosition);
		if (!DisplayLib_ExPortSPISend(u13700di, (ubyte*)&sendBuffer, sendBufferPosition))
			printf("DisplayLib_ExPortSPISend() returned 0\n");
		sendBufferPosition = 0;
	}
	return 1;
}


#else

int initUSB13700EXP(void *rd){return 1;}
void closeUSB13700EXP(void *rd){return;}

#if (__BUILD_USB13700EXP__)
# error __BUILD_USB13700EXP__ requires __BUILD_USB13700LIBUSB__ or __BUILD_USB13700DLL__
#endif

#endif


