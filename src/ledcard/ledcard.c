
// Display driver for a PIC driven 40x12 Chinese LED Namecard

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

#if ((__BUILD_LEDCARD__) && (__BUILD_WIN32__))
#include <windows.h>

#include "../device.h"
#include "../display.h"
#include "../frame.h"
#include "../memory.h"
#include "../pixel.h"
#include "../copy.h"
#include "../lstring.h"
#include "../misc.h"
#include "ledcard.h"

static int LEDCARD_OpenDisplay (TDRIVER *drv);
static int LEDCARD_CloseDisplay (TDRIVER *drv);
static int LEDCARD_ClearDisplay (TDRIVER *drv);
static int LEDCARD_Refresh (TDRIVER *drv, TFRAME *frm);
static int LEDCARD_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int LEDCARD_setOption (TDRIVER *drv, int option, intptr_t *value);
static int LEDCARD_getOption (TDRIVER *drv, int option, intptr_t *value);

static int SetupUart (char *Port, int baud, int Bitsize, float StopBits, int Parity);
static int WriteUart (ubyte *buf, int len);
//static int ReadUart (ubyte *buf, int len);
static int CloseUart();
//static int Uart_SetBaud (int baud);
static INLINE void clearledbuffer();

#define COMPORT	"COM1"

static int Baud = 1200;
static int Bitsize = 8;
static float StopBits = 1;
//static int Parity = NOPARITY;
static ubyte ledbuffer[513];
static unsigned long dwMask;
static HANDLE hPort;


int initLEDCARD (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name, "LEDCARD");
	l_strcpy(dd.comment, "40x12 PIC namecard display driver.");
	dd.open = LEDCARD_OpenDisplay;
	dd.close = LEDCARD_CloseDisplay;
	dd.clear = LEDCARD_ClearDisplay;
	dd.refresh = LEDCARD_Refresh;
	dd.refreshArea = LEDCARD_RefreshArea;
	dd.setOption = LEDCARD_setOption;
	dd.getOption = LEDCARD_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;

	int dnumber =  registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_LEDCARD_MODE, 1); // FIXED:SCROLL 0:1
	return (dnumber > 0);
}

void closeLEDCARD (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int LEDCARD_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv){
		intptr_t *opt = drv->dd->opt;
		opt[option] = *value;
		return 1;
	}else
		return 0;
}

static int LEDCARD_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else
		return 0;
}

static int LEDCARD_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->status = LDRV_READY;
			drv->dd->back = NULL;
			return 1;
		}
	}

	return 0;

}

static int LEDCARD_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			return 1;
		}
	}

	return 0;

}

static int LEDCARD_ClearDisplay (TDRIVER *drv)
{
	if (!drv){
		return 0;
	}else{
		if (drv->dd->status == LDRV_READY){
			TFRAME *temp = _newFrame(drv->dd->hw, 40, 12, 1, LFRM_BPP_1);
			if (temp){
				intptr_t value = 0;
				LEDCARD_getOption(drv, lOPT_LEDCARD_MODE, &value);
				value = 1;
				LEDCARD_setOption(drv, lOPT_LEDCARD_MODE, &value);
				lRefresh(temp);
				deleteFrame(temp);
				value = 0;
				LEDCARD_setOption(drv, lOPT_LEDCARD_MODE, &value);
				return 1;
			}
		}
	}
	return 0;
}

static int LEDCARD_Refresh (TDRIVER *drv, TFRAME *frm)
{

	if (!frm||!drv){
		return 0;
	}else
		return LEDCARD_RefreshArea (drv, frm, 0, 0, frm->width-1, frm->height-1);
}

static int LEDCARD_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{

	if (!frm||!drv){
		return 0;
	}

	if ((x2<x1) || (y2<y1)){
		return 0;
	}

	if (drv->dd->status != LDRV_READY)
		return 0;

	x2 = MIN(MIN(frm->width, drv->dd->width),x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height),y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);
	x2 = MIN(x2,LEDCARD_BUFFER_WIDTH);
	if (y2-y1 > LEDCARD_BUFFER_HEIGHT) y2 = y1+LEDCARD_BUFFER_HEIGHT;

	// frame width must be multiple of 8
	int w = MAX(x2-x1,40);
	if (w&7) w += 8-(w&7);

	int x,y,p,b;
	int pos=0;
	int bit;
	ubyte len[3];


	TFRAME *temp = _newFrame(frm->hw, w, frm->height, 1, LFRM_BPP_1);
	if (!temp) return 0;

	for (y = 0; y < frm->height; y++){
		for (x = 0; x < frm->width; ){
			p=x+7;
			b=8;
			while(b--){
				if (getPixel_BC(frm,x++,y))
					setPixel_BC(temp,p,y,LSP_SET);
				p--;
			}
		}
	}

	int dlen = x2-x1;
	dlen /= 8;
	if ((x2-x1)&7) dlen++;

	intptr_t mode = 0;
	LEDCARD_getOption(drv, lOPT_LEDCARD_MODE, &mode);
	len[0] = mode;
	*(unsigned short *)&len[1] = (unsigned short)dlen;

	SetupUart(COMPORT, Baud, Bitsize, StopBits, NOPARITY);
	WriteUart((ubyte*)"Hello", 5);
	WriteUart((ubyte*)len, 3);

	for (y = y1; y < y2; y++){
		pos = 0;
		clearledbuffer();
		for (x = x1; x < x2;){
			bit=0;
			do{
				if (getPixel_BC(temp,x++,y))
					ledbuffer[pos] |= 1<<bit;

			}while(++bit < 8);
			pos ++;
		}

		WriteUart(ledbuffer, pos);
	}

	if (y<12){
		clearledbuffer();
		while(y++<12)
			WriteUart(ledbuffer, pos);
	}

	CloseUart();
	deleteFrame(temp);
	return 1;
}


