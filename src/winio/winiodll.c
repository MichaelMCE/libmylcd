// ---------------------------------------------------- //
//                      WinIo v2.0                      //
//  Direct Hardware Access Under Windows 9x/NT/2000/XP  //
//           Copyright 1998-2002 Yariv Kaplan           //
//               http://www.internals.com               //
// ---------------------------------------------------- //

#include "mylcd.h"

#if ((__BUILD_WINIO__) && (__BUILD_WIN32__))

#include <windows.h>
#include <winioctl.h>
//#include "winio_nt.h"
#include "winiodll.h"

static HANDLE hDriver = INVALID_HANDLE_VALUE;
static int IsNT;
int IsWinIoInitialized = 0;
static char szWinIoDriverPath[MAX_PATH]={0};


int IsWinNT()
{
  OSVERSIONINFO OSVersionInfo;
  OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&OSVersionInfo);
  return OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;
}


int GetDriverPath()
{

  PSTR pszSlash;

  if (!GetModuleFileName(GetModuleHandle(NULL), szWinIoDriverPath, sizeof(szWinIoDriverPath)))
    return 0;

  pszSlash = strrchr(szWinIoDriverPath, '\\');
  if (pszSlash)
    pszSlash[1] = 0;
  else
    return 0;

	strcat(szWinIoDriverPath, "winio.sys");
	
	//szWinIoDriverPath[0] = 0;
	//strcpy(szWinIoDriverPath,"winio.sys");
	return 1;
}


int  InitializeWinIo()
{
  int bResult;
  DWORD dwBytesReturned;

  IsNT = IsWinNT();
  if (IsNT)
  {
    hDriver = CreateFile("\\\\.\\WINIO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    // If the driver is not running, install it
    if (hDriver == INVALID_HANDLE_VALUE) {
      GetDriverPath();
      bResult = InstallWinIoDriver(szWinIoDriverPath, 1);
      if (!bResult)
        return 0;

      bResult = StartWinIoDriver();
      if (!bResult)
        return 0;

      hDriver = CreateFile("\\\\.\\WINIO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hDriver == INVALID_HANDLE_VALUE)
        return 0;
    }

    // Enable I/O port access for this process
    if (!DeviceIoControl(hDriver, IOCTL_WINIO_ENABLEDIRECTIO, NULL, 0, NULL, 0, &dwBytesReturned, NULL))
      return 0;

  }

/*
  else
  {
    VxDCall = (DWORD (WINAPI *)(DWORD,DWORD,DWORD))GetK32ProcAddress(1);
    hDriver = CreateFile("\\\\.\\WINIO.VXD", 0, 0, 0, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);
    if (hDriver == INVALID_HANDLE_VALUE)
      return 0;
  }
*/
  IsWinIoInitialized = 1;

  return 1;
}


void  ShutdownWinIo()
{
  DWORD dwBytesReturned;

  if (IsNT) {
    if (hDriver != INVALID_HANDLE_VALUE) {
      // Disable I/O port access
      DeviceIoControl(hDriver, IOCTL_WINIO_DISABLEDIRECTIO, NULL, 0, NULL, 0, &dwBytesReturned, NULL);
      CloseHandle(hDriver);
    }
    RemoveWinIoDriver();
  }
  /*
  else
    CloseHandle(hDriver);
*/
  IsWinIoInitialized = 0;
  hDriver = NULL;
}

#endif

