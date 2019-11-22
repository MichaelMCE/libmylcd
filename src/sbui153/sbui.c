
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

#if (__BUILD_SBUI__ && (__WIN32__ || WIN64))



#include "../memory.h"
#include "../frame.h"
#include "../utils.h"
#include "../fileio.h"
#include "../misc.h"
#include "../pixel.h"
#include "../device.h"
#include "../lstring.h"
#include "../convert.h"
#include "../sync.h"
#include "../image.h"
#include <windows.h>
#include <SBUI_153/SwitchBlade.h>
#include "sbui.h"
#include <setupapi.h>
#include <hidsdi.h>





#ifdef DEFINE_GUID
#undef DEFINE_GUID
#endif
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
DEFINE_GUID(GUID_INTERFACE_HID_SBUILCD, 0xf1416dc1, 0x9db4, 0x4b93, 0xb2, 0xdf, 0x7c, 0xa1, 0xf3, 0x56, 0x91, 0xe0);

static const int VendorID = SBUI_ISV_VID;
static const int ProductID = SBUI_ISV_PID;

#define ENABLE_SBUINOTFOUNDWARNING	0
#define ENABLE_ADDRNOTFOUND			0



#if !WIN64

DLL_DECLARE(HRESULT, , RzSBStart, (void));
DLL_DECLARE(HRESULT, , RzSBStop, (void));
DLL_DECLARE(HRESULT, , RzSBGestureEnable, (int, int));
DLL_DECLARE(HRESULT, , RzSBGestureSetNotification, (int, int));
DLL_DECLARE(HRESULT, , RzSBGestureSetOSNotification, (int, int));
DLL_DECLARE(HRESULT, , RzSBGestureSetCallback, (TouchpadGestureCallbackFunctionType));

#else


HRESULT _RzSBStart (void)
{
	return S_OK;
}

HRESULT _RzSBStop (void)
{
	return S_OK;
}

HRESULT _RzSBGestureEnable (int a, int b)
{
	return S_OK;
}

HRESULT _RzSBGestureSetNotification (int a, int b)
{
	return S_OK;
}

HRESULT _RzSBGestureSetOSNotification (int a, int b)
{
	return S_OK;
}

HRESULT _RzSBGestureSetCallback (TouchpadGestureCallbackFunctionType a)
{
	return S_OK;
}
#endif





typedef struct{
	char data[128];
}__attribute__((packed))TSBREPORT;


typedef struct{
	HIDP_CAPS capabilities;
	OVERLAPPED overlapped;
	HANDLE deviceHandle;
	HANDLE hRead;
	HANDLE hIOPort;
	ULONG InputReportBufferSize;
	int key;
	char devicePathname[1024];
	TSBREPORT report[4];
}TSBHID;

#include <pshpack2.h>

typedef struct {
	unsigned short op;
	unsigned short left;
	unsigned short top;
	unsigned short right;
	unsigned short bottom;
	unsigned short crc;
}TSB_IMAGEOP;

#include <poppack.h>


typedef struct{
	void *buffer[2];

	int flags;
	TSBGESTURE sbg;	
	pgesturecb gestCB;
	pdkcb dkCB;
	void *udata_ptr;

	HANDLE hDevDK;
	HANDLE hDevLCD;

	struct {
		struct {
			int x;
			int y;
		}offset;			// render offset applied to each key
		
		TLPOINTEX pos[10];	// precomputed location within LCD
	}keys;

	TTHRDSYNCCTRL s;
	//TTHRDSYNCCTRL sbWriteLock;
	HANDLE sbGlobalWriteLock;
	
	TTHRDSYNCCTRL hDKThread;
	int threadState;
	TSBHID hid;
	
	uint64_t t0;
}TSBCTX;


// we need to do this because RzSBGestureSetCallback() does not provide us with a context(user) pointer
static TSBCTX *g_sbctx = NULL;


static int sbui_OpenDisplay (TDRIVER *drv);
static int sbui_CloseDisplay (TDRIVER *drv);
static int sbui_Clear (TDRIVER *drv);
static int sbui_Refresh (TDRIVER *drv, TFRAME *frm);
static int sbui_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int sbui_getOption (TDRIVER *drv, int option, intptr_t *value);
static inline int sbui_setOption (TDRIVER *drv, int option, intptr_t *value);
unsigned int __stdcall sb_dynamicKeyHandler (TSBCTX *sbctx);


static inline int sbWriteLock (TSBCTX *sbctx)
{
	//lock(&sbctx->sbWriteLock);
	return WaitForSingleObject(sbctx->sbGlobalWriteLock, 100) == WAIT_OBJECT_0;
}

static inline void sbWriteRelease (TSBCTX *sbctx)
{
	//unlock(&sbctx->sbWriteLock);
	ReleaseMutex(sbctx->sbGlobalWriteLock);
}


