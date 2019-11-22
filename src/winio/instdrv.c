// ---------------------------------------------------- //
//                      WinIo v2.0                      //
//  Direct Hardware Access Under Windows 9x/NT/2000/XP  //
//           Copyright 1998-2002 Yariv Kaplan           //
//               http://www.internals.com               //
// ---------------------------------------------------- //

#include "mylcd.h"

#if ((__BUILD_WINIO__) && (__BUILD_WIN32__))

#include <windows.h>
#include "winiodll.h"



int InstallWinIoDriver (PSTR pszWinIoDriverPath, int IsDemandLoaded)
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;

  // Remove any previous instance of the driver

  RemoveWinIoDriver();
  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCManager) {
    // Install the driver
    
    hService = CreateService(hSCManager,
                             "WINIO",
                             "WINIO",
                             SERVICE_ALL_ACCESS,
                             SERVICE_KERNEL_DRIVER,
                             (IsDemandLoaded == 1) ? SERVICE_DEMAND_START : SERVICE_SYSTEM_START,
                             SERVICE_ERROR_NORMAL,
                             pszWinIoDriverPath,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);

    CloseServiceHandle(hSCManager);

    if (hService == NULL)
      return 0;
  }
  else
    return 0;

  CloseServiceHandle(hService);
  
  return 1;
}


int RemoveWinIoDriver ()
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;
  int bResult;

  StopWinIoDriver();
  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCManager) {
    hService = OpenService(hSCManager, "WINIO", SERVICE_ALL_ACCESS);
    CloseServiceHandle(hSCManager);
    
    if (hService) {
      bResult = DeleteService(hService);
      CloseServiceHandle(hService);
    } else
      return 0;
  } else
    return 0;

  return bResult;
}


int  StartWinIoDriver ()
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;
  int bResult;

  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (hSCManager) {
    hService = OpenService(hSCManager, "WINIO", SERVICE_ALL_ACCESS);
    CloseServiceHandle(hSCManager);

    if (hService) {
      bResult = StartService(hService, 0, NULL) || GetLastError() == ERROR_SERVICE_ALREADY_RUNNING;
      CloseServiceHandle(hService);
    } else
      return 0;
  } else
    return 0;

  return bResult;
}


int StopWinIoDriver ()
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;
  SERVICE_STATUS ServiceStatus;
  int bResult;

  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCManager) {
    hService = OpenService(hSCManager, "WINIO", SERVICE_ALL_ACCESS);
    CloseServiceHandle(hSCManager);

    if (hService) {
      bResult = ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);

      CloseServiceHandle(hService);
    } else
      return 0;
  } else
    return 0;

  return bResult;
}


#endif

