#ifndef DISPLAYLIB_LIB_H
#define DISPLAYLIB_LIB_H

#define DISPLAYLIB_OK		1
#define DISPLAYLIB_ERROR	0

#define DISPLAYLIB_OPEN_WITH_USERNAME 0x01
#define DISPLAYLIB_INVERTMODE	1000
#define DISPLAYLIB_ERROR_WRONG_MODE	-2
#define DISPLAYLIB_ERROR_FILE_NOT_FOUND -100
#define DISPLAYLIB_ERROR_ALLOCATING_MEMORY -101
#define DISPLAYLIB_ERROR_NOT_ENOUGH_ROOM_IN_DEVICE -200
#define DISPLAYLIB_ERROR_ERROR_SAVING_DATA_OBJECT -201

#define DISPLAYLIB_TRUE		1
#define DISPLAYLIB_FALSE	0

#define EXPORT0_IO          1
#define EXPORT0_UART_I2C    2
#define EXPORT0_PWM_I2C     3
#define EXPORT0_UART_IO     4
#define EXPORT0_T6963C      5
#define EXPORT0_KS0108      6

#define EXPORT1_IO          1
#define EXPORT1_SPI_PWM     2
#define EXPORT1_SPI_IO      3
#define EXPORT1_TOUCHSCREEN 4
#define EXPORT1_T6963C      5
#define EXPORT1_KS0108      6

#define SPI_SSEL_AUTO		0
#define SPI_SSEL_BUFFER		1
#define SPI_SSEL_MANUAL		2

#define SPI_SSEL_LOW		0
#define SPI_SSEL_HIGH		1

#define OPEN_WITH_USERNAME 0x01

#define USB13700TXTLAYERADDR			0x0000
#define USB13700GFXLAYERADDR			0x1000


#ifdef __cplusplus

#ifdef DISPLAYLIB_DLL
#define DISPLAYLIB_API _declspec(dllexport)
#else
#define DISPLAYLIB_API _declspec(dllimport)
#endif

