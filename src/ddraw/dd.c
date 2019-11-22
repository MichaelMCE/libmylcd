
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

// based on TinyPTC by Gaffer, www.gaffer.org/tinyptc


#include "mylcd.h"

#if ((__BUILD_DDRAW__) && (__BUILD_WIN32__))


#include <signal.h>
#include "../device.h"
#include "../display.h"
#include "../memory.h"
#include "../lmath.h"
#include "../convert.h"
#include "converter.h"


#include "dd.h"

#include "../sync.h"
#include "../misc.h"

#include <initguid.h>


//static HMODULE library = NULL;
#ifdef __MDD_ALLOW_CLOSE__
static int windowTotal;
#endif



//typedef HRESULT (WINAPI *DIRECTDRAWCREATE) (GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
//static DIRECTDRAWCREATE _DirectDrawCreate_;

//typedef HRESULT (WINAPI *DIRECTDRAWCREATEEX) (GUID*, LPVOID*, REFIID, IUnknown*);
//static DIRECTDRAWCREATEEX _DirectDrawCreateEx_;


intptr_t directDraw_open (TMYLCDDDRAW *mddraw, char *title, int width, int height);
int directDraw_update (TMYLCDDDRAW *mddraw, void *buffer, size_t bufferLength);
void directDraw_close (TMYLCDDDRAW *mddraw);


static void paint_primary (TMYLCDDDRAW *mddraw)
{
	if (!mddraw || !mddraw->status)
		return;

    if (mddraw->lpDDS){
    	POINT point;
    	RECT destination;
		RECT source;
        source.left = 0;
        source.top = 0;
        source.right = mddraw->des.x;
        source.bottom = mddraw->des.y;
        point.x = 0;
        point.y = 0;

        ClientToScreen(mddraw->wnd, &point);
        GetClientRect(mddraw->wnd, &destination);

        // offset destination rectangle
        destination.left += point.x;
        destination.top += point.y;
        destination.right += point.x;
        destination.bottom += point.y;

        //printf("%i %i %i %i\n", source.right, source.bottom, destination.right, destination.bottom);

        // blt secondary to primary surface
       /* int ret = (int)*/IDirectDrawSurface_Blt(mddraw->lpDDS, &destination, mddraw->lpDDS_secondary, &source, DDBLT_DONOTWAIT|DDBLT_ASYNC, 0);
        //int ret = (int)IDirectDrawSurface_BltFast(mddraw->lpDDS, point.x, point.y, mddraw->lpDDS_secondary, &source, DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);
        //int ret = (int)IDirectDrawSurface_Flip(mddraw->lpDDS, NULL, DDFLIP_WAIT);
        
       // printf("paint_primary  %i\n", ret);
    }
}

static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//    TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);


	//if (message != WM_DD_PAINT && message != WM_PAINT /*&& message != WM_MOUSEMOVE && message != WM_NCHITTEST && message != WM_SETCURSOR*/)
	//	if (message != 512 && message != 32 && message != 132 && message != 160)
	//		printf("DD WndProc %i/0x%X %i %p\n", (int)message, (int)message, (int)wParam, (void*)lParam);

    switch (message){
    case WM_DD_PAINT:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw) directDraw_update(mddraw, (void*)lParam, wParam);
		break;
	}

	case WM_PAINT:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw) paint_primary(mddraw);
		break;
	}

    case WM_MOUSEMOVE	:
    case WM_LBUTTONDOWN	:
    case WM_LBUTTONUP	:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN	:
    case WM_RBUTTONUP	:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN	:
    case WM_MBUTTONUP	:
    case WM_MBUTTONDBLCLK:
    case WM_MOUSEACTIVATE:
    case WM_MOUSEHOVER:
    case WM_MOUSELEAVE:
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
    case WM_MOUSEWHEEL	:
    case WM_MOUSEHWHEEL	:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    	if (mddraw && mddraw->hTargetWin)
    		PostMessage(mddraw->hTargetWin, WM_MM+message, wParam, lParam);
    	break;
	}

    case WM_DD_CLOSE:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw){
			mddraw->open.close = 1;
			PostMessage(hWnd, WM_NULL, 0, 0);
		}
		break;
	}

