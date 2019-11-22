//
//  SwitchBladeSDK_types.h
//
//  Copyright© 2012, Razer USA Ltd. All rights reserved.
//

#ifndef _SwitchBladeSDK_TYPES_H_ 	// shield against multiple inclusion
#define _SwitchBladeSDK_TYPES_H_

#include "SwitchBladeSDK_defines.h"

/******************************************************************************
** SwitchBladeQueryCabilities
** This SDK call tells about the hardware and resources we supply to
** applications.
*/
#define MAX_SUPPORTED_SURFACES	2

#define PIXEL_FORMAT_INVALID	0
#define PIXEL_FORMAT_RGB_565	1

typedef struct _RZSBSDK_QUERYCAPABILITIES
{
	DWORD                   qc_version;
	DWORD					qc_BEVersion;
	SWITCHBLADEHARDWARETYPE qc_HardwareType;
	DWORD                   qc_numSurfaces;
	POINT                   qc_surfacegeometry[MAX_SUPPORTED_SURFACES];
	DWORD                   qc_pixelformat[MAX_SUPPORTED_SURFACES];
	BYTE                    qc_numDynamicKeys;
	POINT                   qc_DynamicKeyArrangement;
	POINT                   qc_keyDynamicKeySize;
} RZSBSDK_QUERYCAPABILITIES, *PRZSBSDK_QUERYCAPABILITIES;


/******************************************************************************
** Common Definitions 
*/

// this is the set of possible states for a dynamic key or a static key.
// Note that zero is not a valid key state value.
typedef enum _RZSBSDK_KEYSTATE
{
    RZSBSDK_KEYSTATE_NONE = 0, 
    RZSBSDK_KEYSTATE_UP, 
    RZSBSDK_KEYSTATE_DOWN, 
    RZSBSDK_KEYSTATE_HOLD, 
    RZSBSDK_KEYSTATE_INVALID, 
} RZSBSDK_KEYSTATETYPE, *PRZSBSDK_KEYSTATETYPE; 

#define ValidKeyState(a) (RZSBSDK_KEYSTATE_NONE < (a)) && ((a) < RZSBSDK_KEYSTATE_INVALID)

/******************************************************************************
** Static keys section
** definitions, enumerated types, helpful macros and callback info
*/

// this is the set of possible dynamic keys.
// Note that zero is not a valid key value.
typedef enum _RZSDKSTATICKEYTYPE
{
    RZSBSDK_STATICKEY_NONE = 0,
    RZSBSDK_STATICKEY_RAZER,
    RZSBSDK_STATICKEY_GAME,
    RZSBSDK_STATICKEY_MACRO,
    RZSBSDK_STATICKEY_INVALID
} RZSBSDK_STATICKEYTYPE, *PRZSBSDK_STATICKEYTYPE;

// typedef for callback used by Razer infrastructure to notify Applets when a static key 
// is pressed. The static keys are: Razer key, Game Mode and Macro. 
// This typedef is used as a parameter to SetDynamicKeyCallback(). 
typedef HRESULT (CALLBACK *StaticKeyCallbackFunctionType)(RZSBSDK_STATICKEYTYPE, RZSBSDK_KEYSTATETYPE); 


/******************************************************************************
** Dynamic keys section
** definitions, enumerated types, helpful macros and callback info
*/

// this is the set of possible dynamic keys.
// Note that zero is not a valid key value.
typedef enum _RZSBSDK_DKTYPE
{
    RZSBSDK_DK_NONE = 0,
    RZSBSDK_DK_1,
    RZSBSDK_DK_2,
    RZSBSDK_DK_3,
    RZSBSDK_DK_4,
    RZSBSDK_DK_5,
    RZSBSDK_DK_6,
    RZSBSDK_DK_7,
    RZSBSDK_DK_8,
    RZSBSDK_DK_9,
    RZSBSDK_DK_10,
    RZSBSDK_DK_INVALID,
    RZSBSDK_DK_COUNT = 10
} RZSBSDK_DKTYPE, *PRZSBSDK_DKTYPE;

#define ValidDynamicKey(a) (RZSBSDK_DK_NONE < (a)) && ((a) < RZSBSDK_DK_INVALID)

// typedef for callback used by Razer infrastructure to notify Applets when their
// dynamic key events of interest take place. 
// This typedef is used as a parameter to SetDynamicKeyCallback(). 
typedef HRESULT (CALLBACK *DynamicKeyCallbackFunctionType)(RZSBSDK_DKTYPE, RZSBSDK_KEYSTATETYPE); 


/******************************************************************************
** Touchpad Display section
** definitions, enumerated types, macros and callback info
*/

// BufferParameters
// Used for sending raw data of type PIXEL_BYTE to SwitchBlade subsystem. 
#define TARGET_MASK(x)				((1 << 16) | (x))

