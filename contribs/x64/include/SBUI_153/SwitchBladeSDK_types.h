//
//  SwitchBladeSDK_types.h
//
//  Copyright© 2012, Razer USA Ltd. All rights reserved.
//

#ifndef _SwitchBladeSDK_TYPES_H_ 	// shield against multiple inclusion
#define _SwitchBladeSDK_TYPES_H_

/******************************************************************************
** control(lock) section
**
** helpful macros and callback info
*/

/*
** SwitchBladeQueryCabilities
**
** This SDK call tells about the hardware and resources we supply to
** applications.
*/
#define MAX_SUPPORTED_SURFACES	2

#define PIXEL_FORMAT_INVALID	0
#define PIXEL_FORMAT_RGB_565	1

typedef struct _SBSDK_QUERY_CAPABILITIES_
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
} SBSDKQUERYCAPABILITIES, *PSBSDKQUERYCAPABILITIES;

/******************************************************************************
** dynamic keys section
**
** definitions, enumerated types, helpful macros and callback info
*/

/*
** this is the set of possible states for a dynamic key.
** Note that zero is not a valid key state value.
*/
typedef enum _RZSDKSTATETYPE_
{
    RZSDKSTATE_INVALID = 0,
    RZSDKSTATE_DISABLED,
    RZSDKSTATE_UP,
    RZSDKSTATE_DOWN,
    RZSDKSTATE_UNDEFINED
} RZDKSTATETYPE, *PRZDKSTATETYPE;

/*
** macro to validate dynamic key state
*/
#define ValidDynamicKeyState(a) (RZSDKSTATE_INVALID < (a)) && ((a) < RZSDKSTATE_UNDEFINED)

/*
** this is the set of possible dynamic keys.
** Note that zero is not a valid key value.
*/
typedef enum _RZDYNAMICKEYTYPE
{
    RZDYNAMICKEY_INVALID = 0,
    RZDYNAMICKEY_DK1,
    RZDYNAMICKEY_DK2,
    RZDYNAMICKEY_DK3,
    RZDYNAMICKEY_DK4,
    RZDYNAMICKEY_DK5,
    RZDYNAMICKEY_DK6,
    RZDYNAMICKEY_DK7,
    RZDYNAMICKEY_DK8,
    RZDYNAMICKEY_DK9,
    RZDYNAMICKEY_DK10,
    RZDYNAMICKEY_UNDEFINED
} RZDKTYPE, *PRZDKTYPE;


/*
 * BufferParameters
 * ----------------
 * Used for sending raw data of type PIXEL_BYTE to 
 * SwitchBlade subsystem. 
 */
#define TARGET_MASK(x)				((1 << 16) | (x))

enum RZTARGET_DISPLAY 
{
	TARGET_DISPLAY_WIDGET	= TARGET_MASK(RZDYNAMICKEY_INVALID), 
	TARGET_DISPLAY_DK_1		= TARGET_MASK(RZDYNAMICKEY_DK1),
	TARGET_DISPLAY_DK_2		= TARGET_MASK(RZDYNAMICKEY_DK2),
	TARGET_DISPLAY_DK_3		= TARGET_MASK(RZDYNAMICKEY_DK3),
	TARGET_DISPLAY_DK_4		= TARGET_MASK(RZDYNAMICKEY_DK4),
	TARGET_DISPLAY_DK_5		= TARGET_MASK(RZDYNAMICKEY_DK5),
	TARGET_DISPLAY_DK_6		= TARGET_MASK(RZDYNAMICKEY_DK6),
	TARGET_DISPLAY_DK_7		= TARGET_MASK(RZDYNAMICKEY_DK7),
	TARGET_DISPLAY_DK_8		= TARGET_MASK(RZDYNAMICKEY_DK8),
	TARGET_DISPLAY_DK_9		= TARGET_MASK(RZDYNAMICKEY_DK9),
	TARGET_DISPLAY_DK_10	= TARGET_MASK(RZDYNAMICKEY_DK10),
};

enum PIXEL_TYPE		{ RGB565 = 0 };
typedef struct __rzSBBufferParams
{	
	int /*PIXEL_TYPE*/	PixelType;
	DWORD		DataSize;	// Buffer size
	BYTE		*pData;
} RZSB_BUFFERPARAMS, *PRZSB_BUFFERPARAMS;

/*
** macro to validate dynamic key values
*/
#define ValidDynamicKey(a) (RZDYNAMICKEY_INVALID < (a)) && ((a) < RZDYNAMICKEY_UNDEFINED)