extern "C" {
#else
#include <windows.h>
#define DISPLAYLIB_API __stdcall
#endif



typedef enum
{
	PF_1BPP = 0,
	PF_2BPP,
	PF_4BPP,
	PF_8BPP,
	PF_16BPP,
	PF_32BPP
} PIXEL_FORMAT;

typedef enum
{
	DIV_4_1 = 0,
	DIV_8_1,
	DIV_16_1
} FPSHIFT_DIV;

typedef struct
{
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelFormat;
	uint32_t Mode;
    char Name[64];   
    char Username[16]; // user can give each display a unique name that makes it easy to handle 
                       // several same type displays at the same time
	uint32_t Handle;
	uint32_t Version;
	uint32_t DispClkDiv;
	uint32_t FPSHIFTDiv;
	uint32_t BacklightControlPWM;
	uint32_t BacklightCCFLPWM;
	uint32_t BacklightConfig;
	uint32_t BacklightPWMFrequency;
	uint32_t TCRCRDiff;
} DisplayInfo;

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
DISPLAYLIB_API int DisplayLib_GetNumberOfDisplays(void);

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
//! \return Returns DISPLAYLIB_OK if configuration was read successfully. In
//! case of a failure DISPLAYLIB_ERROR is returned. If the display is not 
//! available DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_GetDisplayConfiguration(uint32_t index, DisplayInfo *di);

//*****************************************************************************
//
//! Open the display.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to open.
//! \param flags can be used to give options to the open command. Currently
//! only supported flag is DISPLAYLIB_OPEN_WITH_USERNAME which uses the Username
//! field from the \e DisplayInfo structure to open a display with specific
//! user defined name. If no flags are given the default behavior is to open
//! the first USB13700 display found.
//!
//! This function opens the display and prepares it for further use. 
//!
//! \return Returns DISPLAYLIB_OK if display was opened successfully. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_OpenDisplay(DisplayInfo *di, uint32_t flags);

//*****************************************************************************
//
//! Close the display.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to close.
//!
//! This function closes the display and frees any allocated resources. 
//!
//! \return Returns DISPLAYLIB_OK if display was closed successfully. If
//! supplied DisplayInfo structure is invalid DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_CloseDisplay(DisplayInfo *di);

//*****************************************************************************
//
//! Draw full screen image to the display.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param fb is a pointer to the buffer including the data to write to the 
//! display.
//!
//! This function updates the full screen with data from the buffer. The buffer
//! size has to be display width * display heigth bytes in size. One byte
//! represents one pixel.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_DrawScreen(DisplayInfo *di, uint8_t *fb);

//*****************************************************************************
//
//! Draw a rectangle area to the display.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param fb is a pointer to the buffer including the data to write to the 
//! display.
//!
//! This function updates an area on the screen with data from the buffer. The buffer
//! size has to be area width * area heigth bytes in size. One byte
//! represents one pixel. Area X coordinate has to be dividable by 8 and also the area 
//! width has to be dividable by 8.
//!
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_DrawRect(DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

//*****************************************************************************
//
//! Draw a rectangle area to the display from a full screen sized buffer.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param fb is a pointer to the buffer including the data to write to the 
//! display.
//!
//! This function updates an area on the screen with data from the buffer. The buffer
//! size has to be display width * display heigth bytes in size. One byte
//! represents one pixel. Area X coordinate has to be dividable by 8 and also the area 
//! width has to be dividable by 8.
//!
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_DrawRectFromScrBuf(DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

//*****************************************************************************
//
//! Set backlight values.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param onOff is the backlight state. 0 = OFF  1 = ON 
//! \param brightness is the backlight brightness. (0-255)
//!
//! This function sets the backlight values.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_SetBacklight(DisplayInfo *di, uint32_t onOff, uint32_t brightness);

//*****************************************************************************
//
//! Set DisplayLib options.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param option is the name of the option to set. 
//! \param value is the value to set the option to.
//!
//! This function sets the DisplayLib options. Currently supported option is 
//! DISPLAYLIB_INVERTMODE which has values DISPLAYLIB_TRUE or DISPLAYLIB_FALSE.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_SetDisplayLibOption(DisplayInfo *di, uint32_t option, uint32_t value);

//*****************************************************************************
//
//! Gets version information.
//!
//! \param libFWVer is a pointer to unsigned integer where to store the 
//! DisplayLib firmware version. 
//! \param libVer is a pointer to unsigned integer where to store the
//! DisplayLib version.
//!
//! This function gets version information for the DisplayLib.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_VersionInfo(uint32_t *libFWVer, uint32_t *libVer);

//*****************************************************************************
//
//! Writes text to the display.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param text is a pointer to the string to write. 
//! \param size is the length of the string.
//! \param x is the X position where to write the text.
//! \param y is the Y position where to write the text.
//!
//! This function writes text on the display. Text is written on the S1D13700
//! character layer so the coordinates are also character coordinates. 
//! Character set is the S1D13700 character set and font size is 8x8 pixels.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_WriteText(DisplayInfo *di, char *text, uint32_t size, uint32_t x, uint32_t y);

//*****************************************************************************
//
//! Write S1D13700 command byte.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param cmd is the S1D13700 command byte. 
//! \param queue determines if the queue functionality is used. 0 = no queue 
//! 1 = use queue
//!
//! This function writes a S1D13700 command byte. For the S1D13700 commands 
//! and their usage refer to the S1D13700 data sheet. The queue option allows 
//! collecting more commands and data to a queue before sending them to the 
//! controller board. This helps with USB latency when writing many short 
//! data packets and in some situations can significantly improve performance. 
//! The queue buffer is automatically allocated as more data is placed in it.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_WriteS1D13700Command(DisplayInfo *di, uint8_t cmd, uint32_t queue);

//*****************************************************************************
//
//! Write S1D13700 data byte.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param data is the S1D13700 command byte. 
//! \param queue determines if the queue functionality is used. 0 = no queue 
//! 1 = use queue
//!
//! This function writes one S1D13700 data byte. 
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_WriteS1D13700DataByte(DisplayInfo *di, uint8_t data, uint32_t queue);

//*****************************************************************************
//
//! Write S1D13700 data bytes.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param data is a pointer to the buffer where the data is. 
//! \param size is the number of data bytes to write. 
//! \param queue determines if the queue functionality is used. 0 = no queue 
//! 1 = use queue
//!
//! This function writes S1D13700 data bytes from a buffer. Useful when larger 
//! data blocks are written.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_WriteS1D13700Data(DisplayInfo *di, uint8_t *data, uint32_t size, uint32_t queue);

//*****************************************************************************
//
//! Write S1D13700 queue to the controller.
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//!
//! This function writes the S1D13700 queue to the controller. Before calling this 
//! function none of the commands and data written with the 
//! \e WriteS1D13700Command, \e WriteS1D13700DataByte and \e WriteS1D13700Data
//! functions are actually sent if the queue option was used. After calling this 
//! function the queue is again empty and new commands and data can be placed
//! in it.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_WriteS1D13700ExecuteQueue(DisplayInfo *di);

//*****************************************************************************
//
//! Clear S1D13700 queue.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//!
//! This function clears the S1D13700 command and data queue. After calling this 
//! function the queue is empty.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ClearS1D13700Queue(DisplayInfo *di);

//*****************************************************************************
//
//! Get S1D13700 queue length.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param length is a pointer to unsigned integer where to store the
//! current queue length.
//!
//! This function gets the S1D13700 queue length. 
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_GetS1D13700QueueLength(DisplayInfo *di, uint32_t *length);

//*****************************************************************************
//
//! Set S1D13700 clock dividers.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param DispClkDiv is the the divider for setting the S1D13700 clock.
//! \param FPShiftDiv is the S1D13700 internal divider.
//!
//! This command allows settings the S1D13700 frequency settings on the fly. 
//! The settings are not saved so after powering off the controller the settings 
//! saved with the configuration tool are again used.
//!
//! The formula for the clock is CLK = 30 Mhz / (DispClkDiv + 1)
//! 
//! DispClkDiv : Clock frequency
//! - 1 : 15 MHz
//! - 2 : 10 MHz
//! - 3 : 7.5 MHz
//! - 4 : 6 MHz
//! - 5 : 5 MHz
//! - 6 : 4.29 MHz
//! - 7 : 3.75 MHz
//! - 8 : 3.33 MHz
//! - 9 : 3 MHz
//! - 10 : 2.73 MHz
//!
//! FPShiftDiv supports values
//! - 0 - 4:1
//! - 1 - 8:1
//! - 2 - 16:1
//! 
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_SetS1D13700ClockDiv(DisplayInfo *di, uint8_t DispClkDiv, uint8_t FPShiftDiv);

//*****************************************************************************
//
//! Configure expansion port function.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param port0 is the function to set for the 6-bit PORT0.
//! \param port1 is the function to set for the 8-bit PORT1.
//!
//! Sets the expansion port functionality. There are total 14 pins available
//! that have been divided in one 6 pin port (PORT0) and one 8 pin port (PORT1).
//! 
//! PORT0
//! - bit 0 - expansion connector pin 6
//! - bit 1 - expansion connector pin 5
//! - bit 2 - expansion connector pin 13 (open drain)
//! - bit 3 - expansion connector pin 14 (open drain)
//! - bit 4 - expansion connector pin 16 (open drain)
//! - bit 5 - expansion connector pin 17 (open drain)
//! 
//! PORT1
//! - bit 0 - expansion connector pin 18
//! - bit 1 - expansion connector pin 12
//! - bit 2 - expansion connector pin 11
//! - bit 3 - expansion connector pin 9
//! - bit 4 - expansion connector pin 10
//! - bit 5 - expansion connector pin 7
//! - bit 6 - expansion connector pin 4
//! - bit 7 - expansion connector pin 3
//!
//! Currently supported configurations for PORT0:
//! - EXPORT0_IO
//!
//! Currently supported configurations for PORT1:
//! - EXPORT1_IO
//! - EXPORT1_SPI_IO
//!
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortConfig(DisplayInfo *di, uint8_t port0, uint8_t port1);

//*****************************************************************************
//
//! Configure expansion port in I/O port mode.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param port0Dir is the direction mask to set for the 6-bit PORT0.
//! \param port1Dir is the direction mask to set for the 8-bit PORT1.
//!
//! Sets the expansion port I/O port direction in I/O mode. Set a bit to 1 
//! to configure it as an output. If the port is not configured in I/O mode
//! setting the direction has no effect.
//! 
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortIOConfig(DisplayInfo *di, uint8_t port0Dir, uint8_t port1Dir);

//*****************************************************************************
//
//! Set port pins in I/O port mode.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param port0Mask is the mask for PORT0.
//! \param port0 is the data to write to PORT0.
//! \param port1Mask is the mask for PORT1.
//! \param port1 is the data to write to PORT1.
//!
//! Sets the expansion port I/O port pins in I/O mode. If the port is not 
//! configured in I/O mode setting the pins has no effect.
//! Both ports have mask bytes. Set the mask to all 0's to not update the port.
//! 
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortIOSet(DisplayInfo *di, uint8_t port0Mask, uint8_t port0, uint8_t port1Mask, uint8_t port1);

//*****************************************************************************
//
//! Get port pin state in I/O port mode.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param port0 is a pointer to unsigned char where to store the PORT0 state.
//! \param port1 is a pointer to unsigned char where to store the PORT1 state.
//!
//! Reads the expansion port I/O pins in I/O mode.
//! 
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortIOGet(DisplayInfo *di, uint8_t *port0, uint8_t *port1);

//*****************************************************************************
//
//! Configure SPI settings in SPI mode.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param frameLength is the SPI frame length in bits. (3 to 15)
//! \param frameFormat is the serial frame format. (set to 0 for SPI)
//! \param cpol is the SPI clock polarity. (0 or 1)
//! \param cpha is the SPI clock phase. (0 or 1)
//! \param bitclock is the SPI clock divider. 
//! \param prescaler is the SPI clock prescaler. (2 to 254)
//! \param sselMode is the SPI SSEL mode.
//!
//! Configures SPI settings in SPI mode. If the port is not configured to
//! SPI mode these settings have no effect. Frame lengths supported are 
//! 4 to 16 bits. Bitrate is calculated using formula 
//! 60000000 / (prescaler * [bitclock+1]).
//!
//! SSEL modes supported are
//! - SPI_SSEL_AUTO
//!    - This is the mode normally used. SSEL is handled automatically for every frame.
//! - SPI_SSEL_BUFFER
//!    - In this mode SSEL gets asserted before writing the buffer and after all frames
//!      in the buffer have been written SSEL is deasserted. During writing the
//!      buffer SSEL stays asserted.
//! - SPI_SSEL_MANUAL
//!    - SSEL needs to be controlled manually using ExPortSPISetSSEL().
//!
//! Possible options for \e frameFormat
//! - 0 : SPI (only one officially supported for now)
//! - 1 : SSI
//! - 2 : Microwire
//!
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortSPIConfig(DisplayInfo *di, uint8_t frameLength, uint8_t frameFormat, 
													   uint8_t cpol, uint8_t cpha, uint8_t bitclock, uint8_t prescaler, uint8_t sselMode);

//*****************************************************************************
//
//! Send SPI data.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param data is a pointer to the buffer where the data is. 
//! \param size is the number of data bytes to write. 
//!
//! Sends SPI data. If SPI frame size is configured to 8 bits or smaller 
//! one byte per frame is used from the buffer. If frame size is 9 to 16 bits 
//! one SPI frame is constructed from 2 bytes from the buffer.
//! 
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortSPISend(DisplayInfo *di, uint8_t *data, uint32_t size);

//*****************************************************************************
//
//! Receive SPI data.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param rcvData is a pointer to unsigned integer where to store the
//! SPI frame received. 
//!
//! Receives one SPI frame.
//! 
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortSPIReceive(DisplayInfo *di, uint32_t * rcvData);

//*****************************************************************************
//
//! Set SPI SSEL state.
//!
//! \since 0.9
//!
//! \param di is a pointer to the \e DisplayInfo structure of the display 
//! to use.
//! \param ssel is the SSEL pin state. (SPI_SSEL_LOW or SPI_SSEL_HIGH)
//!
//! Sets the SPI SSEL pin state manually. SPI SSEL mode needs to be configured 
//! to SPI_SSEL_MANUAL for this to work.
//! 
//! Requires firmware version 0.9 or newer.
//!
//! \return Returns DISPLAYLIB_OK if success. In
//! case of a failure DISPLAYLIB_ERROR is returned.
//
//*****************************************************************************
DISPLAYLIB_API int DisplayLib_ExPortSPISetSSEL(DisplayInfo *di, uint32_t ssel);


DISPLAYLIB_API int DisplayLib_SetStartupBitmap(DisplayInfo *di, char *bitmapFile);
DISPLAYLIB_API int DisplayLib_SetupDisplayProperties(DisplayInfo *di);
DISPLAYLIB_API int DisplayLib_UFW(DisplayInfo *di);

// Rest is experimental and will likely change
DISPLAYLIB_API int DisplayLib_GetTouchScreenSample(DisplayInfo *di, uint32_t *x, uint32_t *y);
DISPLAYLIB_API int DisplayLib_SPI8Bit(DisplayInfo *di, uint8_t dc, uint8_t txData, uint8_t *rxData);


#ifdef __cplusplus
}
#endif

#endif
