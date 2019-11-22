
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


#ifndef _PDRIVER_H_
#define _PDRIVER_H_

enum _pamId {
	PAM_P1 = 1,
	PAM_P2,
	PAM_P3,
	PAM_P4,
	PAM_P5,
	PAM_P6,
	PAM_PF
};






#define lPGMPIXELTOTAL	256
#define lPGMPIXELSET	120
#define lPGMPIXELCLEAR	240


int savePgm (TFRAME *frame, const wchar_t *filename, int width, int height);
int loadPgm (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy);

int savePpm (TFRAME *frame, const wchar_t *filename, int width, int height);
int loadPpm (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy);

int savePbm (TFRAME *frame, const wchar_t *filename, const int width, const int height);
int loadPbm (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy);


int pgmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp);
int ppmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp);
int pbmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp);

#endif


