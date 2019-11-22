
#include <windows.h>
#include "lglcd.h"

#define LGEXPORT __declspec(dllexport) __cdecl



LGEXPORT DWORD lg_LcdConnectW (IN OUT lgLcdConnectContextW *ctx)
{
	return lgLcdConnectW(ctx);
}

LGEXPORT DWORD lg_LcdConnectA (IN OUT lgLcdConnectContextA *ctx)
{
	return lgLcdConnectA(ctx);
}

LGEXPORT DWORD lg_LcdConnectExW (IN OUT lgLcdConnectContextExW *ctx)
{
	return lgLcdConnectExW(ctx);
}

LGEXPORT DWORD lg_LcdConnectExA (IN OUT lgLcdConnectContextExA *ctx)
{
	return lgLcdConnectExA(ctx);
}

LGEXPORT DWORD lg_LcdDisconnect (int connection)
{
	return lgLcdDisconnect(connection);
}

LGEXPORT DWORD lg_LcdSetDeviceFamiliesToUse (IN int connection, DWORD dwDeviceFamiliesSupported, DWORD dwReserved)
{
	return lgLcdSetDeviceFamiliesToUse(connection, dwDeviceFamiliesSupported, dwReserved);
}

LGEXPORT DWORD lg_LcdEnumerate (IN int connection, IN int index, OUT lgLcdDeviceDesc *description)
{
	return lgLcdEnumerate(connection, index, description);
}

LGEXPORT DWORD lg_LcdEnumerateExW (IN int connection, IN int index, OUT lgLcdDeviceDescExW *description)
{
	return lgLcdEnumerateExW(connection, index, description);
}

LGEXPORT DWORD lg_LcdEnumerateExA (IN int connection, IN int index, OUT lgLcdDeviceDescExA *description)
{
	return lgLcdEnumerateExA(connection, index, description);
}

LGEXPORT DWORD lg_LcdClose (IN int device)
{
	return lgLcdClose(device);
}

LGEXPORT DWORD lg_LcdReadSoftButtons (IN int device, OUT DWORD *buttons)
{
	return lgLcdReadSoftButtons(device, buttons);
}

LGEXPORT DWORD lg_LcdUpdateBitmap (IN int device, IN const lgLcdBitmapHeader *bitmap, IN DWORD priority)
{
	return lgLcdUpdateBitmap(device, bitmap, priority);
}

LGEXPORT DWORD lg_LcdSetAsLCDForegroundApp (IN int device, IN int foregroundYesNoFlag)
{
	return lgLcdSetAsLCDForegroundApp(device, foregroundYesNoFlag);
}

LGEXPORT DWORD lg_LcdDeInit ()
{
	return lgLcdDeInit();
}

LGEXPORT DWORD lg_LcdInit ()
{
	return lgLcdInit();
}

LGEXPORT DWORD lg_LcdOpen (IN OUT lgLcdOpenContext *ctx)
{
	return lgLcdOpen(ctx);
}

LGEXPORT DWORD lg_LcdOpenByType (IN OUT lgLcdOpenByTypeContext *ctx)
{
	return lgLcdOpenByType(ctx);
}


BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return 1;
}