static inline int sbHid_open (TSBHID *hid)
{
	//printf("sbHid_open\n");
	
	hid->hRead = hid->deviceHandle = CreateFile(hid->devicePathname, MAXIMUM_ALLOWED, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hid->hRead)
		hid->hIOPort = CreateIoCompletionPort(hid->hRead, hid->hIOPort, (ULONG_PTR)&hid->key, 0);
		
	//printf("sbHid_open %i %i '%s'\n", (int)hid->hIOPort, (int)hid->hRead, hid->devicePathname);
		
	return (hid->hRead != NULL) && (hid->hIOPort != NULL);
}

static inline void sbHid_close (TSBHID *hid)
{
	//printf("sbHid_close in %i %i\n", (int)hid->hIOPort, (int)hid->hRead);
	
	if (hid->hIOPort){
		PostQueuedCompletionStatus(hid->hIOPort, 0, (ULONG_PTR)0, 0);
		CloseHandle(hid->hIOPort);
		hid->hIOPort = NULL;
	}
	if (hid->hRead){
		CloseHandle(hid->hRead);
		hid->hRead = NULL;
		hid->deviceHandle = NULL;
	}
	//printf("sbHid_close out\n");
}

static inline int sbGetDevicePath (const GUID *guid, wchar_t *buffer)
{

 	int contLoop = 0;
	int memberIndex = 0;
	DWORD RequiredSize = 0;
	DWORD Size = 0;

	SP_DEVICE_INTERFACE_DATA DID;
	DID.cbSize = sizeof(DID);
	SP_DEVICE_INTERFACE_DETAIL_DATA_W *DIDD = NULL;
	
	HDEVINFO *hDevInfo = SetupDiGetClassDevsW(guid, 0, 0, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
	if (hDevInfo == NULL)
		return 0;

	do{
    	contLoop = (int)SetupDiEnumDeviceInterfaces(hDevInfo, 0, guid, memberIndex, &DID);
		if (contLoop){
    		SetupDiGetDeviceInterfaceDetailW(hDevInfo, &DID, 0, 0, &RequiredSize, 0);
    		if (DIDD) l_free(DIDD);
			DIDD = l_calloc(1, RequiredSize);
			if (DIDD == NULL) return 0;
			
			DIDD->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
    		Size = RequiredSize;
    		SetupDiGetDeviceInterfaceDetailW(hDevInfo, &DID, DIDD, Size, &RequiredSize, 0);

			//wprintf(L"found \"%s\"\n", DIDD->DevicePath);
			wcsncpy(buffer, DIDD->DevicePath, wcslen(DIDD->DevicePath));
			l_free(DIDD);
			return ++memberIndex;
    	}
		memberIndex++;
	}while(contLoop);

	if (DIDD) l_free(DIDD);
	SetupDiDestroyDeviceInfoList(hDevInfo);
	
	return 0;
}

static inline void dll_load_error (const char *fn, const char *dll, const int err)
{
#if ENABLE_ADDRNOTFOUND
	char buffer[MAX_PATH+1];
	
	if (err == -1)
		snprintf(buffer, MAX_PATH, "LoadLibrary(): File not found\n\n'%s'", dll);
	else if (err == -2)
		snprintf(buffer, MAX_PATH, "GetProcAddress(): Symbol '%s' not found in '%s'", fn, dll);
	else
		return;

	MessageBoxA(NULL, buffer, dll, MB_SYSTEMMODAL|MB_ICONSTOP|MB_OK);
#endif
}

static inline int sbuiLoadDll (const char *lib)
{
#if !WIN64
	DLL_LOAD(lib, RzSBStart, 1);
	DLL_LOAD(lib, RzSBStop, 1);
	DLL_LOAD(lib, RzSBGestureEnable, 1);
	DLL_LOAD(lib, RzSBGestureSetNotification, 1);
	DLL_LOAD(lib, RzSBGestureSetOSNotification, 1);
	DLL_LOAD(lib, RzSBGestureSetCallback, 1);
#endif
	return 1;
}

int sbui153Init (TREGISTEREDDRIVERS *rd)
{
	if (sbuiLoadDll(SBSDKDLL) <= 0)
		return 0;

	g_sbctx = NULL;
	TDISPLAYDRIVER dd;
	l_strcpy(dd.comment, "SwitchBlade (direct access)");
		
	dd.open = sbui_OpenDisplay;
	dd.close = sbui_CloseDisplay;
	dd.clear = sbui_Clear;
	dd.refresh = sbui_Refresh;
	dd.refreshArea = sbui_RefreshArea;
	dd.setOption = sbui_setOption;
	dd.getOption = sbui_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 7;
	
	l_strcpy(dd.name, "SWITCHBLADEFIO");		// switchblade direct access
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_GESTURECB, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_DKCB, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_UDATAPTR, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_GESTURECFG, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_SETDK, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_RECONNECT, 0);
	
	return (dnumber > 0);
}

void sbui153Close (TREGISTEREDDRIVERS *rd)
{
	//printf("sbui153Close\n");
	g_sbctx = NULL;
}

