
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
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


    
#include <signal.h>

#include "mylcd.h"
#include "lcd.h"
#include "memory.h"
#include "apilock.h"
#include "device.h"
#include "lstring.h"
#include "pixel.h"
#include "frame.h"
#include "display.h"
#include "copy.h"
#include "utils.h"
#include "bdf.h"
#include "print.h"
#include "cmap.h"
#include "fonts.h"
#include "fileio.h"
#include "textbdf.h"
#include "chardecode.h"
#include "scroll.h"
#include "misc.h"

#include "usb13700/usb13700exp.h"
#include "pcf8814/pcf8814_sio.h"
#include "pcf8814/pcf8814_spi.h"
#include "pcd8544/pcd8544_sio.h"
#include "pcd8544/pcd8544_spi.h"
#include "s1d15g14/s1d15g14_sio.h"
#include "s1d15g14/s1d15g14_spi.h"
#include "pcf8833/pcf8833_spi.h"
#include "s1d15g10/s1d15g10_spi.h"

#include "usbd480libusb/usbd480.h"
#include "usbd480hid/usbd480hid.h"
#include "usbd480dll/usbd480dll.h"

#include "winio/winio.h"
#include "null/nullport.h"
#include "null/nulldisplay.h"
#include "dlportio/dlp.h"
#include "sdl/sdldisplay.h"
#include "opengl/gldisplay.h"
#include "openparport/openparport.h"
#include "ddraw/ddrawdisplay.h"
#include "ks0108/ks0108.h"
#include "sed1565/sed1565sio.h"
#include "sed1565/sed1565pio.h"
#include "linux/linux.h"
#include "serial/serial.h"
#include "ledcard/ledcard.h"
#include "ft245/ft245.h"
#include "ftdi/ftdi.h"
#include "t6963c/t6963c.h"
#include "sed1335/sed1335.h"
#include "usb13700dll/usb13700dll.h"
#include "usb13700libusb/usb13700_libusb.h"
#include "g15/g15display.h"
#include "g15/g15.h"
#include "g19/g19display.h"
#include "g15libusb/g15_libusb.h"
#include "sbui153/sbui.h"
#include "sbui20/sbui.h"


//#define __ENABLE_EXSIG__

#ifdef __ENABLE_EXSIG__
#if defined(__WIN32__)
#ifndef SIGBREAK
#define SIGBREAK 21
#endif
#endif

static inline int initSignals (void (*func)(int));
static inline void poked (int sig);
#endif

static inline int initFilesRD (TREGISTEREDDRIVERS *rd);
static inline void closeFilesRD (TREGISTEREDDRIVERS *rd);

#if HAVE_SEE
#include "xmmintrin.h"
#endif

THWD *openLib (const wchar_t *fontPath, const wchar_t *mapPath)
{

#ifdef __ENABLE_EXSIG__
	initSignals(poked);
#endif

	initDebugPrint();
	initMemory();
	
	THWD *hw = newHWDeviceDesc();
	if (hw){
		hw->dtree = createDeviceTree();	// registered devices are stored here
		hw->flist = createFrameTree();	// allocated frames are here
		hw->sync = createAPILock();		// thread sync (lock) handles are here
		
		
		initDisplay(hw);
		initFileIO(hw);
		initPixel(hw);
		initBdf(hw);
		initScroll(hw);
		initFilesRD(hw->dtree);
		registerInternalFonts(hw, fontPath);
		registerCharacterMaps(hw, mapPath);
		resetTableIndex(hw);
		
		// set some defaults
		setRenderEffect(hw, LTR_DEFAULT);
		setCharacterEncodingTable(hw, CMT_DEFAULT);
		resetCharDecodeFlags(hw);
	}
	return hw;
}

void closeLib (THWD *hw)
{

	void *lock = hw->sync;
	void *flist = hw->flist;
	void *dtree = hw->dtree;
		
	deleteFonts(hw);
	deleteRootCMapList(hw);

	closeScroll(hw);
	closeDisplay(hw);
	closeBdf(hw);
	closePixel(hw);
	closeFileIO(hw);
	freeHWDeviceDesc(hw);
	closeFilesRD(dtree);
	freeFrameTree(flist);
	freeDeviceTree(dtree);
	closeAPILock(lock);
	closeMemory();
	closeDebugPrint();
}

