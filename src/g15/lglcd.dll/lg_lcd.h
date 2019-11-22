

#include "lglcd.h"

#ifdef USELGLCDDLL

#define LGEXPORT __declspec(dllexport) __cdecl

LGEXPORT DWORD lg_LcdConnectW (IN OUT lgLcdConnectContextW *ctx);
LGEXPORT DWORD lg_LcdConnectA (IN OUT lgLcdConnectContextA *ctx);
LGEXPORT DWORD lg_LcdConnectExW (IN OUT lgLcdConnectContextExW *ctx);
LGEXPORT DWORD lg_LcdConnectExA (IN OUT lgLcdConnectContextExA *ctx);
LGEXPORT DWORD lg_LcdDisconnect (int connection);
LGEXPORT DWORD lg_LcdSetDeviceFamiliesToUse (IN int connection, DWORD dwDeviceFamiliesSupported, DWORD dwReserved);
LGEXPORT DWORD lg_LcdEnumerate (IN int connection, IN int index, OUT lgLcdDeviceDesc *description);
LGEXPORT DWORD lg_LcdEnumerateExW (IN int connection, IN int index, OUT lgLcdDeviceDescExW *description);
LGEXPORT DWORD lg_LcdEnumerateExA (IN int connection, IN int index, OUT lgLcdDeviceDescExA *description);
LGEXPORT DWORD lg_LcdClose (IN int device);
LGEXPORT DWORD lg_LcdReadSoftButtons (IN int device, OUT DWORD *buttons);
LGEXPORT DWORD lg_LcdUpdateBitmap (IN int device, IN const lgLcdBitmapHeader *bitmap, IN DWORD priority);
LGEXPORT DWORD lg_LcdSetAsLCDForegroundApp (IN int device, IN int foregroundYesNoFlag);
LGEXPORT DWORD lg_LcdDeInit ();
LGEXPORT DWORD lg_LcdInit ();
LGEXPORT DWORD lg_LcdOpen (IN OUT lgLcdOpenContext *ctx);
LGEXPORT DWORD lg_LcdOpenByType (IN OUT lgLcdOpenByTypeContext *ctx);

#else

#define lg_LcdConnectW lgLcdConnectW
#define lg_LcdConnectA lgLcdConnectA 
#define lg_LcdConnectExW lgLcdConnectExW 
#define lg_LcdConnectExA lgLcdConnectExA 
#define lg_LcdDisconnect lgLcdDisconnect 
#define lg_LcdSetDeviceFamiliesToUse lgLcdSetDeviceFamiliesToUse 
#define lg_LcdEnumerate lgLcdEnumerate 
#define lg_LcdEnumerateExW lgLcdEnumerateExW 
#define lg_LcdEnumerateExA lgLcdEnumerateExA 
#define lg_LcdClose lgLcdClose 
#define lg_LcdReadSoftButtons lgLcdReadSoftButtons 
#define lg_LcdUpdateBitmap lgLcdUpdateBitmap 
#define lg_LcdSetAsLCDForegroundApp lgLcdSetAsLCDForegroundApp 
#define lg_LcdDeInit lgLcdDeInit 
#define lg_LcdInit lgLcdInit 
#define lg_LcdOpen lgLcdOpen 
#define lg_LcdOpenByType lgLcdOpenByType 


#endif


#ifdef UNICODE
#define lg_LcdConnect lg_LcdConnectW
#define lg_LcdConnectEx lg_LcdConnectExW
#else
#define lg_LcdConnect lg_LcdConnectA
#define lg_LcdConnectEx lg_LcdConnectExA
#endif // !UNICODE

#ifdef UNICODE
#define lg_LcdEnumerateEx lg_LcdEnumerateExW
#else
#define lg_LcdEnumerateEx lg_LcdEnumerateExA
#endif // !UNICODE