static inline void * drvToCtx (TDRIVER *drv)
{	
	if (drv){
		TSBCTX *sbctx = NULL;
		drv->dd->getOption(drv, lOPT_SBUI_STRUCT, (void*)&sbctx);
//		printf("drvToCtx a: %p\n", sbctx);
		return sbctx;
	}else{
//		printf("drvToCtx b: %p\n", g_sbctx);
		return (TSBCTX*)g_sbctx;
	}
}

static inline int rzsb_success (int ret, const wchar_t *str)
{
	if (RZSB_SUCCESS(ret)){
		return 1;
	}else{
#if ENABLE_SBUINOTFOUNDWARNING
		wchar_t buffer[4096];
		wchar_t *hLocal = NULL;

		int error = ret;
		int gle = GetLastError();
		if (gle) error = gle;
		
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&hLocal, 0, 0);
		LocalLock(hLocal);
		
		snwprintf(buffer, sizeof(buffer), 
			L"%s() failed with error 0x%X/%i (GLE 0x%X/%i)\n\n%s",
			str, ret, ret&0xFFFF, gle, gle&0xFFFF, hLocal
		);
		
		LocalFree(hLocal);
		
		if (gle == ERROR_ALREADY_EXISTS)
			l_wcscat(buffer, L"\nNote:\nTry closing anything else using the device then try again.");


		MessageBoxW(NULL, buffer, L"SwitchBladeFIO", MB_SYSTEMMODAL|MB_ICONSTOP|MB_OK);
#endif
		return 0;
	}
}

static inline int sbui_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		switch (option){
		  case lOPT_SBUI_RSTATS:{
		  	//TSBUIRENDERSTATS *rs = (TSBUIRENDERSTATS*)value;
		  	//return rzsb_success(_RzSBWinRenderGetStats(&rs->count, &rs->maxTime, &rs->lastTime, &rs->averageTime), L"RzSBWinRenderGetStats");
		  	return 1;
		  }
		  case lOPT_SBUI_RSTATSRESET:
		  	//rzsb_success(_RzSBWinRenderResetStats(), L"RzSBWinRenderResetStats");
		  	return 1;
		  case lOPT_SBUI_SETDK:
		  	if (value) *value = 0;
		  	return 1;
		  case lOPT_SBUI_RECONNECT:
		  case lOPT_SBUI_STRUCT:
		  case lOPT_SBUI_GESTURECB:
		  case lOPT_SBUI_DKCB:
		  case lOPT_SBUI_UDATAPTR:{
			intptr_t *opt = drv->dd->opt;
			*value = opt[option];
			return 1;
		  }
		};
	}
	return 0;
}

static int sbui_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;

			//rzsb_success(_RzSBStop(), L"RzSBStop");

			TSBCTX *sbctx = (TSBCTX*)drvToCtx(drv);

			if (sbctx){
				sbctx->flags = 0;
				sbHid_close(&sbctx->hid);
				drv->dd->setOption(drv, lOPT_SBUI_UDATAPTR, NULL);
				drv->dd->setOption(drv, lOPT_SBUI_GESTURECB, NULL);

				if (sbctx->threadState){
					sbctx->threadState = 0;
					joinThread(&sbctx->hDKThread);
					closeThreadHandle(&sbctx->hDKThread);
				}

				if (sbctx->hDevDK) CloseHandle(sbctx->hDevDK);
				if (sbctx->hDevLCD) CloseHandle(sbctx->hDevLCD);

				sbctx->hDevDK = NULL;
				sbctx->hDevLCD = NULL;
				lock_delete(&sbctx->s);
				//lock_delete(&sbctx->sbWriteLock);
				CloseHandle(sbctx->sbGlobalWriteLock);

				l_free(sbctx->buffer[0]);
				l_free(sbctx->buffer[1]);
				l_free(sbctx);
				g_sbctx = NULL;
			}
			rzsb_success(_RzSBStop(), L"RzSBStop");
			return 1;
		}
	}
	return 0;
}

static CALLBACK HRESULT touchpadGestureCB (RZSDKGESTURETYPE type, DWORD params, WORD x, WORD y, WORD z)
{
	//printf("touchpadGestureCB: %i %i %i %i %i\n", (int)type, (int)params, (int)x, (int)y, (int)z);
	
	if (!ValidGesture(type)){
		printf("..invalid gesture\n");
		return S_OK;
	}

	TSBCTX *sbctx = (TSBCTX*)drvToCtx(NULL/*drv*/);
	if (sbctx){
		if (lock(&sbctx->s)){
			TSBGESTURE *sbg = &sbctx->sbg;

			sbg->timePrev = sbg->time;
			sbg->time = getTicksD();
			sbg->dt = sbg->time - sbg->timePrev;
			sbg->ct++;
			sbg->type = type;
			sbg->params = params;
			sbg->x = x;
			sbg->y = y;
			sbg->z = z;

			if (sbctx->gestCB)
				sbctx->gestCB(sbg, sbctx->udata_ptr);
			
			if (type == RZGESTURE_PRESS && !params) sbg->id++;
			if (type == RZGESTURE_TAP && params) sbg->id++;
			if (type == RZGESTURE_FLICK && params) sbg->id++;
			
			unlock(&sbctx->s);
		}
	}
	
	return S_OK;
}

