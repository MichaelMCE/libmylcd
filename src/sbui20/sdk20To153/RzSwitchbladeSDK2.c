





#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <fcntl.h>




#define DEBUGLOGGING		(0)
#define SBSDKDLL			"SwitchBladeSDK32.dll"

static int initOnce = 0;



#undef DLL_DECLARE
#define DLL_DECLARE(ret, api, name, args) \
  typedef ret (api * __dll_##name##_t)args; static __dll_##name##_t _##name

#undef DLL_LOAD
#define DLL_LOAD(dll, name, ret_on_failure)                 			\
  do {                                             						\
  HANDLE h = (HANDLE)GetModuleHandle(dll);             					\
  if (!h)                                              					\
    h = (HANDLE)LoadLibrary(dll);                      					\
  if (!h){                                            					\
    if (ret_on_failure){                               					\
      dll_load_error(#name, dll, -1);									\
      return -1;                             							\
    }																	\
    else break; }                                             			\
  if ((_##name = (__dll_##name##_t)GetProcAddress(h, #name))) 			\
    break;                                                    			\
																		\
  if (ret_on_failure){                                         			\
    dll_load_error(#name, dll, -2);										\
    return -2;                               						  	\
   }																	\
  }while(0)


DLL_DECLARE(HRESULT, , RzSBStart, (void));
DLL_DECLARE(HRESULT, , RzSBStop, (void));

DLL_DECLARE(HRESULT, , RzSBQueryCapabilities, (void*));
DLL_DECLARE(HRESULT, , RzSBRenderBuffer, (int, void *));

DLL_DECLARE(HRESULT, , RzSBSetImageDynamicKey, (int, int, void*));
DLL_DECLARE(HRESULT, , RzSBSetImageTouchpad, (void*));

DLL_DECLARE(HRESULT, , RzSBGestureEnable, (int, int));
DLL_DECLARE(HRESULT, , RzSBGestureSetNotification, (int, int));
DLL_DECLARE(HRESULT, , RzSBGestureSetOSNotification, (int, int));

DLL_DECLARE(HRESULT, , RzSBDynamicKeySetCallback, (void*));
DLL_DECLARE(HRESULT, , RzSBGestureSetCallback, (void*));
DLL_DECLARE(HRESULT, , RzSBAppEventSetCallback, (void*));





typedef HRESULT (CALLBACK *touchpadGestureCBType)(int, int, int, int, int);
typedef HRESULT (CALLBACK *dynamicKeySetCBType)(int, int);
typedef HRESULT (CALLBACK *AppEventCBType) (int event, int, int);

static touchpadGestureCBType touchpadGestureCB;
static dynamicKeySetCBType dynamicKeySetCB;
static AppEventCBType appEventCB;




#define RZGESTURE_INVALID    0x00000000
#define RZGESTURE_NONE       0x00000001
#define RZGESTURE_PRESS      0x00000002
#define RZGESTURE_TAP        0x00000004
#define RZGESTURE_FLICK      0x00000008
#define RZGESTURE_ZOOM       0x00000010
#define RZGESTURE_ROTATE     0x00000020
#define RZGESTURE_ALL        0x0000003e
#define RZGESTURE_UNDEFINED  0xffffffc0

#define RZSBSDK_GESTURE_NONE       0x00000000 
#define RZSBSDK_GESTURE_PRESS      0x00000001 //dwParameters(touchpoints), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
#define RZSBSDK_GESTURE_TAP        0x00000002 //dwParameters(reserved), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
#define RZSBSDK_GESTURE_FLICK      0x00000004 //dwParameters(number of touch points), wXPos(reserved), wYPos(reserved), wZPos(direction)
#define RZSBSDK_GESTURE_ZOOM       0x00000008 //dwParameters(1:zoomin, 2:zoomout), wXPos(), wYPos(), wZPos()
#define RZSBSDK_GESTURE_ROTATE     0x00000010 //dwParameters(1:clockwise 2:counterclockwise), wXPos(), wYPos(), wZPos()
#define RZSBSDK_GESTURE_MOVE       0x00000020 //dwParameters(reserved), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
#define RZSBSDK_GESTURE_HOLD       0x00000040 //reserved
#define RZSBSDK_GESTURE_RELEASE    0x00000080 //dwParameters(touchpoints), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
#define RZSBSDK_GESTURE_SCROLL     0x00000100 //reserved
#define RZSBSDK_GESTURE_ALL        0xFFFF


enum RZSBSDK_EVENTTYPE
{
    RZSBSDK_EVENT_NONE = 0,
	RZSBSDK_EVENT_ACTIVATED,	    
    RZSBSDK_EVENT_DEACTIVATED,	
    RZSBSDK_EVENT_CLOSE,	        
    RZSBSDK_EVENT_EXIT,	        
	RZSBSDK_EVENT_INVALID,	    
};




enum _RZSBSDK_KEYSTATE20_
{
    RZSBSDK_KEYSTATE_NONE = 0, 
    RZSBSDK_KEYSTATE_UP, 
    RZSBSDK_KEYSTATE_DOWN, 
    RZSBSDK_KEYSTATE_HOLD, 
    RZSBSDK_KEYSTATE_INVALID, 
};

enum _RZSDKSTATETYPE153_
{
    RZSDKSTATE_INVALID = 0,
    RZSDKSTATE_DISABLED,
    RZSDKSTATE_UP,
    RZSDKSTATE_DOWN,
    RZSDKSTATE_UNDEFINED
};
	


#if DEBUGLOGGING
unsigned int __stdcall initConsole (void *ptr)
{
	if (AllocConsole()){
		int hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);
		if (hCrt){
			FILE *hf = fdopen(hCrt, "w");
			if (hf){
				*stdout = *hf;
				setvbuf(stdout, NULL, _IONBF, 0);
			}else{
				FreeConsole();
			}
		}else{
			FreeConsole();
		}
	}
	return 1;
}
#endif



CALLBACK HRESULT DynamicKeySetCallback (int type, int state)
{
	if (state == RZSDKSTATE_UP)
		state = RZSBSDK_KEYSTATE_UP;
	else if (state == RZSDKSTATE_DOWN)
		state = RZSBSDK_KEYSTATE_DOWN;
	
	return dynamicKeySetCB(type, state);
}
	
CALLBACK HRESULT TouchpadGestureCallback (int type, int dwa, int wa, int wb, int wc)
{
	
	//printf("TouchpadGestureCallback %i %i %i %i %i\n", type, dwa, wa, wb, wc);

	switch (type){
	case RZGESTURE_INVALID:
		type = RZSBSDK_GESTURE_NONE;
		break;
	case RZGESTURE_NONE:
		type = RZSBSDK_GESTURE_NONE;
		break;
    case RZGESTURE_PRESS:
    	type = RZSBSDK_GESTURE_PRESS;
    	break;
    case RZGESTURE_TAP:
    	type = RZSBSDK_GESTURE_TAP;
    	break;
    case RZGESTURE_FLICK:
    	type = RZSBSDK_GESTURE_FLICK;
    	break;
    case RZGESTURE_ZOOM:
    	type = RZSBSDK_GESTURE_ZOOM;
    	break;
    case RZGESTURE_ROTATE:
    	type = RZSBSDK_GESTURE_ROTATE;
    	break;
    case RZGESTURE_ALL:
    	type = RZSBSDK_GESTURE_ALL;
    	break;
	case RZGESTURE_UNDEFINED:
		type = RZSBSDK_GESTURE_NONE;
		break;
	};
	
	return touchpadGestureCB(type, dwa, wa, wb, wc);
}


static inline void dll_load_error (const char *fn, const char *dll, const int err)
{
#if 0
	char buffer[MAX_PATH+1];
	
	if (err == -1)
		snprintf(buffer, MAX_PATH, "LoadLibrary(): File not found\n\n'%s'", dll);
	else if (err == -2)
		snprintf(buffer, MAX_PATH, "GetProcAddress(): Symbol '%s' not found in '%s'", fn, dll);
	else
		return;

	MessageBoxA(NULL,
	  buffer,
	  dll,
	  MB_SYSTEMMODAL|MB_ICONSTOP|MB_OK
	);
#endif
}

static inline int sbuiLoadDll (const char *lib)
{
	DLL_LOAD(lib, RzSBStart, 1);
	DLL_LOAD(lib, RzSBStop, 1);
	DLL_LOAD(lib, RzSBQueryCapabilities, 1);
	DLL_LOAD(lib, RzSBRenderBuffer, 1);
	
	DLL_LOAD(lib, RzSBSetImageDynamicKey, 1);
	DLL_LOAD(lib, RzSBSetImageTouchpad, 1);
	
	DLL_LOAD(lib, RzSBGestureEnable, 1);
	DLL_LOAD(lib, RzSBGestureSetOSNotification, 1);
	DLL_LOAD(lib, RzSBGestureSetNotification, 1);
	DLL_LOAD(lib, RzSBDynamicKeySetCallback, 1);
	DLL_LOAD(lib, RzSBGestureSetCallback, 1);
	DLL_LOAD(lib, RzSBAppEventSetCallback, 1);

	
	return 1;
}

static inline void sdk2init ()
{
	if (!initOnce){
		initOnce = 1;
#if DEBUGLOGGING
		initConsole(NULL);
		printf("sdk2init\n");
#endif
		
		sbuiLoadDll(SBSDKDLL);
	}
}


__declspec (dllexport) HRESULT RzSBQueryCapabilities (void *ptr)
{
#if DEBUGLOGGING
	printf("RzSBQueryCapabilities\n");
#endif

	sdk2init();
	
	return _RzSBQueryCapabilities(ptr);
}


__declspec (dllexport) HRESULT RzSBStart (void)
{
#if DEBUGLOGGING
	printf("RzSBStart\n");
#endif

	sdk2init();
	
	return _RzSBStart();
}

__declspec (dllexport) HRESULT RzSBStop (void)
{
#if DEBUGLOGGING
	printf("RzSBStop\n");
#endif

	sdk2init();
	
	return _RzSBStop();
}

__declspec (dllexport) HRESULT RzSBRenderBuffer (int i32, void *ptr)
{
#if DEBUGLOGGING
	printf("RzSBRenderBuffer %i %p\n", i32, ptr);
#endif

	sdk2init();
	
	return _RzSBRenderBuffer(i32, ptr);
}

static inline void resetCurrentDirectory ()
{
	wchar_t drive[MAX_PATH+1];
	wchar_t dir[MAX_PATH+1];
	wchar_t szPath[MAX_PATH+1];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	_wsplitpath(szPath, drive, dir, NULL, NULL);
	_swprintf(szPath, L"%s%s", drive, dir);
	SetCurrentDirectoryW(szPath);
}

__declspec (dllexport) HRESULT RzSBSetImageDynamicKey (int i32a, int keyState, void *ptr)
{
#if DEBUGLOGGING
	wprintf(L"RzSBSetImageDynamicKey %i %i %p '%s'\n", i32a, keyState, ptr, ptr);
#endif

	sdk2init();

	if (keyState == RZSBSDK_KEYSTATE_UP)
		keyState = RZSDKSTATE_UP;
	else if (keyState == RZSBSDK_KEYSTATE_DOWN)
		keyState = RZSDKSTATE_DOWN;

#if 0
	resetCurrentDirectory();
	
	wchar_t buffer[MAX_PATH+1];
	wchar_t drive[MAX_PATH+1];
	wchar_t dir[MAX_PATH+1];
	wchar_t szPath[MAX_PATH+1];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	_wsplitpath(szPath, drive, dir, NULL, NULL);
	_swprintf(szPath, L"%s%s", drive, dir);

	snwprintf(buffer, MAX_PATH, L"%s%s", szPath, ptr);
	//wprintf(L"#%s#\n", buffer);
	return _RzSBSetImageDynamicKey(i32a, keyState, buffer);
#else
	return _RzSBSetImageDynamicKey(i32a, keyState, ptr);
#endif
	
}

__declspec (dllexport) HRESULT RzSBSetImageTouchpad (void *ptr)
{
#if DEBUGLOGGING
	printf("RzSBSetImageTouchpad %p\n", ptr);
#endif

	sdk2init();
	
	return _RzSBSetImageTouchpad(ptr);
}

__declspec (dllexport) HRESULT RzSBGestureEnable (int i32a, int i32b)
{
#if DEBUGLOGGING
	printf("RzSBGestureEnable %i %i\n", i32a, i32b);
#endif

	sdk2init();
	
	return _RzSBGestureEnable(i32a, i32b);
}

__declspec (dllexport) HRESULT RzSBGestureSetNotification (int i32a, int i32b)
{
#if DEBUGLOGGING
	printf("RzSBGestureSetNotification %i %i\n", i32a, i32b);
#endif

	sdk2init();
	
	return _RzSBGestureSetNotification(i32a, i32b);
}

__declspec (dllexport) HRESULT RzSBGestureSetOSNotification (int i32a, int i32b)
{
#if DEBUGLOGGING
	printf("RzSBGestureSetOSNotification %i %i\n", i32a, i32b);
#endif

	sdk2init();
	
	return _RzSBGestureSetOSNotification(i32a, i32b);
}

__declspec (dllexport) HRESULT RzSBDynamicKeySetCallback (void *ptr)
{
#if DEBUGLOGGING
	printf("RzSBDynamicKeySetCallback %p\n", ptr);
#endif

	sdk2init();
	
	dynamicKeySetCB = ptr;
	HRESULT ret = _RzSBDynamicKeySetCallback(DynamicKeySetCallback);
	//if (appEventCB) appEventCB(RZSBSDK_EVENT_ACTIVATED, 0, 0);	
	
	return ret;
}

__declspec (dllexport) HRESULT RzSBGestureSetCallback (void *ptr)
{
#if DEBUGLOGGING
	printf("RzSBGestureSetCallback %p\n", ptr);
#endif

	sdk2init();
	
	touchpadGestureCB = ptr;
	return _RzSBGestureSetCallback(TouchpadGestureCallback);
}

__declspec (dllexport) HRESULT RzSBAppEventSetCallback (void *ptr)
{
#if DEBUGLOGGING
	printf("RzSBAppEventSetCallback %p\n", ptr);
#endif

	sdk2init();
	
	appEventCB = ptr;
	HRESULT ret = _RzSBAppEventSetCallback(ptr);

	appEventCB(RZSBSDK_EVENT_ACTIVATED, 0, 0);	
	return ret;
}

__declspec (dllexport) HRESULT RzSBEnableOSGesture (int i32a, int i32b)
{
#if DEBUGLOGGING
	printf("RzSBEnableOSGesture %i %i\n", i32a, i32b);
#endif

	sdk2init();
	
	return _RzSBGestureSetOSNotification(i32a, i32b);
}

__declspec (dllexport) HRESULT RzSBEnableGesture (int type, int i32b)
{
#if DEBUGLOGGING
	printf("RzSBEnableGesture %i %i\n", type, i32b);
#endif
	sdk2init();

	if (type == RZSBSDK_GESTURE_NONE)
		type = RZGESTURE_NONE;
    else if (type == RZSBSDK_GESTURE_PRESS)
    	type = RZGESTURE_PRESS;
    else if (type == RZSBSDK_GESTURE_TAP)
    	type = RZGESTURE_TAP;
    else if (type == RZSBSDK_GESTURE_FLICK)
    	type = RZGESTURE_FLICK;
    else if (type == RZSBSDK_GESTURE_ZOOM)
    	type = RZGESTURE_ZOOM;
    else if (type == RZSBSDK_GESTURE_ROTATE)
    	type = RZGESTURE_ROTATE;
    else if (type == RZSBSDK_GESTURE_ALL)
    	type = RZGESTURE_ALL;
    else if (type == RZSBSDK_GESTURE_MOVE || type == RZSBSDK_GESTURE_RELEASE || type == RZSBSDK_GESTURE_HOLD || type == RZSBSDK_GESTURE_SCROLL)
		return S_OK;
	else
		type = RZGESTURE_INVALID;
	
	return _RzSBGestureEnable(type, i32b);
}

__declspec (dllexport) HRESULT RzSBKeyboardCaptureSetCallback (void *ptr)
{
#if DEBUGLOGGING
	printf("RzSBKeyboardCaptureSetCallback %p\n", ptr);
#endif

	sdk2init();
	
	return S_OK;//_RzSBKeyboardCaptureSetCallback(ptr);
}

__declspec (dllexport) HRESULT RzSBCaptureKeyboard (int i32a)
{
#if DEBUGLOGGING
	printf("RzSBCaptureKeyboard %i\n", i32a);
#endif

	sdk2init();
	
	return S_OK;//_RzSBCaptureKeyboard(i32a);
}
