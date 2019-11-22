#ifndef USBD480_LIB_H
#define USBD480_LIB_H



#define USBD480_OK		1
#define USBD480_ERROR	0

#define USBD480_OPEN_WITH_USERNAME 0x01


#ifdef __cplusplus

#ifdef USBD480_LIB_DLL
#define USBD480_API _declspec(dllexport)
#else
#define USBD480_API _declspec(dllimport)
#endif
extern "C" {
#else
#include <windows.h>
#define USBD480_API __stdcall
#endif

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef enum
{
	PF_1BPP = 0,
	PF_1BPP_DS,
	PF_2BPP,
	PF_4BPP,
	PF_8BPP,
	PF_16BPP_RGB565,
	PF_16BPP_BGR565,
	PF_32BPP
} PIXEL_FORMAT;

typedef struct
{
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelFormat;
    char Name[32];   
    char Username[16]; // user can give each display a unique name that makes it easy to handle 
                       // several same type displays at the same time
	uint32_t Handle;
	uint32_t Version;
} DisplayInfo;

typedef struct
{
	uint16_t x;	// x sample
	uint16_t y;	// y sample
	uint16_t z1; // z1 sample
	uint16_t z2; // z2 sample
	uint8_t pen; // 0=pen down, 1=pen up
	uint8_t pressure; // firmware calculated pressure value
	uint8_t reserved[6]; // reserved for future use
} TouchReport;

//*****************************************************************************
//
//! Returns the number of attached displays.
//!
//! This function returns the number of attached displays. Returns the
//! number of all attached displays even if they are in use and not 
//! available.
//!
//! \return Returns the number of displays attached.
//
//*****************************************************************************
USBD480_API int USBD480_GetNumberOfDisplays(void);

//*****************************************************************************
//
//! Get display configuration information for specified display.
//!
//! \param index is the number of the display for which the information is read.
//! \param di is a pointer to the \e DisplayInfo structure where the information 
//! will be saved.
//!
//! This function fills the configuration data structure for the display which 
//! index is given in parameter \e index.
//!
//! \return Returns USBD480_OK if configuration was read successfully. In
//! case of a failure USBD480_ERROR is returned. If the display is not 
//! available USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_GetDisplayConfiguration(uint32_t index, DisplayInfo *di);

//*****************************************************************************
//
//! Open the display.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to open.
//! \param flags can be used to give options to the open command. Currently
//! only supported flag is USBD480_OPEN_WITH_USERNAME which uses the Username
//! field from the \e DisplayInfo structure to open a display with specific
//! user defined name. If no flags are given the default behavior is to open
//! the first USBD480 display found.
//!
//! This function opens the display and prepares it for further use. 
//!
//! \return Returns USBD480_OK if display was opened successfully. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_Open(DisplayInfo *di, uint32_t flags);

//*****************************************************************************
//
//! Close the display.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to close.
//!
//! This function closes the display and frees any allocated resources. 
//!
//! \return Returns USBD480_OK if display was closed successfully. If
//! supplied DisplayInfo structure is invalid USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_Close(DisplayInfo *di);

//*****************************************************************************
//
//! Draw full screen image to the display from RGB565 buffer.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param fb is a pointer to the buffer including the data to write to the 
//! display.
//!
//! This function updates the full screen with data from the buffer. The buffer
//! size needs to be at least display width * display heigth * 2 bytes in size 
//! (for RGB565 pixel data).
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_DrawFullScreen(DisplayInfo *di, uint8_t *fb);

//*****************************************************************************
//
//! Draw full screen image to the display from RGBA buffer.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param fb is a pointer to the buffer including the data to write to the 
//! display.
//!
//! This function updates the full screen with data from the buffer. The buffer
//! size needs to be at least display width * display heigth * 4 bytes in size 
//! (for RGB pixel data). Format is RGBA => 0xAABBGGRR
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_DrawFullScreenRGBA32(DisplayInfo *di, uint32_t *fb);

//*****************************************************************************
//
//! Draw full screen image to the display from BGRA buffer.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param fb is a pointer to the buffer including the data to write to the 
//! display.
//!
//! This function updates the full screen with data from the buffer. The buffer
//! size needs to be at least display width * display heigth * 4 bytes in size 
//! (for RGB pixel data). Format is BGRA => 0xAARRGGBB
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_DrawFullScreenBGRA32(DisplayInfo *di, uint32_t *fb);

//*****************************************************************************
//
//! Set display brightness.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param brightness is the backlight brightness. (0-255)
//!
//! This function sets the backlight values.
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_SetBrightness(DisplayInfo *di, uint32_t brightness);

//*****************************************************************************
//
//! Set touchscreen report mode.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param mode is the touch mode. 
//!
//! This function sets the touchscreen report mode. There are different report
//! modes available that can report either raw touch samples or then samples
//! with some filtering applied.
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_SetTouchMode(DisplayInfo *di, uint32_t mode);

//*****************************************************************************
//
//! Set address pointer in the frame buffer memory.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param address is the address in the frame buffer memory.
//!
//! This function sets the frame buffer address pointer.
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_SetAddress(DisplayInfo *di, uint32_t address);

//*****************************************************************************
//
//! Set frame start address in the frame buffer.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param address is the address in the frame buffer memory.
//!
//! This function sets the start address of the currently visible frame inside 
//! the frame buffer. 
//! Default frame start address is 0. The actual address change is synchronized
//!  with VSYNC inside the controller hardware.
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_SetFrameStartAddress(DisplayInfo *di, uint32_t address);

//*****************************************************************************
//
//! Draw to the display from a buffer.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param fb is a pointer to the buffer including the data to write to the 
//! display.
//! \param size is the size of the data. (data is expected in 512 byte blocks) 
//!
//! This function writes the data to the frame buffer memory starting from 
//! the frame buffer address pointer. Address in automatically incremented
//! as data is written.
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_DrawFromBuffer(DisplayInfo *di, uint8_t *fb, uint32_t size);

//*****************************************************************************
//
//! Get touch screen report.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param x is a pointer to the touch report struct
//!
//! This function gets the touch screen sample report.
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_GetTouchReport(DisplayInfo *di, TouchReport *touch);

//*****************************************************************************
//
//! Get touch screen position.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param x is a pointer to unsigned integer where to store the X position
//! \param y is a pointer to unsigned integer where to store the Y position
//!
//! This function gets the raw touch screen position reading. The values 
//! are the output from the A/D converter without any filtering.
//!
//! \return Returns USBD480_OK if success. In
//! case of a failure USBD480_ERROR is returned.
//
//*****************************************************************************
USBD480_API int USBD480_GetTouchPosition(DisplayInfo *di, uint32_t *x, uint32_t *y);



USBD480_API int USBD480_SetStartupImage(DisplayInfo *di, uint8_t *buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