static inline int sbuiStart (TDRIVER *drv, TSBCTX *sbctx)
{
	HRESULT ret = _RzSBStart();
	//printf("sbuiStart %i\n", (int)ret);
	
	if (rzsb_success(ret, L"RzSBStart")){
		wchar_t path[512] = {0};
		if (sbGetDevicePath(&GUID_INTERFACE_HID_SBUILCD, path)){

			if (sbctx->hDevDK) CloseHandle(sbctx->hDevDK);
			wcscat(path, L"\\02");	// dk lcd
			sbctx->hDevDK = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			//wprintf(L"sbuiStart dklcd %p '%s\n", sbctx->hDevDK, path);


			if (sbctx->hDevDK)
				sbctx->flags |= SBUI_CTX_CANRENDER_KEYLCD;
			else
				sbctx->flags &= ~SBUI_CTX_CANRENDER_KEYLCD;

			if (sbctx->hDevLCD) CloseHandle(sbctx->hDevLCD);
			
			//wcscat(path, L"\\01");	// touchpad lcd
			path[wcslen(path)-1] = '1';
			sbctx->hDevLCD = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			//wprintf(L"sbuiStart padlcd %p '%s\n", sbctx->hDevLCD, path);
			
			if (sbctx->hDevLCD)
				sbctx->flags |= SBUI_CTX_CANRENDER_PADLCD;
			else
				sbctx->flags &= ~SBUI_CTX_CANRENDER_PADLCD;
	
	
			// precompute key image rects/locations
			const int imageSpace = 48;
			const int dkWidth = SBUI_DK_WIDTH;
			const int dkHeight = SBUI_DK_HEIGHT;
			const int col = 3;
			const int row1 = 150;
			const int row2 = row1 + dkHeight + imageSpace-1;

			for (int i = 0; i < 5; i++){
				sbctx->keys.pos[i].x1 = col + ((imageSpace+dkWidth)*i);
				sbctx->keys.pos[i].y1 = row2;
				sbctx->keys.pos[i].x2 = sbctx->keys.pos[i].x1 + dkWidth-1;
				sbctx->keys.pos[i].y2 = sbctx->keys.pos[i].y1 + dkHeight-1;
			}
			for (int i = 5; i < 10; i++){
				sbctx->keys.pos[i].x1 = col + ((imageSpace+dkWidth)*(i-5));
				sbctx->keys.pos[i].y1 = row1;
				sbctx->keys.pos[i].x2 = sbctx->keys.pos[i].x1 + dkWidth-1;
				sbctx->keys.pos[i].y2 = sbctx->keys.pos[i].y1 + dkHeight-1;
			}
			sbctx->keys.offset.x = 0;
			sbctx->keys.offset.y = 0;			
		}

		const int flags = RZGESTURE_PRESS|RZGESTURE_TAP|RZGESTURE_FLICK|RZGESTURE_ZOOM|RZGESTURE_ROTATE;
		rzsb_success(_RzSBGestureEnable(flags, 0), L"RzSBGestureEnable");
		rzsb_success(_RzSBGestureSetNotification(flags, 0), L"RzSBGestureSetNotification");
		rzsb_success(_RzSBGestureSetOSNotification(flags, 0), L"RzSBGestureSetOSNotification");
		rzsb_success(_RzSBGestureSetCallback(touchpadGestureCB), L"RzSBGestureSetCallback");

		if (sbctx)
			sbctx->t0 = GetTickCount();
		return sbctx->hDevLCD != NULL;
	}
	return 0;
}

static inline unsigned short swap16 (const unsigned short src)
{
	unsigned char *tmp = (unsigned char*)&src;
	return (tmp[0] << 8) | tmp[1];
}

static inline int sbWriteImage (TSBCTX *sbctx, HANDLE des, TSB_IMAGEOP *idesc, void *pixelData)
{
	
	idesc->op = 1;		// is always 1
	idesc->crc = 0;
	
	TSB_IMAGEOP	rect = *idesc;
	//l_memcpy(&rect, idesc, sizeof(rect));
	
	unsigned short *header = (unsigned short*)idesc;
	for (int i = 0; i < 5; i++){
		header[i] = swap16(header[i]);
		idesc->crc ^= header[i];
	}
	
	DWORD bwritten = 0;
	DWORD len = sizeof(TSB_IMAGEOP);
	
	if (sbWriteLock(sbctx)){
		/*int reta = */WriteFile(des, header, len, &bwritten, NULL);

		len = ((rect.right - rect.left)+1) * ((rect.bottom - rect.top)+1) * 2;
		/*int retb = */WriteFile(des, pixelData, len, &bwritten, NULL);
				
		sbWriteRelease(sbctx);
		//if (sbctx->hDevDK == des)
			//printf("sbWriteImage %i %i %i %i\n", reta, retb, (int)bwritten, (int)len);
	}
	return (int)bwritten;
}