#ifndef __MDD_SYSTEM_MENU__
	case WM_SYSCOMMAND:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw){
			if (wParam == WM_DD_MINIMZE || wParam == WM_DD_MAXIMIZE || wParam == WM_DD_RESTORE){
				//printf("sys_cmd: %i\n", wParam);
    			if (mddraw && mddraw->hTargetWin)
					PostMessage(mddraw->hTargetWin, wParam, 0, 0);
			}
		}
	}


#elif __MDD_SYSTEM_MENU__

	case WM_SYSCOMMAND:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw){
			if (wParam == WM_DD_MINIMZE || wParam == WM_DD_MAXIMIZE || wParam == WM_DD_RESTORE){
    			if (mddraw && mddraw->hTargetWin)
					PostMessage(mddraw->hTargetWin, wParam, 0, 0);

			}else if (wParam == MU_DDSTATE_DISABLE){
				pauseDisplay(mddraw->drv->dd->hw, driverNameToID(mddraw->drv->dd->hw, (char *)mddraw->drv->dd->name, LDRV_DISPLAY));
				mddraw->menuinfo.fType = MFT_STRING;
				mddraw->menuinfo.wID = MU_DDSTATE_ENABLE;
   				mddraw->menuinfo.dwTypeData = "Engage";
				SetMenuItemInfo(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);
				
			}else if (wParam == MU_DDSTATE_ENABLE){
				resumeDisplay(mddraw->drv->dd->hw, driverNameToID(mddraw->drv->dd->hw, (char *)mddraw->drv->dd->name, LDRV_DISPLAY));
				mddraw->menuinfo.fType = MFT_STRING;
				mddraw->menuinfo.wID = MU_DDSTATE_DISABLE;
   				mddraw->menuinfo.dwTypeData = "Disengage";
				SetMenuItemInfo(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);
				
			}else if ((wParam&0xFFFFFFF0) == SC_ZOOM_MSK){
				float zoom = wParam&0x07;
				if ((int)zoom == (SC_ZOOM_50 & ~SC_ZOOM_MSK))
					zoom = 0.50f;
				else if ((int)zoom == (SC_ZOOM_75 & ~SC_ZOOM_MSK))
					zoom = 0.75f;
				int x = (GetSystemMetrics(SM_CXSCREEN) - mddraw->original_window_width*zoom) / 2;
				int y = (GetSystemMetrics(SM_CYMAXIMIZED) - mddraw->original_window_height*zoom) / 2;
				SetWindowPos(hWnd, NULL, x, y,mddraw->original_window_width*zoom, mddraw->original_window_height*zoom, SWP_NOZORDER);
      		}else{
	    	   	return DefWindowProc(hWnd, message, wParam, lParam);
       		}
       	}
	}
	break;
#endif

	case WM_CHAR:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw && mddraw->hTargetWin){
   			PostMessage(mddraw->hTargetWin, WM_DD_CHARDOWN, wParam, lParam);
   			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_KEYDOWN:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw && mddraw->hTargetWin){
   			PostMessage(mddraw->hTargetWin, WM_DD_KEYDOWN, wParam, lParam);
   			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_QUIT:
		raise(SIGINT);
		break;

	case WM_CLOSE:{
		TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (mddraw){
			if (mddraw->hTargetWin){
    			PostMessage(mddraw->hTargetWin, WM_MM+message, wParam, lParam);
    		}else{
        		mddraw->status = 0;
#ifdef __MDD_ALLOW_CLOSE__
            	if (--windowTotal < 1)
            		SendMessage(hWnd, WM_QUIT, 0, (LPARAM)mddraw);
            		
#endif
            }
    	}
		return 0;
	}

        //default:
            //result = DefWindowProc(hWnd,message,wParam,lParam);
    }
    return DefWindowProc(hWnd,message,wParam,lParam);
}


int initDDraw ()
{
	
	return 1;
	/*;
   	library = (HMODULE)LoadLibrary("ddraw.dll");
   	
   	if (library){
   		_DirectDrawCreate_ = (DIRECTDRAWCREATE)GetProcAddress(library,"DirectDrawCreate");
   		_DirectDrawCreateEx_ = (DIRECTDRAWCREATEEX)GetProcAddress(library,"DirectDrawCreateEx");
   		if (_DirectDrawCreate_){
   			return 1;
   		}else{
   			FreeLibrary(library);
   			mylog("libmylcd: DirectDrawCreate() not found\n");
   		}
   	}else{
   		mylog("libmylcd: ddraw.dll not found\n");
   	}
    library = NULL;
    return 0;*/
}

