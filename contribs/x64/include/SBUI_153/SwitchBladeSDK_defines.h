//
//  SwitchBladeSDK_defines.h
//
//  Copyright© 2012, Razer USA Ltd. All rights reserved.
//
//	Switchblade definitions, mostly having to s with versioning and
//	screen dimensions for Dynamic Keys and the Touchpad
//

#ifndef _SwitchBladeSDK_DEFINES_H_ 	// shield against multiple inclusion
#define _SwitchBladeSDK_DEFINES_H_

typedef enum _SWITCHBLADE_HARDWARE_TYPE_
{
	HARDWARETYPE_INVALID = 0,
	HARDWARETYPE_SWITCHBLADE,	// switchblade module
	HARDWARETYPE_UNDEFINED
} SWITCHBLADEHARDWARETYPE, *PSWITCHBLADEHARDWARETYPE;


/*
** definitions for the Dynamic Key display region of the Switchblade
*/
#define SWITCHBLADE_DYNAMIC_KEYS_PER_ROW	5
#define SWITCHBLADE_DYNAMIC_KEYS_ROWS		2
#define SWITCHBLADE_DYNAMIC_KEY_X_SIZE		115 
#define SWITCHBLADE_DYNAMIC_KEY_Y_SIZE		115
#define SWITCHBLADE_DK_SIZE_IMAGEDATA		(SWITCHBLADE_DYNAMIC_KEY_X_SIZE * SWITCHBLADE_DYNAMIC_KEY_Y_SIZE * sizeof(WORD))

/*
** definitions for the Touchpad display region of the Switchblade
*/
#define SWITCHBLADE_TOUCHPAD_X_SIZE			800
#define SWITCHBLADE_TOUCHPAD_Y_SIZE			480
#define SWITCHBLADE_TOUCHPAD_SIZE_IMAGEDATA (SWITCHBLADE_TOUCHPAD_X_SIZE * SWITCHBLADE_TOUCHPAD_Y_SIZE * sizeof(WORD))

#define SWITCHBLADE_DISPLAY_COLOR_DEPTH		16 // 16 bpp

#define MAX_STRING_LENGTH					260 // no paths allowed long than this
#ifdef _DEBUG
#define DebugCheckFault() \
	if (IsDebuggerPresent()) \
	{ \
		DebugBreak(); \
	}
#else
#define DebugCheckFault()
#endif

#endif // _SwitchBladeSDK_DEFINES_H_