static inline int sbui_setDkImage (TDRIVER *drv, TSBCTX *sbctx, TSBGESTURESETDK *sbdk)
{
	//printf("sbui_setDkImage %i\n", sbdk->op);
	
	if (sbdk->op == SBUI_SETDK_FILE){
		int ret = 0;
		TFRAME *img = newImage(drv->dd->hw, sbdk->u.file.path, LFRM_BPP_32);
		if (img){
			TSBGESTURESETDK tmp;
			tmp.size = sbdk->size;
			tmp.op = SBUI_SETDK_IMAGE;
			tmp.u.image.key = sbdk->u.file.key;
			tmp.u.image.image = img;
			ret = sbui_setDkImage(drv, sbctx, &tmp);
			deleteFrame(img);
		}
		return ret;
		
	}else if (sbdk->op == SBUI_SETDK_IMAGE){
		if (sbctx->flags&SBUI_CTX_CANRENDER_KEYLCD){
			TFRAME *img = sbdk->u.image.image;
			int dk = sbdk->u.image.key-1;
						
			
			int x = sbctx->keys.pos[dk].x1 + (((sbctx->keys.pos[dk].x2 - sbctx->keys.pos[dk].x1)+1) / 2);
			x -= (img->width/2);
			
			int y = sbctx->keys.pos[dk].y1 + (((sbctx->keys.pos[dk].y2 - sbctx->keys.pos[dk].y1)+1) / 2);
			y -= (img->height/2);
			
			TSB_IMAGEOP	idesc;
			idesc.left = sbctx->keys.offset.x + x;
			idesc.top = sbctx->keys.offset.y + y;
			idesc.right = idesc.left + img->width-1;
			idesc.bottom = idesc.top + img->height-1;

			/*idesc.left = sbctx->keys.pos[dk].x1 + sbctx->keys.offset.x;
			idesc.top = sbctx->keys.pos[dk].y1 + sbctx->keys.offset.y;
			idesc.right = sbctx->keys.pos[dk].x1 + img->width-1 + sbctx->keys.offset.x;
			idesc.bottom = sbctx->keys.pos[dk].y1 + img->height-1 + sbctx->keys.offset.y;*/

			//printf("%i %i %i\n", dk, idesc.left, idesc.right);

			unsigned short *data;
			if (img->bpp == LFRM_BPP_16){
				data = (unsigned short*)img->pixels;
			}else{
				data = (unsigned short*)sbctx->buffer[1];
				
				pConverterFn converter = getConverter(img->bpp, LFRM_BPP_16);
				if (converter)
					converter(img, (void*)data);
			}
			
			sbWriteImage(sbctx, sbctx->hDevDK, &idesc, data);
		}
	}else if (sbdk->op == SBUI_SETDK_DIRECT){

		TFRAME *img = sbdk->u.direct.image;
		TLPOINTEX *src = &sbdk->u.direct.src;	// source region within image [to copy from]
		T2POINT *des = &sbdk->u.direct.des;		// where to place the pixels
		const int width = abs(src->x2 - src->x1)+1;
		const int height = abs(src->y2 - src->y1)+1;
		
		TFRAME *frame = _newFrame(img->hw, width, height, img->groupId, img->bpp);
		if (!frame) return 0;
		
		int ydes = 0;
		for (int y = src->y1; y <= src->y2; y++){
			void *pX1 = l_getPixelAddress(img, src->x1, y);
			void *pX2 = l_getPixelAddress(frame, 0, ydes++);
			l_memcpy(pX2, pX1, frame->pitch);
		}

		unsigned short *data = NULL;
		if (frame->bpp == LFRM_BPP_16){
			data = (unsigned short*)frame->pixels;
		}else{
			data = (unsigned short*)sbctx->buffer[1];
				
			pConverterFn converter = getConverter(frame->bpp, LFRM_BPP_16);
			if (converter){
				converter(frame, (void*)data);
			}
		}

		TSB_IMAGEOP	idesc;
		idesc.left = des->x;
		idesc.top = des->y;
		idesc.right = des->x + frame->width-1;
		idesc.bottom = des->y + frame->height-1;;

		sbWriteImage(sbctx, sbctx->hDevDK, &idesc, data);
		
		deleteFrame(frame);
	}
	return 1;
}

