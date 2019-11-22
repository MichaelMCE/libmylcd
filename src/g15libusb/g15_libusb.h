
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


#ifndef _G15_LIBUSB_H_
#define _G15_LIBUSB_H_



int initG15_libusb (TREGISTEREDDRIVERS *rd);
void closeG15_libusb (TREGISTEREDDRIVERS *rd);

struct TMYLCDG15LU;
typedef int (*lpsoftkeycb) (int device, int dwButtons, struct TMYLCDG15LU *mylcdg15);


typedef struct{
	lpsoftkeycb	softkeycb;
	void		*ptrUser;	// user space
	int			intUser;	// user space
	unsigned int updateCt;
	//TG15BUFFERS frames;
	ubyte data[G15_FRAMESIZE];
	TTHRDSYNCCTRL tsc;
	TTHRDSYNCCTRL hThreadKey;
	//TTHRDSYNCCTRL hThreadFrame;
	volatile int threadState;
	unsigned int t0;
	unsigned int t1;
}TMYLCDG15LU;




#endif 


