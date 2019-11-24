
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



#include "../utils/utils.h"
#include "mylcdgl.h"


static int handleMessageQueue ();
static void ShowWindowSetState (int state);


static int G_flags = 0;
static HWND hWnd;
static HDC hDC;
static HDC hdc;
static HGLRC hRC;
static HBITMAP m_hbmOld;
static HBITMAP m_hbmp;	
static int currentWinState = 0;

static volatile int once = 0;
static unsigned char *data = NULL;
static TFRAME *glframe = NULL;
///static TFRAME *updateFrame = NULL;




static int padTo4 (const int width)
{
	if (width&3)
		return ((width>>2)<<2) + 4;
	else
		return width;
}

static void setPixel16_NB (const TFRAME *frm, const int x, const int row, const int value)
{
	*(uint16_t*)(frm->pixels+(row+(x<<1))) = value;
}

void getGLFrame (unsigned char *data, TFRAME *frame, int glwidth, int glheight)
{
	if (!data || !frame)
		return;

	int r,g,b,x,y,row;
	int pad = padTo4(glwidth);
	if (pad)
		pad -= glwidth;

	glReadPixels(0, 0, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, data);

	if (frame->bpp == LFRM_BPP_12){
		for (y = frame->height-1; y>= 0; y--){
			for (x = 0; x < frame->width; x++){
				r = (*data++&0xF0)<<4;
				g = (*data++&0xF0);
				b = (*data++&0xF0)>>4;
				lSetPixel_NB(frame, x, y, r|g|b);
			}
			data += pad;
		}
	}else if (frame->bpp == LFRM_BPP_16){
		for (y = frame->height-1; y>= 0; y--){
			row = y*frame->pitch;
			for (x = 0; x < frame->width; x++){
				r = (*data++&0xF8)<<8;
				g = (*data++&0xFC)<<3;
				b = (*data++&0xF8)>>3;
				//lSetPixel_NB(frame, x, y, r|g|b);
				setPixel16_NB(frame, x, row, r|g|b);
			}
			data += pad;
		}
	}else if (frame->bpp == LFRM_BPP_24){
		for (y = frame->height-1; y>= 0; y--){
			for (x = 0; x < frame->width; x++){
				r = (*data++)<<16;
				g = (*data++)<<8;
				b = (*data++);
				lSetPixel_NB(frame, x, y, r|g|b);
			}
			data += pad;
		}
	}else if (frame->bpp == LFRM_BPP_32){
		for (y = frame->height-1; y>= 0; y--){
			for (x = 0; x < frame->width; x++){
				r = (*data++)<<16;
				g = (*data++)<<8;
				b = (*data++);
				//a = (*data++);
				lSetPixel_NB(frame, x, y, r|g|b);
			}
			data += pad;
		}
	}else{
		for (y = frame->height-1; y>= 0; y--){
			for (x = 0; x < frame->width; x++){
				r = (*data++&0xE0);
				g = (*data++&0xE0)>>3;
				b = (*data++&0xC0)>>6;
				lSetPixel_NB(frame, x, y, r|g|b);
			}
			data += pad;
		}
	}
}

void copyAreaScaled (TFRAME *from, TFRAME *to, int src_x, int src_y, int src_width, int src_height, int dest_x, int dest_y, int dest_width, int dest_height)
{
	int x,y;
	float x2=0,y2=0;
	float scalex = (float)dest_width / (float)src_width;
	float scaley = (float)dest_height / (float)src_height;

	for (y=dest_y; y<dest_y+dest_height; y=y+(float)1.0 ){
		y2 = (float)src_y+(float)(y-dest_y)/scaley;
		for (x=dest_x; x<dest_x+dest_width; x=x+(float)1.0){
			x2 = (float)src_x+(float)(x-dest_x)/scalex;
            lSetPixel(to, x, y, lGetPixel(from,(int)x2,(int)y2));
		}
	}
}

