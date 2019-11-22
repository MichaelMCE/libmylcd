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

#define  SWITCHBLADESDK_FE_VERSION 0x0100

//
// The SwitchBlade SDK requires that start to gain exclusive control of the
// SwitchBlade hardware. Registering a preemption callback is a good idea in case
// the application must be preempted by the framework...
//

extern HRESULT RzSBStart(void);
extern HRESULT RzSBStop(void);
extern HRESULT RzSBQueryCapabilities(PSBSDKQUERYCAPABILITIES);
extern HRESULT RzSBRenderBuffer(int /*RZTARGET_DISPLAY*/ , RZSB_BUFFERPARAMS *);
//
// The SwitchBlade SDK allows us to choose the images on the dynamic keys as
// well as the touchpad underlay.
//
extern HRESULT RzSBSetImageDynamicKey(RZDKTYPE, RZDKSTATETYPE, LPWSTR);
extern HRESULT RzSBSetImageTouchpad(LPWSTR);

//
// The SwitchBlade SDK allows us to choose not to get callbacks for one, some or all
// of the gestures we support. Default is that everything it enabled, but we can
// elect not to get *any* touch data if we choose.
//
extern HRESULT RzSBGestureEnable(RZSDKGESTURETYPE, bool);
extern HRESULT RzSBGestureSetNotification(RZSDKGESTURETYPE, bool);
extern HRESULT RzSBGestureSetOSNotification(RZSDKGESTURETYPE, bool);

extern HRESULT RzSBAppEventSetCallback(AppEventCallbackType);
extern HRESULT RzSBDynamicKeySetCallback(DynamicKeyCallbackFunctionType);
extern HRESULT RzSBGestureSetCallback(TouchpadGestureCallbackFunctionType);

//
// The SwitchBlade SDK allows us to designate an HWND to render in the background.
// This permits native Windows application development to be "remoted" to the LCD.
//
extern HRESULT RzSBWinRenderSetDisabledImage(LPWSTR pszImageFilename);
extern HRESULT RzSBWinRenderAddKeyInputCtrls(RZSB_KEYEVTCTRLS *pKeyboardEvtCtrls, int nCtrlCount, bool bResetList);
extern HRESULT RzSBWinRenderStart(HWND hwnd, bool bTranslateGestures, bool bVisibleOnDesktop);
extern HRESULT RzSBWinRenderStop(bool bEraseOnStop);

//
// The SwitchBlade SDK HWND rendering capabilities keep statistics on the time
// required to execute each rendering pass. This includes the max, min, and last
// run time as well as averages that can be reset at the caller's discretion.
//
extern HRESULT RzSBWinRenderResetStats();
extern HRESULT RzSBWinRenderGetStats(DWORD *pdwCount, DWORD *pdwMaxTime, DWORD *pdwLastTime, DWORD *pdwAverageTime);


#ifdef __cplusplus
}
#endif

#endif // _SwitchBlade_H_