static inline int sbui_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	//printf("sbui_setOption %i %p\n", option, value);
	
	if (!drv) return 0;
	intptr_t *opt = drv->dd->opt;
	
	if (option == lOPT_SBUI_STRUCT){
		opt[option] = (intptr_t)value;
		g_sbctx = (TSBCTX*)value;
		return 1;
		
	}else if (option == lOPT_SBUI_SETDK){
		TSBCTX *sbctx = (TSBCTX*)drvToCtx(drv);
		if (sbctx && value){
			TSBGESTURESETDK *sbdk = (TSBGESTURESETDK*)value;
			if (sbdk->size == sizeof(TSBGESTURESETDK)){
				opt[option] = (intptr_t)value;
				return sbui_setDkImage(drv, sbctx, sbdk);				
			}
		}
		return 0;
		
	}else if (option == lOPT_SBUI_RECONNECT){
		opt[option] = 0;
		
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			rzsb_success(_RzSBStop(), L"RzSBStop");
			sbHid_close(&sbctx->hid);
	
			if (sbctx->threadState){
				sbctx->threadState = 0;
				joinThread(&sbctx->hDKThread);
				closeThreadHandle(&sbctx->hDKThread);
			}
			int ret = sbuiStart(drv, sbctx);
			newThread(&sbctx->hDKThread, sb_dynamicKeyHandler, sbctx);
			return ret;
		}
		return 0;
	}else if (option == lOPT_SBUI_DKCB){
		opt[option] = (intptr_t)value;
		
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			sbctx->dkCB = (pdkcb)value;
			return 1;
		}
	}else if (option == lOPT_SBUI_GESTURECB){
		opt[option] = (intptr_t)value;
		
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			sbctx->gestCB = (pgesturecb)value;
			return 1;
		}
	}else if (option == lOPT_SBUI_RSTATSRESET){
	  	//rzsb_success(_RzSBWinRenderResetStats(), L"RzSBWinRenderResetStats");
	  	return 1;

	}else if (option == lOPT_SBUI_RSTATS){
		return 1;

	}else if (option == lOPT_SBUI_UDATAPTR){
		opt[option] = (intptr_t)value;
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			sbctx->udata_ptr = (void*)value;
			return 1;
		}
	}else if (option == lOPT_SBUI_GESTURECFG){
		if (value){
			const TSBGESTURECBCFG *sbcfg = (TSBGESTURECBCFG*)value;
			if (sbcfg && sbcfg->op && sbcfg->gesture){
				switch (sbcfg->op){
			  	case SBUICB_OP_GestureEnable:
					return rzsb_success(_RzSBGestureEnable(sbcfg->gesture, sbcfg->state), L"RzSBGestureEnable");
			  	case SBUICB_OP_GestureSetNotification:
					return rzsb_success(_RzSBGestureSetNotification(sbcfg->gesture, sbcfg->state), L"RzSBGestureSetNotification");
			  	case SBUICB_OP_GestureSetOSNotification:
					return rzsb_success(_RzSBGestureSetOSNotification(sbcfg->gesture, sbcfg->state), L"RzSBGestureSetOSNotification");
				};
			}
		}
	}
	return 0;
}

static int sbui_OpenDisplay (TDRIVER *drv)
{
	//printf("sbui_OpenDisplay %p\n", drv);
	
	if (!drv) return 0;

	if (drv->dd->status != LDRV_CLOSED)
		return 0;

	TSBCTX *sbctx = l_calloc(1, sizeof(TSBCTX));
	if (!sbctx) return 0;
	
	if (sbuiStart(drv, sbctx)){
		const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
		lock_create(&sbctx->s);
		//lock_create(&sbctx->sbWriteLock);
		sbctx->sbGlobalWriteLock = CreateMutexW(NULL, FALSE, L"RZ_GLOBAL_DISPLAY_LOCK");
		sbctx->sbg.time = getTicksD();
			
		sbctx->buffer[0] = (unsigned int*)l_malloc(fsize);
		sbctx->buffer[1] = (unsigned int*)l_malloc(fsize);
		if (sbctx->buffer[0] && sbctx->buffer[1]){
			newThread(&sbctx->hDKThread, sb_dynamicKeyHandler, sbctx);
			drv->dd->setOption(drv, lOPT_SBUI_STRUCT, (intptr_t*)sbctx);
			drv->dd->status = LDRV_READY;
			return 1;
		}
	}
	
	l_free(sbctx);
	drv->dd->status = LDRV_CLOSED;
	return 0;
}

static inline int sbui_update (TDRIVER *drv, TSBCTX *sbctx, TSB_IMAGEOP *idesc, void *buffer, const int fsize)
{
	if (sbctx->flags&SBUI_CTX_CANRENDER_PADLCD){
		return (sbWriteImage(sbctx, sbctx->hDevLCD, idesc, buffer) > 100);
	}else{
		// shouldn't get here but print an error anyways
		return 0;
	}
}