/*
** typedef for callback used by Razer infrastructure to notify Applets when their
** dynamic key events of interest take place. This typedef is used as a parameter to
** SetDynamicKeyCallback() along with the dynamic key and it's state you want to bind
** to this callback routine. 
*/
typedef HRESULT (CALLBACK *DynamicKeyCallbackFunctionType)(RZDKTYPE, RZDKSTATETYPE);


/******************************************************************************
** Touchpad section
** definitions, enumerated types, macros and callback info
*/

/*
** this is the set of possible gestures.
** Note that zero is not a gestural value.
**
** ALL is defined, but not supported, and we
** use in the indices the value UNDEFINED for
** anything we don't understand.
*/
#ifndef GT_DEF
/*
** typedef for app events to applets
*/
typedef enum _RZSDKAPPEVENTTYPE_
{
    RZSDKAPPEVENTTYPE_INVALID	= 0,
	RZSDKAPPEVENTTYPE_APPMODE	= ( RZSDKAPPEVENTTYPE_INVALID + 1 ) ,
	RZSDKAPPEVENTTYPE_UNDEFINED	= ( RZSDKAPPEVENTTYPE_INVALID + 2 )
} RZSDKAPPEVENTTYPE, *PRZSDKAPPEVENTTYPE;

typedef enum _RZSDKAPPEVENTMODE_
{
	RZSDKAPPEVENTMODE_APPLET	= 0x02,
	RZSDKAPPEVENTMODE_NORMAL	= 0x04
} RASDKAPPEVENTMODE, *PRASDKAPPEVENTMODE;

/*
** typedef for callback in cases where we want to receive ownership change notification.
** This usually gets called shortly before an Applet is going to lose control of
** The Switchblade hardware. When this happens, Applet developers should clean up and
** save state immediately.
*/

typedef HRESULT (CALLBACK *AppEventCallbackType) (RZSDKAPPEVENTTYPE, DWORD, DWORD);

typedef enum _RZSDKGESTURETYPE_
{
    RZGESTURE_INVALID   = 0x00000000,
	RZGESTURE_NONE      = 0x00000001,
    RZGESTURE_PRESS     = 0x00000002,
    RZGESTURE_TAP       = 0x00000004,
    RZGESTURE_FLICK     = 0x00000008,
    RZGESTURE_ZOOM      = 0x00000010,
    RZGESTURE_ROTATE    = 0x00000020,
    RZGESTURE_ALL       = 0x0000003e,
	RZGESTURE_UNDEFINED = 0xffffffc0
} RZSDKGESTURETYPE, *PRZSDKGESTURETYPE;
#else
#define RZGESTURE_INVALID	0x00000000
#define	RZGESTURE_NONE      0x00000001
#define RZGESTURE_PRESS     0x00000002
#define RZGESTURE_TAP       0x00000004
#define RZGESTURE_FLICK     0x00000008
#define RZGESTURE_ZOOM      0x00000010
#define RZGESTURE_ROTATE    0x00000020
#define RZGESTURE_ALL       0x0000003e
#define	RZGESTURE_UNDEFINED 0xffffffc0
#endif

/*
** this definition and macro can be used to validate that the value of an RZSDKGESTURETYPE
** variable is valid.
*/

#define ValidGesture(a) ((a) & RZGESTURE_ALL)


#define SingleGesture(a) (0 == ((a - 1) & (a))) // test for only one bit set, exploiting 2s' complement math

/*
** typedef for callback used by Razer infrastructure to notify Applets when their
** touchpad events of interest take place. This typedef is used as a parameter to
** SetTouchpadGestureCallback() along with the gesture set and it's state you want to bind
** to this callback routine. 
*/

typedef HRESULT (CALLBACK *TouchpadGestureCallbackFunctionType)(RZSDKGESTURETYPE, DWORD, WORD, WORD, WORD);

/*
** typedef for callback used by Razer infrastructure to notify Applets when keyboard
** events take place after setting the keyboard capture. This capture is in the backend
** a disable keyboard call, which keeps Razer keyboard events from being sent to the OS
** allowing the calling application to trap these events and process them exclusively.
*/

typedef HRESULT (CALLBACK *KeyboardCallbackFunctionType)(UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
** typedef for list of controls for which the SDK captures keyboard input.
** This includes the windows handle of the control and whether or not the SDK should
** release keyboard capturing when <Enter> is pressed.
*/
typedef struct __rzSBKeyEvtCtrls
{
	HWND hwndTarget;
	bool bReleaseOnEnter;
} RZSB_KEYEVTCTRLS;

#endif // _SwitchBladeSDK_TYPES_H_