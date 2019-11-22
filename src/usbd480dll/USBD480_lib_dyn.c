
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

#if (__BUILD_USBD480DLL__)


#include "USBD480_lib.h"
#include "USBD480_lib_dyn.h"

#define USBD480LIB "USBD480_lib.dll"

static int isLoaded = 0;


static void dll_load_error (const char *fn, const char *dll, int err)
{
	if (err == -1)
		printf("libmylcd: '%s' not found\n", dll);
	else if (err == -2)
		printf("libmylcd: symbol '%s' not found in '%s'\n", fn, dll);
}

static int loadDLLFns (const char *fn)
{
	DLL_LOAD(fn, USBD480_Open, 1);
	DLL_LOAD(fn, USBD480_Close, 1);
	DLL_LOAD(fn, USBD480_GetNumberOfDisplays, 1);
	DLL_LOAD(fn, USBD480_GetDisplayConfiguration, 1);
	DLL_LOAD(fn, USBD480_DrawFullScreen, 1);
	DLL_LOAD(fn, USBD480_DrawFullScreenRGBA32, 0);
	DLL_LOAD(fn, USBD480_DrawFullScreenBGRA32, 0);
	DLL_LOAD(fn, USBD480_SetBrightness, 1);
	DLL_LOAD(fn, USBD480_SetTouchMode, 1);
	DLL_LOAD(fn, USBD480_SetAddress, 1);
	DLL_LOAD(fn, USBD480_SetFrameStartAddress, 1);
	DLL_LOAD(fn, USBD480_DrawFromBuffer, 1);
	DLL_LOAD(fn, USBD480_GetTouchReport, 1);
	//DLL_LOAD(fn, USBD480_GetTouchPosition, 0);
	return 1;
}

USBD480_API int __stdcall USBD480_Open (DisplayInfo *di, uint32_t flags)
{
	if (!isLoaded){
		if (loadDLLFns(USBD480LIB) != 1)
			return 0;
		isLoaded = 1;
	}
	return _USBD480_Open(di, flags);
}

USBD480_API int __stdcall USBD480_Close (DisplayInfo *di)
{
	return _USBD480_Close(di);
}

USBD480_API int __stdcall USBD480_GetNumberOfDisplays ()
{
	if (!isLoaded){
		if (loadDLLFns(USBD480LIB) != 1)
			return 0;
		isLoaded = 1;
	}
	return _USBD480_GetNumberOfDisplays();
}

USBD480_API int __stdcall USBD480_GetDisplayConfiguration (uint32_t index, DisplayInfo *di)
{
	if (!isLoaded){
		if (loadDLLFns(USBD480LIB) != 1)
			return 0;
		isLoaded = 1;
	}
	return _USBD480_GetDisplayConfiguration(index, di);
}

USBD480_API int __stdcall USBD480_DrawFullScreen (DisplayInfo *di, uint8_t *fb)
{
	return _USBD480_DrawFullScreen(di, fb);
}

USBD480_API int __stdcall USBD480_DrawFullScreenRGBA32 (DisplayInfo *di, uint32_t *fb)
{
	return _USBD480_DrawFullScreenRGBA32(di, fb);
}

USBD480_API int __stdcall USBD480_DrawFullScreenBGRA32 (DisplayInfo *di, uint32_t *fb)
{
	return _USBD480_DrawFullScreenBGRA32(di, fb);
}

USBD480_API int __stdcall USBD480_SetBrightness (DisplayInfo *di, uint32_t brightness)
{
	return _USBD480_SetBrightness(di, brightness);
}

USBD480_API int __stdcall USBD480_SetTouchMode (DisplayInfo *di, uint32_t mode)
{
	return _USBD480_SetTouchMode(di, mode);
}

USBD480_API int __stdcall USBD480_SetAddress (DisplayInfo *di, uint32_t address)
{
	return _USBD480_SetAddress(di, address);
}

USBD480_API int __stdcall USBD480_SetFrameStartAddress (DisplayInfo *di, uint32_t address)
{
	return _USBD480_SetFrameStartAddress(di, address);
}

USBD480_API int __stdcall USBD480_DrawFromBuffer (DisplayInfo *di, uint8_t *fb, uint32_t size)
{
	return _USBD480_DrawFromBuffer(di, fb, size);
}

USBD480_API int __stdcall USBD480_GetTouchReport (DisplayInfo *di, TouchReport *touch)
{
	return _USBD480_GetTouchReport(di, touch);
}

#endif