enum RZSBSDK_DISPLAY 
{
	RZSBSDK_DISPLAY_WIDGET	    = TARGET_MASK(RZSBSDK_DK_NONE), 
	RZSBSDK_DISPLAY_DK_1		= TARGET_MASK(RZSBSDK_DK_1),
	RZSBSDK_DISPLAY_DK_2		= TARGET_MASK(RZSBSDK_DK_2),
	RZSBSDK_DISPLAY_DK_3		= TARGET_MASK(RZSBSDK_DK_3),
	RZSBSDK_DISPLAY_DK_4		= TARGET_MASK(RZSBSDK_DK_4),
	RZSBSDK_DISPLAY_DK_5		= TARGET_MASK(RZSBSDK_DK_5),
	RZSBSDK_DISPLAY_DK_6		= TARGET_MASK(RZSBSDK_DK_6),
	RZSBSDK_DISPLAY_DK_7		= TARGET_MASK(RZSBSDK_DK_7),
	RZSBSDK_DISPLAY_DK_8		= TARGET_MASK(RZSBSDK_DK_8),
	RZSBSDK_DISPLAY_DK_9		= TARGET_MASK(RZSBSDK_DK_9),
	RZSBSDK_DISPLAY_DK_10	    = TARGET_MASK(RZSBSDK_DK_10),
};

enum PIXEL_TYPE		{ RGB565 = 0 };
typedef struct _RZSBSDK_BUFFERPARAMS
{	
	int /*PIXEL_TYPE*/	PixelType;
	DWORD		DataSize;	// Buffer size
	BYTE		*pData;
} RZSBSDK_BUFFERPARAMS, *PRZSBSDK_BUFFERPARAMS;

typedef enum _RZSBSDK_GESTURETYPE
{
    RZSBSDK_GESTURE_NONE      = 0x00000000, 
    RZSBSDK_GESTURE_PRESS     = 0x00000001, //dwParameters(touchpoints), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
    RZSBSDK_GESTURE_TAP       = 0x00000002, //dwParameters(reserved), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
    RZSBSDK_GESTURE_FLICK     = 0x00000004, //dwParameters(number of touch points), wXPos(reserved), wYPos(reserved), wZPos(direction)
    RZSBSDK_GESTURE_ZOOM      = 0x00000008, //dwParameters(1:zoomin, 2:zoomout), wXPos(), wYPos(), wZPos()
    RZSBSDK_GESTURE_ROTATE    = 0x00000010, //dwParameters(1:clockwise 2:counterclockwise), wXPos(), wYPos(), wZPos()
    RZSBSDK_GESTURE_MOVE      = 0x00000020, //dwParameters(reserved), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
    RZSBSDK_GESTURE_HOLD      = 0x00000040, //reserved
    RZSBSDK_GESTURE_RELEASE   = 0x00000080, //dwParameters(touchpoints), wXPos(coordinate), wYPos(coordinate), wZPos(reserved)
    RZSBSDK_GESTURE_SCROLL    = 0x00000100, //reserved
    RZSBSDK_GESTURE_ALL       = 0xFFFF
} RZSBSDK_GESTURETYPE, *PRZSBSDK_GESTURETYPE;

#define ValidGesture(a) ((a) & RZSBSDK_GESTURE_ALL)
#define SingleGesture(a) (0 == ((a - 1) & (a))) // test for only one bit set, exploiting 2s' complement math


/******************************************************************************
** Application Events section
** definitions, enumerated types, macros and callback info
*/

// typedef for app events to applets
typedef enum _RZSBSDK_EVENTTYPE
{
    RZSBSDK_EVENT_NONE = 0,
	RZSBSDK_EVENT_ACTIVATED,	    
    RZSBSDK_EVENT_DEACTIVATED,	
    RZSBSDK_EVENT_CLOSE,	        
    RZSBSDK_EVENT_EXIT,	        
	RZSBSDK_EVENT_INVALID,	    
} RZSBSDK_EVENTTYPETYPE, *PRZSBSDK_EVENTTYPETYPE;

// typedef for callback for Applets to recieve framework-triggered events 
// other than the Keys, Gestures and Keyboard events. 
typedef HRESULT (CALLBACK *AppEventCallbackType) (RZSBSDK_EVENTTYPETYPE, DWORD, DWORD);


/******************************************************************************
** Gesture Events section
** definitions, enumerated types, macros and callback info
*/

// Generic gestures direction types 
// Currently used to determine Flick direction 
typedef enum _RZSBSDK_DIRECTIONTYPE
{	
    RZSBSDK_DIRECTION_NONE	= 0,
    RZSBSDK_DIRECTION_LEFT,
    RZSBSDK_DIRECTION_RIGHT,
    RZSBSDK_DIRECTION_UP,
    RZSBSDK_DIRECTION_DOWN, 
    RZSBSDK_DIRECTION_INVALID
} RZSBSDK_DIRECTIONTYPE, *PRZSBSDK_DIRECTIONTYPE;


// Macro that checks if the value of an RZSDKGESTURETYPE variable is valid.
#define ValidGesture(a) ((a) & RZSBSDK_GESTURE_ALL)

#define SingleGesture(a) (0 == ((a - 1) & (a))) // test for only one bit set, exploiting 2s' complement math

// typedef for callback used by Razer infrastructure to notify Applets when their
// touchpad events of interest take place. 
// This typedef is used as a parameter to SetTouchpadGestureCallback(). 
typedef HRESULT (CALLBACK *TouchpadGestureCallbackFunctionType)(RZSBSDK_GESTURETYPE, DWORD, WORD, WORD, WORD);

// typedef for callback used by Razer infrastructure to notify Applets when keyboard
// events take place after setting the keyboard capture.
typedef HRESULT (CALLBACK *KeyboardCallbackFunctionType)(UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // _SwitchBladeSDK_TYPES_H_