int registerWC ()
{
	WNDCLASS wc;

    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    
#ifdef __MDD_ICON__
      wc.hInstance = GetModuleHandle(0);
      wc.hIcon = LoadIcon(wc.hInstance,__MDD_ICON__);
#else
      wc.hInstance = 0;
      wc.hIcon = 0;
#endif

    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = MYLCDDDRAWWC;
    RegisterClass(&wc);

	return initDDraw();
}

intptr_t directDraw_open (TMYLCDDDRAW *mddraw, char *title, int width, int height)
{
    RECT rect;
    rect.right = mddraw->des.x = width;
    rect.bottom = mddraw->des.y = height;
    rect.left = 0;
    rect.top = 0;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, 0);

    rect.right -= rect.left;
    rect.bottom -= rect.top;
	mddraw->original_window_width = rect.right;
	mddraw->original_window_height = rect.bottom;


    // center window
	int x = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
	int y = (GetSystemMetrics(SM_CYMAXIMIZED) - rect.bottom) / 2;

	mddraw->hTargetWin = NULL;

#ifdef __MDD_ALLOW_CLOSE__
	//WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
	  int flag =/* WS_POPUP |*/ WS_CLIPCHILDREN | WS_CLIPSIBLINGS|WS_OVERLAPPEDWINDOW;//WS_POPUP|WS_EX_TOPMOST|WS_OVERLAPPEDWINDOW;
	  
#else
	  int flag = 0;
#endif

	//printf("directDraw_open adjust %i %i %i %i %i %i\n", width, height, (int)rect.left, (int)rect.right, (int)rect.top, (int)rect.bottom);

	uint32_t exflags = WS_EX_APPWINDOW;

#ifdef __MDD_RESIZE_WINDOW__
      mddraw->wnd = CreateWindowEx(exflags, MYLCDDDRAWWC, title, flag, x, y, rect.right, rect.bottom, 0, 0, 0, 0);
#else
      mddraw->wnd = CreateWindowEx(exflags, MYLCDDDRAWWC, title, flag & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, x, y, rect.right-(abs(rect.left)/2), rect.bottom-(abs(rect.left)/2),0,0,0,0);
#endif


	if (mddraw->wnd == NULL)
		return 0;

#ifdef __MDD_ALLOW_CLOSE__
	  windowTotal++;
#endif

	SetWindowLongPtr(mddraw->wnd, GWLP_USERDATA, (intptr_t)mddraw);
    ShowWindow(mddraw->wnd, SW_SHOW);
    SetForegroundWindow(mddraw->wnd);
    BringWindowToTop(mddraw->wnd);
    SetFocus(mddraw->wnd);
   /* RECT rc;
    GetClientRect(mddraw->wnd, &rc);
    printf("directDraw_open %i %i %i %i\n",  mddraw->original_window_width, width, (int)rc.left, (int)rc.right);*/


#ifdef __MDD_RESIZE_WINDOW__
#ifdef __MDD_SYSTEM_MENU__
      mddraw->menu = GetSystemMenu(mddraw->wnd, FALSE);
	  mddraw->mcount = GetMenuItemCount(mddraw->menu);
      mddraw->menuinfo.cbSize = sizeof(MENUITEMINFO);
	  mddraw->menuinfo.fMask = MIIM_TYPE|MIIM_ID|MIIM_STATE;
	  mddraw->menuinfo.fState = MFS_ENABLED;
	  mddraw->menuinfo.fType = MFT_STRING;

      mddraw->menuinfo.wID = SC_ZOOM_50;
      mddraw->menuinfo.dwTypeData = "Zoom 50%";
      InsertMenuItem(mddraw->menu, mddraw->mcount-1, 1, &mddraw->menuinfo);

      mddraw->menuinfo.wID = SC_ZOOM_75;
      mddraw->menuinfo.dwTypeData = "Zoom 75%";
      InsertMenuItem(mddraw->menu, mddraw->mcount, 1, &mddraw->menuinfo);

      mddraw->menuinfo.wID = SC_ZOOM_100;
      mddraw->menuinfo.dwTypeData = "Zoom 100%";
      InsertMenuItem(mddraw->menu, mddraw->mcount+1, 1, &mddraw->menuinfo);

     /* mddraw->menuinfo.wID = SC_ZOOM_200;
      mddraw->menuinfo.dwTypeData = "Zoom 200%";
      InsertMenuItem(mddraw->menu, mddraw->mcount+1, 1, &mddraw->menuinfo);

      mddraw->menuinfo.wID = SC_ZOOM_400;
      mddraw->menuinfo.dwTypeData = "Zoom 400%";
      InsertMenuItem(mddraw->menu, mddraw->mcount+2, 1, &mddraw->menuinfo);*/