static int SetupUart (char *Port, int baud, int Bitsize, float StopBits, int Parity)
{
	int STOPBITS=0;
	COMSTAT comstat;
	DCB PortDCB;
  	unsigned long comError;

	if(StopBits == 1)	STOPBITS = ONESTOPBIT;
	if(StopBits == 1.5) STOPBITS = ONE5STOPBITS;
	if(StopBits == 2)	STOPBITS = TWOSTOPBITS;

	// Open the serial port.
	hPort = CreateFile (TEXT(Port),						// Name of the port
						GENERIC_READ | GENERIC_WRITE,   // Access (read-write) mode
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

	// If it fails to open the port, return 0.
	if ( hPort == INVALID_HANDLE_VALUE) return 0;

	//Get the default port setting information.
	GetCommState (hPort, &PortDCB);
	// Change the settings.
	PortDCB.BaudRate = baud;              // BAUD Rate
	PortDCB.ByteSize = Bitsize;           // Number of bits/byte, 5-8
	PortDCB.Parity = Parity;              // 0-4=no,odd,even,mark,space
	PortDCB.StopBits = STOPBITS;          // StopBits
	PortDCB.fNull = 0;					  // Allow NULL Receive bytes
	PortDCB.fDtrControl=DTR_CONTROL_HANDSHAKE;


	// Re-configure the port with the new DCB structure.
	if (!SetCommState (hPort, &PortDCB)){
		CloseHandle(hPort);
		return 0;
	}

	// Retrieve the time-out parameters for all read and write operations on the port.
	COMMTIMEOUTS CommTimeouts;
	GetCommTimeouts (hPort, &CommTimeouts);
	memset(&CommTimeouts, 0x00, sizeof(CommTimeouts));
	CommTimeouts.ReadIntervalTimeout = -1;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 5000;

	// Set the time-out parameters for all read and write operations on the port.
	if (!SetCommTimeouts (hPort, &CommTimeouts)){
		CloseHandle(hPort);
		return 0;
	}

	ClearCommError(hPort, &comError, &comstat);
	SetCommMask(hPort, EV_RXCHAR |EV_TXEMPTY |EV_CTS |EV_DSR |EV_RLSD |EV_BREAK |EV_ERR |EV_RING);

	// Clear the port of any existing data.
	if(PurgeComm(hPort, PURGE_TXCLEAR | PURGE_RXCLEAR)==0){
		CloseHandle(hPort);
		return 0;
	}else
		return 1;
}


static int WriteUart (ubyte *buf, int len)
{
	DWORD dwNumBytesWritten;
	WriteFile (hPort,				// Port handle
		       buf,					// Pointer to the data to write
			   len,					// Number of bytes to write
			   &dwNumBytesWritten,	// Pointer to the number of bytes written
			   NULL);				// Must be NULL

	WaitCommEvent(hPort, &dwMask, NULL);

	if (dwNumBytesWritten > 0)
		return 1;					//Transmission was success
	else{
		mylog("libmylcd: uart write error:%i\n",*buf);
		return 0;					//Error transmitting?
	}
}
#if 0
static int ReadUart (ubyte *buf, int len)
{
	BOOL ret;
	unsigned long retlen;

	WaitCommEvent(hPort, &dwMask, NULL);

	memset(buf,0,len);
	ret = ReadFile(hPort,		// handle of file to read
					buf,		// pointer to buffer that receives data
					len,		// number of bytes to read
					&retlen,	// pointer to number of bytes read
					NULL		// pointer to structure for data
					);


	if (retlen)					//If we have data
		return (int)retlen;		//return the length
	else{
		mylog("libmylcd: uart read error: %i\n",(int)ret);
		return 0;				//else no data has been read
	}
 }

static int Uart_SetBaud (int sbaud)
{
	int STOPBITS=0;

	if(StopBits == 1)	STOPBITS = ONESTOPBIT;
	if(StopBits == 1.5) STOPBITS = ONE5STOPBITS;
	if(StopBits == 2)	STOPBITS = TWOSTOPBITS;

	DCB PortDCB;
	GetCommState (hPort, &PortDCB);
	PortDCB.BaudRate = sbaud;             // BAUD Rate
	PortDCB.ByteSize = Bitsize;           // Number of bits/byte, 5-8
	PortDCB.Parity = Parity;              // 0-4=no,odd,even,mark,space
	PortDCB.StopBits = STOPBITS;          // StopBits
	PortDCB.fNull = 0;					  // Allow NULL Receive bytes
	//PortDCB.fRtsControl=RTS_CONTROL_DISABLE;
	PortDCB.fDtrControl=DTR_CONTROL_HANDSHAKE; //DTR_CONTROL_ENABLE;

	// Re-configure the port with the new DCB structure.
	return SetCommState (hPort, &PortDCB);
}
#endif

static int CloseUart ()
{
	FlushFileBuffers(hPort);
	CloseHandle(hPort);
	return 1;
}

static INLINE void clearledbuffer ()
{
	memset(ledbuffer,0,sizeof(ledbuffer));
}

#else

int initLEDCARD (void *rd){return 1;}
void closeLEDCARD (void *rd){return;}


#endif

