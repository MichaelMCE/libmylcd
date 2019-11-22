
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

#include <windows.h>
#include "../usb13700/display_lib_USB13700.h"

#define USB13700_DLL_NAME "display_lib_USB13700.dll"


typedef int (*DisplayLib_GetNumberOfDisplays_t) ();
typedef int (*DisplayLib_GetDisplayConfiguration_t) (uint32_t index, DisplayInfo *di);
typedef int (*DisplayLib_OpenDisplay_t) (DisplayInfo *di, uint32_t flags);
typedef int (*DisplayLib_CloseDisplay_t) (DisplayInfo *di);
typedef int (*DisplayLib_DrawScreen_t) (DisplayInfo *di, uint8_t *fb);
typedef int (*DisplayLib_SetBacklight_t) (DisplayInfo *di, uint32_t onOff, uint32_t brightness);
typedef int (*DisplayLib_WriteS1D13700Command_t) (DisplayInfo *di, uint8_t cmd, uint32_t queue);
typedef int (*DisplayLib_WriteS1D13700Data_t) (DisplayInfo *di, uint8_t *data, uint32_t size, uint32_t queue);
typedef int (*DisplayLib_WriteS1D13700DataByte_t) (DisplayInfo *di, uint8_t data, uint32_t queue);
typedef int (*DisplayLib_WriteS1D13700ExecuteQueue_t) (DisplayInfo *di);
typedef int (*DisplayLib_WriteText_t) (DisplayInfo *di, char *text, uint32_t size, uint32_t x, uint32_t y);
typedef int (*DisplayLib_GetTouchScreenSample_t) (DisplayInfo *di, uint32_t *x, uint32_t *y);
typedef int (*DisplayLib_SetDisplayLibOption_t) (DisplayInfo *di, uint32_t option, uint32_t value);
typedef int (*DisplayLib_VersionInfo_t) (uint32_t *fw, uint32_t *ver);
typedef int (*DisplayLib_SetStartupBitmap_t) (DisplayInfo *di, char *path);
typedef int (*DisplayLib_ClearS1D13700Queue_t) (DisplayInfo *di);
typedef int (*DisplayLib_GetS1D13700QueueLength_t) (DisplayInfo *di, uint32_t *length);
typedef int (*DisplayLib_DrawRect_t) (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef int (*DisplayLib_DrawRectFromScrBuf_t) (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef int (*DisplayLib_SPI8Bit_t) (DisplayInfo *di, uint8_t dc, uint8_t txData, uint8_t *rxData);
typedef int (*DisplayLib_ExPortSPISend_t) (DisplayInfo *di, uint8_t *data, uint32_t size);
typedef int (*DisplayLib_ExPortSPIReceive_t) (DisplayInfo *di, uint32_t *rcvData);
typedef int (*DisplayLib_ExPortIOGet_t) (DisplayInfo *di, uint8_t *port0, uint8_t *port1);
typedef int (*DisplayLib_ExPortConfig_t) (DisplayInfo *di, uint8_t port0, uint8_t port1);
typedef int (*DisplayLib_ExPortIOConfig_t) (DisplayInfo *di, uint8_t port0Dir, uint8_t port1Dir);
typedef int (*DisplayLib_ExPortSPIConfig_t) (DisplayInfo *di, uint8_t frameLength, uint8_t frameFormat, uint8_t cpol, uint8_t cpha, uint8_t bitclock, uint8_t prescaler, uint8_t sselMode);
typedef int (*DisplayLib_ExPortIOSet_t) (DisplayInfo *di, uint8_t port0Mask, uint8_t port0, uint8_t port1Mask, uint8_t port1);

static DisplayLib_ExPortSPISend_t _DisplayLib_ExPortSPISend = NULL;
static DisplayLib_ExPortSPIReceive_t _DisplayLib_ExPortSPIReceive = NULL;
static DisplayLib_ExPortIOGet_t _DisplayLib_ExPortIOGet = NULL;
static DisplayLib_ExPortConfig_t _DisplayLib_ExPortConfig = NULL;
static DisplayLib_ExPortIOConfig_t _DisplayLib_ExPortIOConfig = NULL;
static DisplayLib_ExPortSPIConfig_t _DisplayLib_ExPortSPIConfig = NULL;
static DisplayLib_ExPortIOSet_t _DisplayLib_ExPortIOSet = NULL;
static DisplayLib_SPI8Bit_t _DisplayLib_SPI8Bit = NULL;
static DisplayLib_GetNumberOfDisplays_t _DisplayLib_GetNumberOfDisplays = NULL;
static DisplayLib_GetDisplayConfiguration_t _DisplayLib_GetDisplayConfiguration = NULL;
static DisplayLib_OpenDisplay_t _DisplayLib_OpenDisplay = NULL;
static DisplayLib_CloseDisplay_t _DisplayLib_CloseDisplay = NULL;
static DisplayLib_DrawScreen_t _DisplayLib_DrawScreen = NULL;
static DisplayLib_SetBacklight_t _DisplayLib_SetBacklight = NULL;
static DisplayLib_WriteS1D13700Command_t _DisplayLib_WriteS1D13700Command = NULL;
static DisplayLib_WriteS1D13700Data_t _DisplayLib_WriteS1D13700Data = NULL;
static DisplayLib_WriteS1D13700DataByte_t _DisplayLib_WriteS1D13700DataByte = NULL;
static DisplayLib_WriteS1D13700ExecuteQueue_t _DisplayLib_WriteS1D13700ExecuteQueue = NULL;
static DisplayLib_WriteText_t _DisplayLib_WriteText = NULL;
static DisplayLib_GetTouchScreenSample_t _DisplayLib_GetTouchScreenSample = NULL;
static DisplayLib_SetDisplayLibOption_t _DisplayLib_SetDisplayLibOption = NULL;
static DisplayLib_VersionInfo_t _DisplayLib_VersionInfo = NULL;
static DisplayLib_SetStartupBitmap_t _DisplayLib_SetStartupBitmap = NULL;
static DisplayLib_ClearS1D13700Queue_t _DisplayLib_ClearS1D13700Queue = NULL;
static DisplayLib_GetS1D13700QueueLength_t _DisplayLib_GetS1D13700QueueLength = NULL;
static DisplayLib_DrawRect_t _DisplayLib_DrawRect = NULL;
static DisplayLib_DrawRectFromScrBuf_t _DisplayLib_DrawRectFromScrBuf = NULL;


static int dllInitStatus = 0;



int dllInit ()
{
	HINSTANCE usb13700_dll = LoadLibrary(USB13700_DLL_NAME);
	if (!usb13700_dll){
		printf("libmylcd: %s: "USB13700_DLL_NAME" not found\n", __FILE__);
		return 0;
	}
	
	_DisplayLib_GetNumberOfDisplays = (DisplayLib_GetNumberOfDisplays_t)GetProcAddress(usb13700_dll, "_DisplayLib_GetNumberOfDisplays@0");
	_DisplayLib_GetDisplayConfiguration = (DisplayLib_GetDisplayConfiguration_t)GetProcAddress(usb13700_dll, "_DisplayLib_GetDisplayConfiguration@8");
	_DisplayLib_OpenDisplay = (DisplayLib_OpenDisplay_t)GetProcAddress(usb13700_dll, "_DisplayLib_OpenDisplay@8");
	_DisplayLib_CloseDisplay = (DisplayLib_CloseDisplay_t)GetProcAddress(usb13700_dll, "_DisplayLib_CloseDisplay@4");
	_DisplayLib_DrawScreen = (DisplayLib_DrawScreen_t)GetProcAddress(usb13700_dll, "_DisplayLib_DrawScreen@8");
	_DisplayLib_SetBacklight = (DisplayLib_SetBacklight_t)GetProcAddress(usb13700_dll, "_DisplayLib_SetBacklight@12");
	_DisplayLib_WriteS1D13700Command = (DisplayLib_WriteS1D13700Command_t)GetProcAddress(usb13700_dll, "_DisplayLib_WriteS1D13700Command@12");
	_DisplayLib_WriteS1D13700Data = (DisplayLib_WriteS1D13700Data_t)GetProcAddress(usb13700_dll, "_DisplayLib_WriteS1D13700Data@16");
	_DisplayLib_WriteS1D13700DataByte = (DisplayLib_WriteS1D13700DataByte_t)GetProcAddress(usb13700_dll, "_DisplayLib_WriteS1D13700DataByte@12");
	_DisplayLib_WriteS1D13700ExecuteQueue = (DisplayLib_WriteS1D13700ExecuteQueue_t)GetProcAddress(usb13700_dll, "_DisplayLib_WriteS1D13700ExecuteQueue@4");
	_DisplayLib_WriteText = (DisplayLib_WriteText_t)GetProcAddress(usb13700_dll, "_DisplayLib_WriteText@20");
	_DisplayLib_GetTouchScreenSample = (DisplayLib_GetTouchScreenSample_t)GetProcAddress(usb13700_dll, "_DisplayLib_GetTouchScreenSample@12");
	_DisplayLib_SetDisplayLibOption = (DisplayLib_SetDisplayLibOption_t)GetProcAddress(usb13700_dll, "_DisplayLib_SetDisplayLibOption@12");
	_DisplayLib_SetStartupBitmap = (DisplayLib_SetStartupBitmap_t)GetProcAddress(usb13700_dll, "_DisplayLib_SetStartupBitmap@8");
	_DisplayLib_VersionInfo = (DisplayLib_VersionInfo_t)GetProcAddress(usb13700_dll, "_DisplayLib_VersionInfo@8");
	_DisplayLib_ClearS1D13700Queue = (DisplayLib_ClearS1D13700Queue_t)GetProcAddress(usb13700_dll, "_DisplayLib_ClearS1D13700Queue@4");
	_DisplayLib_GetS1D13700QueueLength = (DisplayLib_GetS1D13700QueueLength_t)GetProcAddress(usb13700_dll, "_DisplayLib_GetS1D13700QueueLength@8");
	_DisplayLib_DrawRect = (DisplayLib_DrawRect_t)GetProcAddress(usb13700_dll, "_DisplayLib_DrawRect@24");
	_DisplayLib_DrawRectFromScrBuf = (DisplayLib_DrawRectFromScrBuf_t)GetProcAddress(usb13700_dll, "_DisplayLib_DrawRectFromScrBuf@24");
	_DisplayLib_SPI8Bit = (DisplayLib_SPI8Bit_t)GetProcAddress(usb13700_dll, "_DisplayLib_SPI8Bit@16");
	
	
	_DisplayLib_ExPortSPISend = (DisplayLib_ExPortSPISend_t)GetProcAddress(usb13700_dll, "_DisplayLib_ExPortSPISend@12");
	_DisplayLib_ExPortSPIReceive = (DisplayLib_ExPortSPIReceive_t)GetProcAddress(usb13700_dll, "_DisplayLib_ExPortSPIReceive@8");
	_DisplayLib_ExPortIOGet = (DisplayLib_ExPortIOGet_t)GetProcAddress(usb13700_dll, "_DisplayLib_ExPortIOGet@12");
	_DisplayLib_ExPortConfig = (DisplayLib_ExPortConfig_t)GetProcAddress(usb13700_dll, "_DisplayLib_ExPortConfig@12");
	_DisplayLib_ExPortIOConfig = (DisplayLib_ExPortIOConfig_t)GetProcAddress(usb13700_dll, "_DisplayLib_ExPortIOConfig@12");
	_DisplayLib_ExPortSPIConfig = (DisplayLib_ExPortSPIConfig_t)GetProcAddress(usb13700_dll, "_DisplayLib_ExPortSPIConfig@32");
	_DisplayLib_ExPortIOSet = (DisplayLib_ExPortIOSet_t)GetProcAddress(usb13700_dll, "_DisplayLib_ExPortIOSet@20");

	dllInitStatus = (_DisplayLib_GetNumberOfDisplays && _DisplayLib_GetDisplayConfiguration && _DisplayLib_OpenDisplay &&
					 _DisplayLib_SetBacklight && _DisplayLib_DrawScreen && _DisplayLib_CloseDisplay && _DisplayLib_DrawRect &&
					 _DisplayLib_WriteS1D13700DataByte && _DisplayLib_WriteS1D13700Data && _DisplayLib_WriteS1D13700Command &&
					 _DisplayLib_WriteS1D13700ExecuteQueue && _DisplayLib_WriteText && _DisplayLib_GetTouchScreenSample &&
					 _DisplayLib_SetDisplayLibOption && _DisplayLib_SetStartupBitmap &&
					 _DisplayLib_ClearS1D13700Queue && _DisplayLib_GetS1D13700QueueLength && _DisplayLib_DrawRectFromScrBuf &&
					 _DisplayLib_SPI8Bit && _DisplayLib_ExPortSPISend && _DisplayLib_ExPortSPIReceive && _DisplayLib_ExPortIOGet && 
					 _DisplayLib_ExPortConfig && _DisplayLib_ExPortIOConfig && _DisplayLib_ExPortSPIConfig && _DisplayLib_ExPortIOSet);
	return dllInitStatus;
}


int __stdcall DisplayLib_GetNumberOfDisplays ()
{
	if (!dllInitStatus){
		if (!dllInit())
			return -1;
	}
	volatile int ret = _DisplayLib_GetNumberOfDisplays();
	return ret;
}

int __stdcall DisplayLib_GetDisplayConfiguration (uint32_t index, DisplayInfo *di)
{
	if (!dllInitStatus){
		if (!dllInit())
			return -1;
	}
	volatile int ret = _DisplayLib_GetDisplayConfiguration(index, di);
	return ret;
}

int __stdcall DisplayLib_OpenDisplay (DisplayInfo *di, uint32_t flags)
{
	if (!dllInitStatus){
		if (!dllInit())
			return -1;
	}
	volatile int ret = _DisplayLib_OpenDisplay(di, flags);
	return ret;
}

int __stdcall DisplayLib_VersionInfo (uint32_t *fw, uint32_t *ver)
{
	if (!dllInitStatus){
		if (!dllInit())
			return -1;
	}
	volatile int ret = _DisplayLib_VersionInfo(fw, ver);
	return ret;
}

int __stdcall DisplayLib_CloseDisplay (DisplayInfo *di)
{
	volatile int ret = _DisplayLib_CloseDisplay(di);
	return ret;
}

int __stdcall DisplayLib_DrawScreen (DisplayInfo *di, uint8_t *fb)
{
	volatile int ret = _DisplayLib_DrawScreen(di, fb);
	return ret;
}

int __stdcall DisplayLib_SetBacklight (DisplayInfo *di, uint32_t onOff, uint32_t brightness)
{
	volatile int ret = _DisplayLib_SetBacklight(di, onOff, brightness);
	return ret;
}

int __stdcall DisplayLib_WriteS1D13700Command (DisplayInfo *di, uint8_t cmd, uint32_t queue)
{
	volatile int ret = _DisplayLib_WriteS1D13700Command(di, cmd, queue);
	return ret;
}

int __stdcall DisplayLib_WriteS1D13700Data (DisplayInfo *di, uint8_t *data, uint32_t size, uint32_t queue)
{
	volatile int ret = _DisplayLib_WriteS1D13700Data(di, data, size, queue);
	return ret;
}

int __stdcall DisplayLib_WriteS1D13700DataByte (DisplayInfo *di, uint8_t data, uint32_t queue)
{
	volatile int ret = _DisplayLib_WriteS1D13700DataByte(di, data, queue);
	return ret;
}

int __stdcall DisplayLib_WriteS1D13700ExecuteQueue (DisplayInfo *di)
{
	volatile int ret = _DisplayLib_WriteS1D13700ExecuteQueue(di);
	return ret;
}

int __stdcall DisplayLib_WriteText (DisplayInfo *di, char *text, uint32_t size, uint32_t x, uint32_t y)
{
	volatile int ret = _DisplayLib_WriteText(di, text, size, x, y);
	return ret;
}

int __stdcall DisplayLib_GetTouchScreenSample (DisplayInfo *di, uint32_t *x, uint32_t *y)
{
	volatile int ret = _DisplayLib_GetTouchScreenSample(di, x, y);
	return ret;
}

int __stdcall DisplayLib_SetDisplayLibOption (DisplayInfo *di, uint32_t option, uint32_t value)
{
	volatile int ret = _DisplayLib_SetDisplayLibOption(di, option, value);
	return ret;
}

int __stdcall DisplayLib_SetStartupBitmap (DisplayInfo *di, char *path)
{
	volatile int ret = _DisplayLib_SetStartupBitmap(di, path);
	return ret;
}


int __stdcall DisplayLib_ClearS1D13700Queue (DisplayInfo *di)
{
	volatile int ret = _DisplayLib_ClearS1D13700Queue(di);
	return ret;
}

int __stdcall DisplayLib_GetS1D13700QueueLength (DisplayInfo *di, uint32_t *length)
{
	volatile int ret = _DisplayLib_GetS1D13700QueueLength(di, length);
	return ret;
}

int __stdcall DisplayLib_DrawRect (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	volatile int ret = _DisplayLib_DrawRect(di, fb, x, y, width, height);
	return ret;
}

int __stdcall DisplayLib_DrawRectFromScrBuf (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	volatile int ret = _DisplayLib_DrawRectFromScrBuf(di, fb, x, y, width, height);
	return ret;
}
	
int __stdcall DisplayLib_SPI8Bit (DisplayInfo *di, uint8_t dc, uint8_t txData, uint8_t *rxData)
{
	volatile int ret = _DisplayLib_SPI8Bit(di, dc, txData, rxData);
	return ret;
}

int __stdcall DisplayLib_ExPortSPISend (DisplayInfo *di, uint8_t *data, uint32_t size)
{
	volatile int ret =  _DisplayLib_ExPortSPISend(di, data, size);
	return ret;
}

int __stdcall DisplayLib_ExPortSPIReceive (DisplayInfo *di, uint32_t *rcvData)
{
	volatile int ret =  _DisplayLib_ExPortSPIReceive(di, rcvData);
	return ret;
}

int __stdcall DisplayLib_ExPortIOGet (DisplayInfo *di, uint8_t *port0, uint8_t *port1)
{
	volatile int ret = _DisplayLib_ExPortIOGet(di, port0, port1);
	return ret; 
}

int __stdcall DisplayLib_ExPortConfig (DisplayInfo *di, uint8_t port0, uint8_t port1)
{
	if (!dllInitStatus){
		if (!dllInit())
			return -1;
	}
	volatile int ret = _DisplayLib_ExPortConfig(di, port0, port1);
	return ret; 
}

int __stdcall DisplayLib_ExPortIOConfig (DisplayInfo *di, uint8_t port0Dir, uint8_t port1Dir)
{
	if (!dllInitStatus){
		if (!dllInit())
			return -1;
	}
	volatile int ret = _DisplayLib_ExPortIOConfig(di, port0Dir, port1Dir);
	return ret; 
}

int __stdcall DisplayLib_ExPortSPIConfig (DisplayInfo *di, uint8_t frameLength, uint8_t frameFormat, uint8_t cpol, uint8_t cpha, uint8_t bitclock, uint8_t prescaler, uint8_t sselMode)
{
	if (!dllInitStatus){
		if (!dllInit())
			return -1;
	}
	volatile int ret = _DisplayLib_ExPortSPIConfig(di, frameLength, frameFormat, cpol, cpha, bitclock, prescaler, sselMode);
	return ret; 
}

int __stdcall DisplayLib_ExPortIOSet (DisplayInfo *di, uint8_t port0Mask, uint8_t port0, uint8_t port1Mask, uint8_t port1)
{
	volatile int ret = _DisplayLib_ExPortIOSet(di, port0Mask, port0, port1Mask, port1);
	return ret;
}

#endif