#if 0
      mddraw->menuinfo.wID = MU_DDSTATE_DISABLE;
      mddraw->menuinfo.dwTypeData = "Disengage";
      InsertMenuItem(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);
#endif

      mddraw->menuinfo.fType = MFT_SEPARATOR;
      InsertMenuItem(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);

#endif
#endif

    // create directdraw interface
    //GUID iid = IID_IDirectDraw7;
    //if (FAILED(DirectDrawCreateEx(0, (void*)&mddraw->lpDD, &iid, 0)))
    if (FAILED(DirectDrawCreate(0, &mddraw->lpDD, 0)))
    	return 0;

    // enter cooperative mode
    if (FAILED(IDirectDraw_SetCooperativeLevel(mddraw->lpDD, mddraw->wnd, DDSCL_NORMAL|DDSCL_ALLOWREBOOT)))
    	return 0;

    // primary with no back buffers
	DDSURFACEDESC descriptor;
    descriptor.dwSize  = sizeof(descriptor);
    descriptor.dwFlags = DDSD_CAPS;//| DDSD_BACKBUFFERCOUNT;
    descriptor.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;//| DDSCAPS_LIVEVIDEO;//DDSCAPS_FLIP/* | DDSCAPS_COMPLEX*/;
    //descriptor.dwBackBufferCount = 1;
    if (FAILED(IDirectDraw_CreateSurface(mddraw->lpDD, &descriptor, &mddraw->lpDDS, 0)))
    	return 0;

    // create secondary surface
    descriptor.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN/*|DDSCAPS_FRONTBUFFER*/;
    descriptor.dwWidth = width;
    descriptor.dwHeight = height;

    if (FAILED(IDirectDraw_CreateSurface(mddraw->lpDD, &descriptor, &mddraw->lpDDS_secondary, 0)))
    	return 0;


#ifdef __MDD_CLIPPER__
     // create clipper
     if (FAILED(IDirectDraw_CreateClipper(mddraw->lpDD, 0, &mddraw->lpDDC, 0)))
    	return 0;

     // set clipper to window
     if (FAILED(IDirectDrawClipper_SetHWnd(mddraw->lpDDC, 0, mddraw->wnd)))
     	return 0;

     // attach clipper object to primary surface
     if (FAILED(IDirectDrawSurface_SetClipper(mddraw->lpDDS, mddraw->lpDDC)))
     	return 0;
#endif

    // set back to secondary
    mddraw->lpDDS_back = mddraw->lpDDS_secondary;

    // get pixel format
	DDPIXELFORMAT format;
    format.dwSize = sizeof(format);
    if (FAILED(IDirectDrawSurface_GetPixelFormat(mddraw->lpDDS, &format)))
    	return 0;

    // check format is direct colour
    if (!(format.dwFlags & DDPF_RGB))
    	return 0;

    // request converter function

//#if defined(NONAMELESSUNION)
#if !defined(WIN64)
//#ifdef _WIN32
    mddraw->convert = request_converter(format.u1.dwRGBBitCount, format.u2.dwRBitMask, format.u3.dwGBitMask, format.u4.dwBBitMask);
#else
	mddraw->convert = request_converter(format.dwRGBBitCount, format.dwRBitMask, format.dwGBitMask, format.dwBBitMask);
#endif
	if (!mddraw->convert)
		return 0;

	mddraw->status = 1;

    return (intptr_t)mddraw->wnd;
}