static inline void closeFilesRD (TREGISTEREDDRIVERS *rd)
{
	
	// shutdown display drivers before port drivers
	closeNullDisplay(rd);
	closeDDrawDisplay(rd);

	closePCD8544(rd);
	closePCF8814(rd);
	closeS1D15G14(rd);	
	closePCD8544_USB(rd);
	closePCF8814_USB(rd);
	closePCF8833_USB(rd);
	closeS1D15G10_USB(rd);
	closeS1D15G14_SPI(rd);
	closeUSBD480(rd);
	closeUSBD480HID(rd);
	closeUSBD480DLL(rd);
	closeG15Display(rd);
	closeG19Display(rd);
	closeSDLDisplay(rd);
	closeGLDisplay(rd);
	closeKS0108(rd);
	closeSED1565sio(rd);
	closeSED1565pio(rd);
	closeT6963C(rd);
	closeSED1335(rd);
	closeUSB13700dll(rd);
	closeLIBUSB13700(rd);
	closeLEDCARD(rd);
	closeG15_libusb(rd);
	sbui153Close(rd);
	sbui20Close(rd);
	
	closeNullPort(rd);
	closeDLP(rd);
	closeWinIO(rd);
	closeOPP(rd);
	closeLinux(rd);
	closeSerial(rd);
	closeFT245(rd);
	closeFTDI(rd);
	closeUSB13700EXP(rd);

}

static inline int initFilesRD (TREGISTEREDDRIVERS *rd)
{
	
	int status = 0;

	// port/control drivers must be initialized before display drivers

	status += initWinIO(rd);
	status += initDLP(rd);
	status += initSerial(rd);
	status += initOPP(rd);
	status += initLinux(rd);
	status += initFT245(rd);
	status += initFTDI(rd);
	status += initUSB13700EXP(rd);
	
	status += initNullPort(rd);

	status += sbui153Init(rd);
	status += sbui20Init(rd);
	status += initG15_libusb(rd);
	status += initG15Display(rd);
	status += initG19Display(rd);
	status += initUSBD480(rd);
	status += initUSBD480HID(rd);
	status += initUSBD480DLL(rd);
	status += initPCD8544(rd);
	status += initPCF8814(rd);
	status += initS1D15G14(rd);
	status += initPCD8544_USB(rd);
	status += initPCF8814_USB(rd);
	status += initPCF8833_USB(rd);
	status += initS1D15G10_USB(rd);	
	status += initS1D15G14_SPI(rd);
	status += initKS0108(rd);
	status += initSED1565sio(rd);
	status += initSED1565pio(rd);
	status += initSED1335(rd);
	status += initUSB13700dll(rd);
	status += initLIBUSB13700(rd);
	status += initT6963C(rd);

	status += initDDrawDisplay(rd);

	status += initSDLDisplay(rd);
	status += initGLDisplay(rd);
	status += initLEDCARD(rd);

	status += initNullDisplay(rd);
	return status;
}

#ifdef __ENABLE_EXSIG__
static inline int initSignals (void (*func)(int))
{
	static int signalReg;
	
	if (!signalReg){
		signalReg = 1;
		signal(SIGINT, func);		/* Interactive attention */
		signal(SIGILL, func);		/* Illegal instruction */
		signal(SIGSEGV, func);		/* Segmentation violation */
		signal(SIGTERM, func);		/* Termination request */
#if defined(__WIN32__)
	    signal(SIGBREAK, func);	/* Control-break */
#endif
	}
	return 1;
}

static inline void poked (int sig)
{
	signal(SIGSEGV, SIG_IGN);

	if (sig == SIGINT){
		printf("\nlibmylcd: signal caught: SIGINT\n");
	}else if (sig == SIGILL){
		printf("\nlibmylcd: signal caught: SIGILL\n");
	}else if (sig == SIGSEGV){
		printf("\nlibmylcd: signal caught: SIGSEGV\n");
		exit(EXIT_SUCCESS);
	}else if (sig == SIGTERM){
		printf("\nlibmylcd: signal caught: SIGTERM\n");
#if defined(__WIN32__)
	}else if (sig == SIGBREAK){
		printf("\nlibmylcd: signal caught: SIGBREAK\n");
#endif
	}
	exit(EXIT_SUCCESS);
}

#endif


