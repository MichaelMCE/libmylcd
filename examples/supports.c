
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.



#include "utils/utils.h"



int state = 0;


static void supports (THWD *hw, char *text, int cap)
{
	const int var = lGetCapabilities(hw, cap);
	if (var && state)
		printf("(%i) %s\n", cap, text);
	else if (!state && !var)
		printf("(%i) %s\n", cap, text);
}

void list (THWD *hw, char *text, int _state)
{
	state = _state;
	printf("%s\n", text);
	
	supports(hw, "HTML character reference", CAP_HTML_NUM_REF);
	supports(hw, "HTML entity reference", CAP_HTML_CHAR_REF);
	supports(hw, "PNG read", CAP_PNG_READ);
	supports(hw, "PNG write", CAP_PNG_WRITE);
	supports(hw, "Jpeg read", CAP_JPG_READ);
	supports(hw, "Jpeg write", CAP_JPG_WRITE);
	supports(hw, "BMP image", CAP_BMP);
	supports(hw, "TGA image", CAP_TGA);
	supports(hw, "PGM image", CAP_PGM);
	
	supports(hw, "BDF font", CAP_BDF_FONT);
	supports(hw, "BITMAP font", CAP_BITMAP_FONT);
	
	supports(hw, "Internal Big5", CAP_BIG5);
	supports(hw, "Shift JIS (0213)", CAP_SJISX0213);
	supports(hw, "HZ-GB2312", CAP_HZGB2312);

	supports(hw, "Primitive API", CAP_DRAW);
	supports(hw, "Rotate primitive API", CAP_ROTATE);
	supports(hw, "Print API", CAP_PRINT);
	supports(hw, "Font API", CAP_FONTS);
	supports(hw, "Character decoding API", CAP_CDECODE);
	supports(hw, "Scroll API", CAP_SCROLL);
	
	supports(hw, "PCD8544", CAP_PCD8544);
	supports(hw, "PCF8814", CAP_PCF8814);
	supports(hw, "S1D15G14", CAP_S1D15G14);
	supports(hw, "S1D15G10", CAP_S1D15G10);
	supports(hw, "PCF8833", CAP_PCF8833);
	supports(hw, "SED1565", CAP_SED1565);	
	supports(hw, "SED1335", CAP_SED1335);	
	supports(hw, "KS0108", CAP_KS0108);
	supports(hw, "T6963C", CAP_T6963C);

	supports(hw, "G15 LCD via libusb", CAP_G15LIBUSB);
	supports(hw, "G15 LCD via Logitech driver", CAP_G15DISPLAY);
	supports(hw, "G19 LCD via Logitech driver", CAP_G19DISPLAY);

	supports(hw, "USB13700 via SDK .DLL", CAP_USB13700DLL);
	supports(hw, "USB13700 via libusb13700", CAP_USB13700LIBUSB);
	supports(hw, "USB13700 Exp. port", CAP_USB13700EXP);
	
	supports(hw, "USBD480 (USBD480_lib.dll)", CAP_USBD480DLL);
	supports(hw, "USBD480 libusb", CAP_USBD480LIBUSB);
	supports(hw, "USBD480 libusb+HID (composite intf)", CAP_USBD480LIBUSBHID);

	supports(hw, "Switchblade UI (switchblade32.dll)", CAP_SBUI);

	supports(hw, "PIC RS232 Led Card display", CAP_LEDCARD);
	supports(hw, "Direct Draw virtual display", CAP_DDRAW);
	supports(hw, "SDL virtual display", CAP_SDL);
	supports(hw, "OpenGL virtual display", CAP_OPENGL);
	supports(hw, "NULL display", CAP_NULLDISPLAY);

	supports(hw, "NULL port", CAP_NULLPORT);
	supports(hw, "WinIO", CAP_WINIO);
	supports(hw, "OpenParPort", CAP_OPENPARPORT);
	supports(hw, "DLPortIO", CAP_DLPORTIO);
	supports(hw, "Serial", CAP_SERIAL);
	supports(hw, "FTDI FT245 USB driver", CAP_FT245);
	supports(hw, "FT245BM through libFTDI", CAP_FTDI);
	
	supports(hw, "pthreads ", CAP_PTHREADS);
	supports(hw, "lSet/GetPixel export API", CAP_PIXELEXPORTS);
	supports(hw, "API sync", CAP_APISYNC);
	
	supports(hw, "Debug", CAP_DEBUG);
	supports(hw, "Debug File IO", CAP_DEBUG_FILEIO);
	supports(hw, "Debug mem usage", CAP_DEBUG_MEM);
	
	supports(hw, "MMX", CAP_MMX);
	supports(hw, "MMX2", CAP_MMX2);
	supports(hw, "SSE", CAP_SSE);
	supports(hw, "3DNOW!", CAP_3DNOW);

	printf("\n");
}

int main (int argc, char* argv[])
{

	THWD *hw = lOpen(NULL, NULL);
	if (hw == NULL){
    	printf("lOpen() failed\n");
		return 0;
    }
    	
	printf("libmylcd v%s\n\n",(char*)lVersion());
	list(hw, " Disabled:", 0);
	list(hw, " Enabled:", 1);

    lClose(hw);
	return EXIT_SUCCESS;
}