static int sbui_Clear (TDRIVER *drv)
{
	//printf("sbui_Clear\n");
	
	if (drv){
		if (drv->dd->status == LDRV_READY){
			TSBCTX *sbctx = (TSBCTX*)drvToCtx(drv);
			const int fsize = drv->dd->width * drv->dd->height * 2;
			l_memset(sbctx->buffer[0], drv->dd->clr, fsize);
			TSB_IMAGEOP	idesc;
			idesc.left = 0;
			idesc.top = 0;
			idesc.right = 799;
			idesc.bottom = 479;
			sbui_update(drv, sbctx, &idesc, sbctx->buffer[0], fsize);
			return 1;
		}
	}
	return 0;
}

static int sbui_Refresh (TDRIVER *drv, TFRAME *frame)
{
	if (!frame || !drv || drv->dd->status != LDRV_READY)
		return 0;

	TSBCTX *sbctx = drvToCtx(drv);
	if (sbctx){
		TSB_IMAGEOP	idesc;
		idesc.left = 0;
		idesc.top = 0;
		idesc.right = 799;
		idesc.bottom = 479;
		
		if (frame->bpp == LFRM_BPP_16){
			return sbui_update(drv, sbctx, &idesc, frame->pixels, frame->frameSize);
		}else{
			pConverterFn converter = getConverter(frame->bpp, LFRM_BPP_16);
			if (converter){
				converter(frame, sbctx->buffer[0]);
				const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
				return sbui_update(drv, sbctx, &idesc, sbctx->buffer[0], fsize);
			}
		}
	}	
	return 0;
}

// xn/yn = destination region within lcd
static int sbui_RefreshArea (TDRIVER *drv, TFRAME *frame, const int x1, const int y1, const int x2, const int y2)
{
	if (!frame || !drv || drv->dd->status != LDRV_READY)
		return 0;

	TSBCTX *sbctx = drvToCtx(drv);
	if (sbctx){
		TSB_IMAGEOP	idesc;
		idesc.left = x1;
		idesc.top = y1;
		idesc.right = x2;
		idesc.bottom = y2;
		
		if (frame->bpp == LFRM_BPP_16){
			int idescDataLen = (((y2-y1)+1) * ((x2-x1)+1) * 2);
			int datalen = MIN(frame->frameSize, idescDataLen);
			return sbui_update(drv, sbctx, &idesc, frame->pixels, datalen);
		}else{
			pConverterFn converter = getConverter(frame->bpp, LFRM_BPP_16);
			if (converter){
				converter(frame, sbctx->buffer[0]);
				const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
				return sbui_update(drv, sbctx, &idesc, sbctx->buffer[0], fsize);
			}
		}
	}	
	return 0;
}

static inline void getDeviceCapabilities (TSBHID *hid)
{
	PHIDP_PREPARSED_DATA PreparsedData;

	HidD_GetPreparsedData(hid->deviceHandle, &PreparsedData);
	HidP_GetCaps(PreparsedData, &hid->capabilities);

#if 0
	printf("%s%X\n", "Usage Page: ", hid->capabilities.UsagePage);
	printf("%s%d\n", "Input Report Byte length: ", hid->capabilities.InputReportByteLength);
	printf("%s%d\n", "Output Report Byte length: ", hid->capabilities.OutputReportByteLength);
	printf("%s%d\n", "Feature Report Byte length: ", hid->capabilities.FeatureReportByteLength);
	printf("%s%d\n", "Number of Link Collection Nodes: ", hid->capabilities.NumberLinkCollectionNodes);
	printf("%s%d\n", "Number of Input Button Caps: ", hid->capabilities.NumberInputButtonCaps);
	printf("%s%d\n", "Number of InputValue Caps: ", hid->capabilities.NumberInputValueCaps);
	printf("%s%d\n", "Number of InputData Indices: ", hid->capabilities.NumberInputDataIndices);
	printf("%s%d\n", "Number of Output Button Caps: ", hid->capabilities.NumberOutputButtonCaps);
	printf("%s%d\n", "Number of Output Value Caps: ", hid->capabilities.NumberOutputValueCaps);
	printf("%s%d\n", "Number of Output Data Indices: ", hid->capabilities.NumberOutputDataIndices);
	printf("%s%d\n", "Number of Feature Button Caps: ", hid->capabilities.NumberFeatureButtonCaps);
	printf("%s%d\n", "Number of Feature Value Caps: ", hid->capabilities.NumberFeatureValueCaps);
	printf("%s%d\n", "Number of Feature Data Indices: ", hid->capabilities.NumberFeatureDataIndices);
#endif
	HidD_FreePreparsedData(PreparsedData);
}