int directDraw_update (TMYLCDDDRAW *mddraw, void *buffer, size_t bufferLength)
{
	//const double t0 = getTicksD();

	// restore surfaces
	IDirectDrawSurface_Restore(mddraw->lpDDS);
	IDirectDrawSurface_Restore(mddraw->lpDDS_secondary);
	
     // lock back surface
    DDSURFACEDESC descriptor = {.dwSize = sizeof(descriptor)};
	if (FAILED(IDirectDrawSurface_Lock(mddraw->lpDDS_back,0,&descriptor,DDLOCK_DONOTWAIT,0)))
		return 0;

	uint8_t *restrict src = (uint8_t*)buffer;
	uint8_t *restrict dst = (uint8_t*)descriptor.lpSurface;

#if 1
	if (!(mddraw->des.x&15)){
		l_memcpy(dst, src, bufferLength);
	}else{
		const int src_pitch = mddraw->des.x << 2;

//#if defined(_WIN64) /*fix me*/
//		const int dst_pitch = descriptor.DUMMYUNIONNAMEN(1).lPitch;
//#elif defined(NONAMELESSUNION)
#if !defined(WIN64)
//#ifdef _WIN32
		const int dst_pitch = descriptor.u1.lPitch;
#else
		const int dst_pitch = descriptor.lPitch;
#endif
		for (int y = 0; y < mddraw->des.y; y++){
			mddraw->convert(src, dst, mddraw->des.x);
			src += src_pitch;
			dst += dst_pitch;
		}
	}
#else
	l_memcpy(dst, src, bufferLength);
#endif

	// unlock back surface
	IDirectDrawSurface_Unlock(mddraw->lpDDS_back, descriptor.lpSurface);
	paint_primary(mddraw);
	//const double t1 = getTicksD();
	//printf("directDraw_update %.4f\n", t1-t0);

    return 1;
}

void directDraw_close (TMYLCDDDRAW *mddraw)
{
	if (mddraw == NULL)
		return;
#if __MDD_SYSTEM_MENU__
	if (mddraw->menu){
		DestroyMenu(mddraw->menu);
		mddraw->menu = NULL;
	}
#endif
    // check secondary
    if (mddraw->lpDDS_secondary){
        // release secondary
        IDirectDrawSurface_Release(mddraw->lpDDS_secondary);
        mddraw->lpDDS_secondary = NULL;
    }

    // check
    if (mddraw->lpDDS){
        // release primary
        IDirectDrawSurface_Release(mddraw->lpDDS);
        mddraw->lpDDS = NULL;
    }

    // check
    if (mddraw->lpDD){
        // leave display mode
        IDirectDraw_RestoreDisplayMode(mddraw->lpDD);

        // leave exclusive mode
        IDirectDraw_SetCooperativeLevel(mddraw->lpDD, mddraw->wnd, DDSCL_NORMAL);

        // free direct draw
        IDirectDraw_Release(mddraw->lpDD);
        mddraw->lpDD = NULL;
    }
    DestroyWindow(mddraw->wnd);
    if (mddraw->open.eventTrigger)
    	CloseHandle(mddraw->open.eventTrigger);
}

unsigned int __stdcall winDDrawMessageThread (void *ptr)
{
	//printf("winDDrawMessageThread start\n");

	TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)ptr;

	if (directDraw_open(mddraw, mddraw->open.name, mddraw->open.width, mddraw->open.height)){
		SetEvent(mddraw->open.eventTrigger);
		MSG message;

		while (!mddraw->open.close && GetMessage(&message, mddraw->wnd, 0, 0) > 0){
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		directDraw_close(mddraw);
	}else{
		mddraw->wnd = NULL;
		SetEvent(mddraw->open.eventTrigger);
	}

	//printf("winDDrawMessageThread exit\n");
	_endthreadex(1);
	return 1;
}

int directDraw_create (TMYLCDDDRAW *mddraw, const char *title, const int width, const int height)
{
	mddraw->open.width = width;
	mddraw->open.height = height;
	strncpy(mddraw->open.name, title, 255);

	mddraw->open.eventTrigger = CreateEvent(NULL, 0, 0, NULL);
	mddraw->hThread = (HANDLE)_beginthreadex(NULL, 0, winDDrawMessageThread, mddraw, 0/*CREATE_SUSPENDED*/, &mddraw->tid);
	WaitForSingleObject(mddraw->open.eventTrigger, INFINITE);
	return mddraw->wnd != NULL;
}

void releaseDDLib ()
{
	//if (library)
	//	FreeLibrary(library);
	//library = NULL;

#ifdef __MDD_ALLOW_CLOSE__
	windowTotal = 0;
#endif
}


#endif
