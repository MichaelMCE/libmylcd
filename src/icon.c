
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2008  Michael McElligott
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

#if (__BUILD_ICO_SUPPORT__)

#include "memory.h"
#include "frame.h"
#include "image.h"
#include <shlobj.h>



static inline int getDefaultHeight (THWD *hw)
{
	return hw->flags.image.ico.defaultHeight;
}

static inline int setDefaultHeight (THWD *hw, const int height)
{
	int old = hw->flags.image.ico.defaultHeight;
	hw->flags.image.ico.defaultHeight = height;
	return old;
}

static inline int newFrameFromHICON (TFRAME *frame, const int flags, const HICON icon, int height)
{

	//printf("iconinfo %i\n", iconinfo.fIcon);

	ICONINFO iconinfo = {0};
	GetIconInfo(icon, &iconinfo);
	if (height < 8){
		BITMAP bm;
		int ret = (int)GetObject(iconinfo.hbmMask, sizeof(BITMAP), (LPSTR)&bm);
		//printf("GetObject %i %i %i %i %p\n", ret, (int)bm.bmWidth, (int)bm.bmHeight, bm.bmBitsPixel, bm.bmBits);
		if (!ret) return 0;
		height = bm.bmHeight;
	}
	
	const int len = height * height;
	unsigned int mask[len];
	unsigned int colour[len];		
	GetBitmapBits(iconinfo.hbmColor, sizeof(colour), colour);
	

	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(BITMAPINFO));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biWidth = height;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biSizeImage = sizeof(colour);
	bmi.bmiHeader.biCompression = BI_RGB;
	
	const HDC dc = GetDC(NULL);
	GetDIBits(dc, iconinfo.hbmMask, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
	/*ret = (int)*/GetDIBits(dc, iconinfo.hbmMask, 0, height, &mask, &bmi, DIB_RGB_COLORS);
	//printf("GetDIBits %i %i\n", ret, bmi.bmiHeader.biBitCount);
	ReleaseDC(NULL, dc);
	DeleteObject(iconinfo.hbmColor);
	DeleteObject(iconinfo.hbmMask);
	
	
	if (flags&LOAD_RESIZE){
		if (!_resizeFrame(frame, height, height, 0))
			return 0;
	}

	unsigned int hasAlpha = 0xFF000000;
	for (int i = 0; i < len; i++){
		if (colour[i]&hasAlpha){
			hasAlpha = 0x00;
			break;
		}
	}

	unsigned int *pixels = lGetPixelAddress(frame, 0, 0);
	for (int p = 0; p < len; p++){
		if (!hasAlpha || !mask[p])
			pixels[p] = hasAlpha|colour[p];
	}

	//lSaveImage(frmIcon, L"test.png", IMG_PNG|IMG_KEEPALPHA, 0, 0);

	return 1;
}

static inline int newFrameFromFile (TFRAME *frame, const int flags, const wchar_t *filename, const int height)
{

 	HICON icon = NULL;
	int ret = SHDefExtractIconW(filename, 0, 0, &icon, NULL, height);
	if (!ret && icon){
 		int ret = newFrameFromHICON(frame, flags, icon, height);
		DestroyIcon(icon);
		return ret;
 	}
	return 0;
}

int loadIco (TFRAME *frame, const int flags, const wchar_t *filename, const int ox, const int oy)
{
	return newFrameFromFile(frame, flags, filename, getDefaultHeight(frame->hw));
}

MYLCD_EXPORT int icoSetDefaultHeight (THWD *hw, const int height)
{
	return setDefaultHeight(hw, height);
}

int icoGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp)
{
	
	if (width) *width = 0;
	if (height) *height = 0;
	if (bpp) *bpp = LFRM_BPP_32A;
		
	HICON icon = NULL;
#if 0
	int ret = SHDefExtractIconW(filename, 0, 0, &icon, NULL, 0);
	if (!ret && icon){
		DestroyIcon(icon);

		if (width) *width = 256;
		if (height) *height = 256;
		return 1;
	}
#else
	int ret = SHDefExtractIconW(filename, 0, 0, &icon, NULL, 0);
	if (!ret && icon){
		ICONINFO iconinfo = {0};
		GetIconInfo(icon, &iconinfo);	
	
		BITMAP bm = {0};
		/*int ret = (int)*/GetObject(iconinfo.hbmColor, sizeof(BITMAP), (LPSTR)&bm);
		//printf("icoGetMetrics %i %i %i %i %p\n", ret, (int)bm.bmWidth, (int)bm.bmHeight, bm.bmBitsPixel, bm.bmBits);
		DestroyIcon(icon);
		if (width) *width = bm.bmWidth;
		if (height) *height = bm.bmHeight;
	
		return bm.bmWidth && bm.bmHeight;
	}
#endif
	return 0;
}


#else

int loadIco (TFRAME *frame, const int flags, const wchar_t *filename, const int ox, const int oy) {return 0;}
int icoGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp) {return 0;}

#endif