int sbHidDeviceEnable (TSBHID *hid, const int vid, const int pid)
{
	HIDD_ATTRIBUTES attributes;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL;
	GUID hidGuid;
	HANDLE hDevInfo;
	ULONG required;
	ULONG length = 0;
	LONG result;
	int lastDevice = 0;
	int memberIndex = 0;
	int deviceFound = 0; 

	devInfoData.cbSize = sizeof(devInfoData);
	hid->deviceHandle = NULL;

	HidD_GetHidGuid(&hidGuid);	
	hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
	
	do{
		result = SetupDiEnumDeviceInterfaces(hDevInfo, 0, &hidGuid, memberIndex, &devInfoData);
		if (result != 0){
			result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &length, NULL);
			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)l_calloc(1, length);
			if (detailData == NULL){
				//printf("libsb: calloc() returned NULL\n");
				return 0;
			}
			detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, length, &required, NULL);

			hid->hRead = hid->deviceHandle = CreateFile(detailData->DevicePath, MAXIMUM_ALLOWED, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, /*FILE_ATTRIBUTE_NORMAL*/FILE_FLAG_OVERLAPPED, NULL);
			if (hid->deviceHandle == NULL){
				//printf("libsb: CreateFile(): returned NULL\n");
				l_free(detailData);
				break;
			}
			
			attributes.Size = sizeof(attributes);
			result = HidD_GetAttributes(hid->deviceHandle, &attributes);
			deviceFound = 0;

			if ((attributes.VendorID == vid && attributes.ProductID == pid)){
				getDeviceCapabilities(hid);
				strncpy(hid->devicePathname, detailData->DevicePath, sizeof(hid->devicePathname)-1);
			
				if (strstr(hid->devicePathname, "&mi_01&col04")){		// dk's
					HidD_GetNumInputBuffers(hid->deviceHandle, &hid->InputReportBufferSize);

					deviceFound = 1;
					hid->key = 11;
					break;
				}
			}
			
			CloseHandle(hid->deviceHandle);
			l_free(detailData);
		}else{
			lastDevice = 1;
		}
		memberIndex = memberIndex + 1;
	}while(lastDevice == 0 && deviceFound == 0);

	SetupDiDestroyDeviceInfoList(hDevInfo);
	return deviceFound;
}

unsigned int __stdcall sb_dynamicKeyHandler (TSBCTX *sbctx)
{
	//printf("sb_dynamicKeyHandler START\n");

	sbctx->threadState = 1;
	TSBHID *hid = &sbctx->hid;
	
	if (!sbHidDeviceEnable(hid, VendorID, ProductID)) goto threadExit;
	if (!sbHid_open(hid)) goto threadExit;

	typedef struct{
		unsigned int stamp;
		unsigned int state;		// 1:pressed: 0:not pressed
		unsigned int sbId;
	}TSbDKState;	
	TSbDKState stateList[128];
	l_memset(stateList, 0, sizeof(stateList));


	int k = SBUI_DK_6;
	for (int i = '0'+32; i <= '4'+32; i++, k++) stateList[i].sbId = k;
	for (int i = '5'+32, k = SBUI_DK_1; i <= '9'+32; i++, k++) stateList[i].sbId = k;
	stateList[2].sbId = SBUI_DK_ACTIVATE;		// Razer key

	unsigned int stampId = 100;
	OVERLAPPED *poverlapped = NULL;
	DWORD len = 0;
	DWORD *pkey = NULL;
	DWORD bytesRead = 0;
	
	while (sbctx->threadState){
		/*int result = (int)*/ReadFile(hid->hRead, &hid->report, hid->capabilities.InputReportByteLength, &bytesRead, &hid->overlapped);
		if (!sbctx->threadState){
			break;
		}
		

		len = 0;
		int ret = GetQueuedCompletionStatus(hid->hIOPort, &len, (void*)&pkey, &poverlapped, INFINITE);
		if (ret){
			if (!sbctx->threadState) break;

			/*for (int i = 0; i < len; i++)
				printf("%X ", hid->report[0].data[i]);
			printf("\n");*/

			if (len != 16 || hid->report[0].data[0] != 4){
				l_sleep(1);
				break;
			}
			stampId++;

			for (int i = 1; i < len; i++){
				unsigned char key = hid->report[0].data[i]&0x7F;
				if (!key) break;

				if (!stateList[key].state){
					stateList[key].state = 1;

					if (lock(&sbctx->s)){
						if (sbctx->dkCB)
							sbctx->dkCB(stateList[key].sbId, SBUI_DK_DOWN, sbctx->udata_ptr);
						unlock(&sbctx->s);
					}
				}
				stateList[key].stamp = stampId;
			}
			
			for (int i = 1; i < 128; i++){
				if (stateList[i].state == 1 && stateList[i].stamp != stampId){
					stateList[i].state = 0;
					
					if (lock(&sbctx->s)){
						if (sbctx->dkCB)
							sbctx->dkCB(stateList[i].sbId, SBUI_DK_UP, sbctx->udata_ptr);
						unlock(&sbctx->s);
					}
				}
			}
		}

	};
	
	sbHid_close(hid);


threadExit:
	sbctx->threadState = 0;
	//printf("sb_dynamicKeHandler EXIT\n");
	_endthreadex(1);
	return 1;
}


#else

int sbui153Init (void *rd){return 0;}
void sbui153Close (void *rd){return;}

#endif

