//
//  SwitchBlade.h
//
//  Copyright© 2012, Razer USA Ltd. All rights reserved.
//
//	This is the main header file for the SwitchBlade SDK. No other
//	include files are required for Switchblade SDK support.
//

#ifndef _SwitchBlade_H_ 	// shield against multiple inclusion
#define _SwitchBlade_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "SwitchBladeSDK.h"

#define  SWITCHBLADESDK_FE_VERSION 0x0200

//
// The SwitchBlade SDK requires that start to gain exclusive control of the
// SwitchBlade hardware. Registering a preemption callback is a good idea in case
// the application must be preempted by the framework...
//

extern HRESULT RzSBStart(void);
extern HRESULT RzSBStop(void);
extern HRESULT RzSBQueryCapabilities(PRZSBSDK_QUERYCAPABILITIES);
extern HRESULT RzSBRenderBuffer(RZSBSDK_DISPLAY , RZSBSDK_BUFFERPARAMS *);
//
// The SwitchBlade SDK allows us to choose the images on the dynamic keys as
// well as the touchpad underlay.
//
extern HRESULT RzSBSetImageDynamicKey(RZSBSDK_DKTYPE, RZSBSDK_KEYSTATETYPE, LPWSTR);
extern HRESULT RzSBSetImageTouchpad(LPWSTR);

//
// The SwitchBlade SDK allows us to choose not to get callbacks for one, some or all
// of the gestures we support. Default is that everything it enabled, but we can
// elect not to get *any* touch data if we choose.
//
extern HRESULT RzSBEnableGesture(RZSBSDK_GESTURETYPE, bool);
extern HRESULT RzSBEnableOSGesture(RZSBSDK_GESTURETYPE, bool);

extern HRESULT RzSBAppEventSetCallback(AppEventCallbackType);
extern HRESULT RzSBDynamicKeySetCallback(DynamicKeyCallbackFunctionType);
extern HRESULT RzSBGestureSetCallback(TouchpadGestureCallbackFunctionType);

extern HRESULT RzSBKeyboardCaptureSetCallback(KeyboardCallbackFunctionType);
extern HRESULT RzSBCaptureKeyboard(bool);

#ifdef __cplusplus
}
#endif

#endif // _SwitchBlade_H_