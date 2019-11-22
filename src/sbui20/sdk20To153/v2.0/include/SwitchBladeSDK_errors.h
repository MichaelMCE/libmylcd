//
//  SwitchBladeSDK_errors.h
//
//  Copyright© 2012, Razer USA Ltd. All rights reserved.
//
//  Closely modeled after WinError.h's HRESULT in format for error,
//	subsystem, customer/ms origin...
//
//  We use HRESULTS and encourage developers to use them
//
//  We leave room between error categories for growth
//
//	Developers are strongly encouraged to use these definitions for
//	compatibilty with previous and subsequent Switchblade SDK versions
//

#ifndef _SwitchBladeSDK_ERRORS_H_	// shield against multiple inclusion
#define _SwitchBladeSDK_ERRORS_H_

/*
** These errors are from the standard set. listed separately for clarity.
*/
#define RZSB_OK                               S_OK
#define RZSB_UNSUCCESSFUL                     E_FAIL
#define RZSB_INVALID_PARAMETER                E_INVALIDARG
#define RZSB_INVALID_POINTER                  E_POINTER     // points to data that is either not fully readable or writable
#define RZSB_ABORTED                          E_ABORT
#define RZSB_NO_INTERFACE                     E_NOINTERFACE
#define RZSB_NOT_IMPLEMENTED                  E_NOTIMPL
#define RZSB_FILE_NOT_FOUND                   ERROR_FILE_NOT_FOUND

/* useful? status macros */
#define RZSB_SUCCESS(a) (S_OK == (a))
#define RZSB_FAILED(a)  (S_OK != (a))

#define RZSB_GENERIC_BASE                     0x20000000
#define RZSB_FILE_ZERO_SIZE                   (RZSB_GENERIC_BASE + 0x1)  // zero-length files not allowed */
#define RZSB_FILE_INVALID_NAME                (RZSB_GENERIC_BASE + 0x2)  // */
#define RZSB_FILE_INVALID_TYPE                (RZSB_GENERIC_BASE + 0x3)  /* zero-sized images not allowed */
#define RSZB_FILE_READ_ERROR                  (RZSB_GENERIC_BASE + 0x4)  /* tried to read X bytes, got back a different number. Chaos! */
#define RZSB_FILE_INVALID_FORMAT              (RZSB_GENERIC_BASE + 0x5)  /* not a supported file format */
#define RZSB_FILE_INVALID_LENGTH              (RZSB_GENERIC_BASE + 0x6)  /* file length not consistent with expected value */
#define RZSB_FILE_NAMEPATH_TOO_LONG           (RZSB_GENERIC_BASE + 0x7)  /* path + name exceeds limit for the string, usually 260 chars */
#define RZSB_IMAGE_INVALID_SIZE               (RZSB_GENERIC_BASE + 0x8)  /* invalid image size -- totally wrong for dimensions */
#define RZSB_IMAGE_INVALID_DATA               (RZSB_GENERIC_BASE + 0x9)  /* image data did not verify as valid */
#define RZSB_WIN_VERSION_INVALID			  (RZSB_GENERIC_BASE + 0xa)  /* must be Win7 or greater workstation */

/* generic callback errors, but specific to the SDK */
#define RZSB_CALLBACK_BASE                    0x20010000
#define RZSB_CALLBACK_NOT_SET                 (RZSB_CALLBACK_BASE + 0x1)  /* tried to call or clear a callback that was not set */
#define RZSB_CALLBACK_ALREADY_SET             (RZSB_CALLBACK_BASE + 0x2)  /* tried to set a previously set callback without clearing it first */
#define RZSB_CALLBACK_REMOTE_FAIL             (RZSB_CALLBACK_BASE + 0x3)  /* set callback failed on the server side */
/* control */
#define RZSB_CONTROL_BASE_ERROR               0x20020000
#define RZSB_CONTROL_NOT_LOCKED               (RZSB_CONTROL_BASE_ERROR + 0x01) /* unlock when we didn't lock? -- careless */
#define RZSB_CONTROL_LOCKED                   (RZSB_CONTROL_BASE_ERROR + 0x02) /* someone else has the lock */
#define RZSB_CONTROL_ALREADY_LOCKED           (RZSB_CONTROL_BASE_ERROR + 0x03) /* we already locked it? -- careless */  
#define RZSB_CONTROL_PREEMPTED                (RZSB_CONTROL_BASE_ERROR + 0x04) /* preemption took place! */

/* dynamic keys */
#define RZSB_DK_BASE_ERROR                    0x20040000
#define RZSB_DK_INVALID_KEY                   (RZSB_DK_BASE_ERROR + 0x1) /* invalid dynamic key */
#define RZSB_DK_INVALID_KEY_STATE             (RZSB_DK_BASE_ERROR + 0x2) /* invalid dynamic key state */

/* touchpad (buttons and gestures) */
#define RZSB_TOUCHPAD_BASE_ERROR              0x20080000
#define RZSB_TOUCHPAD_INVALID_GESTURE         (RZSB_TOUCHPAD_BASE_ERROR + 0x1) /* invalid gesture */

/* interface-specific errors */
#define RZSB_INTERNAL_BASE_ERROR              0x20100000
#define RZSB_ALREADY_STARTED                  (RZSB_INTERNAL_BASE_ERROR + 0x1)  /* callback structures already initialized */
#define RZSB_NOT_STARTED                      (RZSB_INTERNAL_BASE_ERROR + 0x2)  /* internal structure in disorder */
#define RZSB_CONNECTION_ERROR                 (RZSB_INTERNAL_BASE_ERROR + 0x3)  /* connection to application services failed */
#define RZSB_INTERNAL_ERROR                   (RZSB_INTERNAL_BASE_ERROR + 0x4)  /* unknown error -- catch-all for now */

/* windows rendering errors */
#define RZSB_WINRENDER_BASE_ERROR			  0x20200000
#define RZSB_WINRENDER_OUT_OF_RESOURCES		  (RZSB_WINRENDER_BASE_ERROR + 0x01) /* could not allocate critical resources */
#define RZSB_WINRENDER_THREAD_FAILED		  (RZSB_WINRENDER_BASE_ERROR + 0x02) /* could not start rendering thread */
#define RZSB_WINRENDER_WRONG_MODEL			  (RZSB_WINRENDER_BASE_ERROR + 0x03) /* not using multithreaded apartments */

/* room to grow */

#endif // _SwitchBladeSDK_ERRORS_H_
