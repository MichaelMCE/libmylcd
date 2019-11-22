SwitchBlade SDK Guide
version 2.0.0 [April 2013]

Contents
=======================
1) Description
2) Development System Requirements
3) Supported Razer Platforms:
4) Installation
5) Uninstallation
6) Installation Verification
7) Sample applications
8) Using SB SDK
9) Change from the last release
10) Known limitations
11) Revision History

1) Description
--------------------------------
  This document lists the steps needed to setup/use SwitchBlade SDK. Package
  includes the platform drivers for supported platforms.

2) Development System Requirements
--------------------------------
 - Windows Vista or higher (x64/x86)
 - Visual Studio 2010 SP1 

3) Supported Razer Platforms
--------------------------------
 - Razer Blade
 - Razer DeathStalker Ultimate Keyboard
 - Razer SWTOR Keyboard

4) Installation
--------------------------------

 **NOTE**: Do not use any of the installed RzApps (RzYoutube/RzMacro/...) while
           installation is in progress.

 - Install Synapse 2.0, and make sure you have received all software updates
 - As the SDK comes as a zip file, extract the files and run the Installer exe 

5) Uninstallation
--------------------------------
 - Go to C:\Razer\SwitchBladeSDK\v2.0
 - Double click on Uninstall.exe.

6) Installation Verification
--------------------------------
 - Go to C:\Razer\SwitchBladeSDK\v1.5.3\bin to find the pre-built samples
 - The pre-built sample binaries are bundled:
     CamStreamer, FunctionTest
 - Run them to test the functionality of the SDK. 

7) Sample applications
--------------------------------

 - CamStreamer:     Grabs a frame from the selected webcam, converts it to
                    RGB565 and renders it on the widget area. 

                    This app shows how to use RzSBStart()/RzSBStop() and
                    RzSBRenderBuffer() APIs.

 - FunctionTest:    Test application for exercising most of the remaining
                    APIs. Allows you to load images onto DKM keys or the Widget
                    area. Also, prints all gesture / keypress events. Has a
                    mechanism to individually turn on/off gestures as well
                    as turn on/off gesture forwarding to Windows.

 Source code for these applications is available at
 C:\Razer\SwitchbladeSDK\v2.0\source\apps. 
 This location includes a VS2010 solution file to build these samples.


8) Using the SB SDK
--------------------------------
 
 - Include "Switchblade.h" in your CPP. 
 - Change project settings to link with "RzSwitchbladeSDK2.lib" located in
   C:\Razer\SwitchBladeSDK\v2.0\lib.

More details on APIs / usages can be found in the file 
"SwitchBlade SDK v2.0.pdf" located in C:\Razer\SwitchBladeSDK\v2.0\doc. 

To use the C# SDK Wrapper, extract the files from the archive
"RzSwitchbladeSDK2.CSharpWrapper.zip" to your project directories. 
Either reference the dll or use the CS file included. 

9) Changes from the last release
--------------------------------
- The APIs have been updated to support new gestures and events 
  Please refer to the "SwitchBlade SDK v2.0.pdf" for the detailed list. 
- Added support for keyboard capture 
- Added support for Vista and up version of Windows 


10) Known limitations
--------------------------------
- The setting the OS Gesture of MOVE also sets the OS Gesture for TAP
- After the SDK application gets deactivated (i.e. pressing Razer Key), 
  upon reactivation the Touchpad (mouse and buttons) gets disabled. 
  The fix will be released in the next SDK patches and SB Framework updates. 
- Setting of DK images using a buffer is not currently supported 
  (only Trackpad image buffer is supported).
  The feature will be released in the next SDK patches. 


11) Revision History

-----[ v1.x.x to v2.0]-----
- Updated for compatibility with the latest Switchblade Framework (2.0)
- Miniumum OS requirement set to Windows Vista 
- Added support for new gestures (MOVE, RELEASE) 
- Added support for new application events (ACTIVATE, DEACTIVATE, 
  CLOSE, EXIT) 
- Added support for Keyboard capture 
- COM initialization to MTA will only be set if the application did not 
  perform initialization prior to calling RzSbStart() 
- Added information for support of DeathStalker Ultimate keyboard
- Removed the application event for APPMODE_CHANGED
  (replaced by new events)
- Removed functionalities for WinRender 
  (refer to "SwitchBlade SDK v2.0.pdf" for instructions on how to 
  implement Window rendering using the available APIs. 
- Removed support for ISV Module Device 
- Updated the sample application to include Application event logs, 
  Keyboard capture logs, setting of trackpad image. 
- Improved the uninstaller to perform directory cleanup when possible.
  Uninstaller have been renamed to "Uninstall.exe"  

-----[ v1.5.2 to v1.5.3]-----
- EULA requirement added. Minimum OS requirement is currently Windows 7.
- Earlier versions of Windows (XP,Vista) will not permit installation.
- Requires Win7 or greater; RzSBStart() will fail otherwise.
- Included rendering statistics when using RzSBStartRenderingWindow()
- Renamed MFCTestRender to MFCSampleRender and released source code
- Restructured and updated HWND Rendering to optimize performance and
  update the gesture translation engine.
- Addressed a variety of subtle issues related to activation and focus
- Removed COM initialization from .DLL; requires applications to perform
  initialization however supports STA and MTA threading models.
- Updated CamStreamer and FunctionTest to perform COM initialization.
- Released HWND-based rendering through RzSBStartRenderingWindow.
- Included MFCTestRender showing use of RzSBStartRenderingWindow.

-----[ v1.1.4.0 to v1.1.4.1 ]-----
- Installer modified to prohibit installation on any OS older than Windows
  Vista.
- Camstreamer sample fixed to work properly on suspend/resume.
- version bump for paths and files.
- Sample source code has been cleaned up some.
- x64 support removed until necessary fixes become available.
- SwitchBladeSDK32.dll is now installed in the DLL search path, rather than
  being co-located with applications.
- Restructured directories and where intermediate data is placed during builds
  for consistency.
- Moved files in source\apps\CamStreamer\CamStreamer to source\apps\CamStreamer.
- Removed CamStreamer.sln
- Fixed issue in Cam Streamer where selecting a different camera from the
  dropdown menu did not turn off the previously used camera.
- Fixed issue in Cam Streamer where the application would leak memory if it
  failed to place lock on a SwitchBlade device.
- Fixed issue in Cam Streamer where clicking the X in the upper right corner
  while streaming would not release the application’s hold on the SwitchBlade
  device.
- Added a destructor to CamStreamer.
                
-----[ v1.1.3.0 to v1.1.3.1 ]----- 
- Moved files in \bin\x86\Release up to \bin.
- Added x64 versions of CamStreamer and FunctionTest.
- Moved files from \lib\x86\Release up to \lib.
- Renamed RZSBFE.lib to SwitchBladeSDK32.lib for 32-bit use and
  SwitchBladeSDK64lib for 64-bit use.
- SwitchBlade SDK DLLs are now properly installed in system directories and are
  in the default search path for DLLs.
- Removed SwitchBlade SDK DLLs from the \lib folder. 
- Renamed functions and variables in CamStreamer sample application for better
  clarity.
- Cleaned up source code in Function Test.
- Changed the SDK threading model from single-threaded to multi-threaded.
                
-----[ v1.1.2.0 to v1.1.3.0 ]----- 
- Updated installed drivers to more recent ones.
- Updated Cam Streamer to stream video in color.
- Switched responses for zoom in and out.