PCHAR* CommandLineToArgvA ( PCHAR CmdLine, int* _argc)
{
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED, i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( (a = CmdLine[i]) ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
}

int mylcdgl_init (int flags)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char buffer[MAX_PATH+16];

	G_flags = flags;
	
	int argc = 0;
	char **argv = CommandLineToArgvA(GetCommandLine(), &argc);
	_splitpath(argv[0], drive, dir, NULL, NULL);
	sprintf(buffer, "%s%sconfig.cfg", drive, dir);
	
	if (!utilInitConfig("config.cfg"))
		return 0;

	if (G_flags&MYGL_CREATECONTEXT){
		openglInit(flags, DWIDTH, DHEIGHT);
		mylcdgl_SetWindowTitle("OpenGL");
	}
		
    once = 2;
	//updateFrame = lCloneFrame(frame);
	glframe = lCloneFrame(frame);
	data = (unsigned char *)malloc((padTo4(frame->width) * frame->height)*4);
	return data != NULL;
}

void mylcdgl_SetWindowTitle (const char *title)
{
	if (title)
		SetWindowText(hWnd, title);
}

void mylcdgl_SetSize (int width, int height)
{
    DWIDTH = padTo4(width);
    DHEIGHT = height;
}

void mylcdgl_sleep (int ms)
{
	lSleep(ms);
}


static void drawCursorTriangle (TFRAME *frame, int csize, int x, int y)
{
	y += 1;
	lDrawTriangleFilled(frame, x, y, x, y+csize, x+csize, y+csize, 0xFFFF);
	y--;
	csize++;
	lDrawTriangle(frame, x, y, x, y+csize, x+csize, y+csize, 0x0000);
}

static void drawCursorCrosshair (TFRAME *frame, int csize, int x, int y)
{
	int width = csize>>1;
	lDrawRectangleFilled(frame, x-width, y-1, x+width, y+1, frame->hw->render->backGround);
	lDrawRectangleFilled(frame, x-1, y-width, x+1, y+width, frame->hw->render->backGround);
	width--;
	lDrawLine(frame, x-width, y, x+width, y, frame->hw->render->foreGround);
	lDrawLine(frame, x, y-width, x, y+width, frame->hw->render->foreGround);
}

static int mylcdgl_render (TFRAME *des, int flags, int x, int y)
{

    if (!once){
		once = 1;
		mylcdgl_init(flags);
		return 1;
    }
    if (!(once&0x02)) return 0;

	int vidWidth = DWIDTH;
	int vidHeight = DHEIGHT;

	if (glframe->width != vidWidth || glframe->height != vidHeight){
		lResizeFrame(glframe,  vidWidth, vidHeight, 0);
		//lClearFrame(glframe);
		
		if (data) free(data);
		data = (unsigned char *)malloc((padTo4(vidWidth) * vidHeight)*4);
	}
	
	if (glframe->width != des->width || glframe->height != des->height){
		getGLFrame(data, glframe, vidWidth, vidHeight);
		copyAreaScaled(glframe, des, 0, 0, glframe->width, glframe->height, 0, 0, des->width, des->height);
		if (x >= 0) x *= (float)((float)des->width/(float)glframe->width);
		if (y >= 0) y *= (float)((float)des->height/(float)glframe->height);
	}else{
		getGLFrame(data, des, vidWidth, vidHeight);
	}
	
	if (flags&MYGL_DRAWCROSSHAIR)
		drawCursorCrosshair(des, 16, x, y);
	else if (flags&MYGL_DRAWCURSOR)
		drawCursorTriangle(des, 16, x, y);
 
//	lRefreshAsync(frame, 1);

	if (flags&MYGL_CREATECONTEXT){
		if (flags&MYGL_SHOWWINDOW)
			ShowWindowSetState(1);
		if (flags&MYGL_MESSAGEQUEUE)
			handleMessageQueue();
		if (currentWinState)
			SwapBuffers(hDC);
	}
	return 1;
}

int mylcdgl_draw (TFRAME *frame, int flags, int x, int y)
{
	return mylcdgl_render(frame, flags, x, y);
}

