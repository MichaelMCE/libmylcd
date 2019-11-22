//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//

#ifndef _DD_H_
#define _DD_H_

#define NONAMELESSUNION

#include <ddraw.h>
#include "converter.h"


#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

#define WM_MM					(WM_USER+4000)
#define WM_DD_PAINT				(WM_MM+1001)
#define WM_DD_MOUSEACTIVATE  	(WM_MM+WM_MOUSEACTIVATE)
#define WM_DD_MOUSEMOVE	    	(WM_MM+WM_MOUSEMOVE)
#define WM_DD_LBUTTONDOWN	   	(WM_MM+WM_LBUTTONDOWN)
#define WM_DD_LBUTTONUP	    	(WM_MM+WM_LBUTTONUP)
#define WM_DD_LBUTTONDBLCLK   	(WM_MM+WM_LBUTTONDBLCLK)
#define WM_DD_RBUTTONDOWN	   	(WM_MM+WM_RBUTTONDOWN)
#define WM_DD_RBUTTONUP	    	(WM_MM+WM_RBUTTONUP)
#define WM_DD_RBUTTONDBLCLK   	(WM_MM+WM_RBUTTONDBLCLK)
#define WM_DD_MBUTTONDOWN	   	(WM_MM+WM_MBUTTONDOWN)
#define WM_DD_MBUTTONUP	    	(WM_MM+WM_MBUTTONUP)
#define WM_DD_MBUTTONDBLCLK   	(WM_MM+WM_MBUTTONDBLCLK)
#define WM_DD_MOUSEWHEEL	   	(WM_MM+WM_MOUSEWHEEL)
#define WM_DD_MOUSEHWHEEL	   	(WM_MM+WM_MOUSEHWHEEL)
#define WM_DD_MOUSEHOVER	   	(WM_MM+WM_MOUSEHOVER)
#define WM_DD_MOUSELEAVE	   	(WM_MM+WM_MOUSELEAVE)
#define WM_DD_NCMOUSEHOVER		(WM_MM+WM_NCMOUSEHOVER)
#define WM_DD_NCMOUSELEAVE		(WM_MM+WM_NCMOUSELEAVE)
#define WM_DD_CHARDOWN			(WM_MM+WM_CHAR)
#define WM_DD_KEYDOWN			(WM_MM+WM_KEYDOWN)
#define WM_DD_CLOSE				(WM_MM+WM_CLOSE)

#define WM_DD_MINIMZE			0xF020
#define WM_DD_MAXIMIZE			0xF030
#define WM_DD_RESTORE			0xF120


typedef unsigned short short16;
typedef unsigned char char8;

#define MYLCDDDRAWWC "DDraw"

// configuration
#define __MDD_CLIPPER__		/*Z depth clip*/
#define __MDD_RESIZE_WINDOW__ 1
#define __MDD_SYSTEM_MENU__ 1
#define __MDD_ALLOW_CLOSE__ 1

#define __MDD_ICON__ "APP_ICON"


// converter configuration
#define __MDD_CONVERTER_32_TO_32_RGB888
#define __MDD_CONVERTER_32_TO_32_BGR888
#define __MDD_CONVERTER_32_TO_24_RGB888
#define __MDD_CONVERTER_32_TO_24_BGR888
#define __MDD_CONVERTER_32_TO_16_RGB565
#define __MDD_CONVERTER_32_TO_16_BGR565
#define __MDD_CONVERTER_32_TO_16_RGB555
#define __MDD_CONVERTER_32_TO_16_BGR555

//#define __MDD_MAIN_CRT__


// menu option identifier
#define SC_ZOOM_MSK			0x400
#define SC_ZOOM_100			0x401
#define SC_ZOOM_200			0x402
#define SC_ZOOM_400			0x404
#define SC_ZOOM_50			0x405
#define SC_ZOOM_75			0x406
#define MU_DDSTATE_ENABLE	0x801
#define MU_DDSTATE_DISABLE	0x802


// typedef void (*CONVERTER) (void *src, void *dst, int pixels);

typedef struct {
	TDRIVER				*drv;
	HWND				wnd;			// handle to this window
#if __MDD_SYSTEM_MENU__
	HMENU				menu;			// handle to menu
	MENUITEMINFO		menuinfo;		// menu item info
	int					mcount;			// menu item count
#endif
	LPDIRECTDRAW		lpDD;
	LPDIRECTDRAWSURFACE	lpDDS;
	LPDIRECTDRAWSURFACE	lpDDS_back;
	LPDIRECTDRAWSURFACE lpDDS_secondary;
    LPDIRECTDRAWCLIPPER lpDDC;
    CONVERTER			convert;
	unsigned int		*buffer;		// 32 bits per pixel buffer
	unsigned int		bufferLength;	// size of ^^ buffer
	T2POINT				des;
	int					status;			// active or not
	int original_window_width;
	int original_window_height;
	HWND				hTargetWin;		// used for input redirection to client
	
	struct{
		int width;
		int height;
		char name[256];
		
		HANDLE eventTrigger;
		int close;
	}open;
	unsigned int tid;
	HANDLE hThread;
}TMYLCDDDRAW;


int directDraw_create (TMYLCDDDRAW *mddraw, const char *title, const int width, const int height);



int registerWC ();
void releaseDDLib ();


#endif