int mylcdgl_update (int flags, int x, int y)
{
	int ret = mylcdgl_render(frame, flags, x, y);
	if (ret)
		//lRefreshAsync(frame, 1);
		lRefresh(frame);
	return ret;
}

int mylcdgl_setoption (int option, intptr_t *value)
{
	int disp = lDriverNameToID(hw, "USBD480", LDRV_DISPLAY);
	if (disp)
		return lSetDisplayOption(hw, disp, option, value);
	return 0;
}

void mylcdgl_shutdown ()
{
	if (!(once&0x02)) return;
	
	if (G_flags&MYGL_CREATECONTEXT){
		DisableOpenGL(hWnd, hDC, hRC);
		if (m_hbmOld)
			SelectObject(hdc, m_hbmOld);
		if (m_hbmp)
			DeleteObject(m_hbmp);
		if (hdc)
			DeleteDC(hdc);
		DestroyWindow(hWnd);
	}
	if (data)
		free(data);
	//lDeleteFrame(updateFrame);
	lDeleteFrame(glframe);
	utilCleanup();
}

#if DRAW_TO_BITMAP
void *m_pBits;
#endif

void EnableOpenGL (HDC hdc, HDC * hDC, HGLRC * hRC)
{
	static PIXELFORMATDESCRIPTOR pfd;
	int format;

	
#if DRAW_TO_BITMAP
	*hDC = hdc;
	pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI;
#else
	*hDC = GetDC((HWND)hdc);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW| PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
#endif
	
	// set the pixel format for the DC
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(*hDC, &pfd);
	SetPixelFormat(*hDC, format, &pfd);
	
	// create and enable the render context (RC)
	*hRC = wglCreateContext(*hDC);
	wglMakeCurrent(*hDC, *hRC);
}


void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch (message){
		
	case WM_CREATE:
		return 0;
	case WM_CLOSE:
		PostQuitMessage( 0 );
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_KEYDOWN:
		switch ( wParam ){
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
		return 0;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
}

static void ShowWindowSetState (int state)
{	
	if (state != currentWinState){
		if (!state){
			currentWinState = 0;
			ShowWindow(hWnd, SW_HIDE);
		}else{
			currentWinState = 1;
			ShowWindow(hWnd, SW_SHOW);
		}
	}
}

static int handleMessageQueue ()
{
	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
		if (msg.message == WM_QUIT){
			return 0;
		}else{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
   	}
   	return 1;
}

int openglInit (int flags, int width, int height)
{
	WNDCLASS wc;
	RECT rect;
	
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = 0;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "tmp";
	RegisterClass(&wc);
	
	hWnd = CreateWindowEx(0, "tmp", "opengl", WS_POPUP|WS_CLIPSIBLINGS, 0, 0, width, height, NULL, NULL, 0, NULL);
	if (hWnd){
		GetClientRect(hWnd, &rect);
		MoveWindow(hWnd, 100, 100, (width-rect.right)+width, (height-rect.bottom)+height, 0);
	}
	
	//GetClientRect(hWnd, &rect);
	//printf("%i %i\n", rect.right, rect.bottom);
	
#if DRAW_TO_BITMAP
	BITMAPINFOHEADER BIH;
	memset(&BIH, 0, sizeof(BITMAPINFOHEADER));
	BIH.biSize = sizeof(BITMAPINFOHEADER);
	BIH.biWidth = width;
	BIH.biHeight = height;
	BIH.biPlanes = 1;
	BIH.biBitCount = 24;
	BIH.biCompression = BI_RGB;

	hdc = CreateCompatibleDC(0);
	m_hbmp = CreateDIBSection(hdc, (BITMAPINFO*)&BIH, DIB_PAL_COLORS, &m_pBits, NULL, 0);
    m_hbmOld = SelectObject(hdc, m_hbmp);

	EnableOpenGL(hdc, &hDC, &hRC );
#else
	EnableOpenGL((HDC)hWnd, &hDC, &hRC );
#endif
    
	return 1;
}
