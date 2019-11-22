
// libusb13700 - http://mylcd.sourceforge.net/

// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.



#ifndef _LIBUSB13700_H_
#define _LIBUSB13700_H_


#define USB13700_VID					0x16C0
#define USB13700_PID					0x08A2 
#define EP_IN							0x82
#define EP_OUT							0x02

#define DISPLAYLIB_OK					1
#define DISPLAYLIB_ERROR				0
#define DISPLAYLIB_TRUE					1
#define DISPLAYLIB_FALSE				0

#define DISPLAYLIB_OPEN_WITH_USERNAME 0x01
#define DISPLAYLIB_INVERTMODE	1000
#define DISPLAYLIB_ERROR_WRONG_MODE	-2
#define DISPLAYLIB_ERROR_FILE_NOT_FOUND -100
#define DISPLAYLIB_ERROR_ALLOCATING_MEMORY -101
#define DISPLAYLIB_ERROR_NOT_ENOUGH_ROOM_IN_DEVICE -200
#define DISPLAYLIB_ERROR_ERROR_SAVING_DATA_OBJECT -201

#define CMD_GET_BASICCONFIGDATA			1
#define CMD_WRITE_FULLSCREEN			2
#define CMD_SET_BACKLIGHT				4
#define CMD_GET_BACKLIGHTCONFIGDATA		5
#define CMD_SET_STARTUPBITMAP			6
#define CMD_WRITE_S1D13700_COMMAND		7
#define CMD_WRITE_S1D13700_DATA 		8
#define CMD_WRITE_S1D13700_DATA_BYTE	9
#define CMD_WRITE_TEXT					0x0A
#define CMD_GET_TOUCHSCREEN_SAMPLE		0x0C
#define CMD_WRITE_EXPORTCONFIG			0x0F
#define CMD_WRITE_EXPORTIOCONFIG		0x10
#define CMD_SET_EXPORTIO				0x11
#define CMD_WRITE_EXPORTSPICONFIG		0x13
#define CMD_WRITE_EXPORTSPI				0x14

#define MSG_COMMAND			0x0A
#define MSG_REPLY			0x14
#define MODE_NORMAL			0x14
#define FW_CMD_SUCCESS		1
#define FW_CMD_ERROR		0


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

#define OPEN_WITH_USERNAME	0x01

#define USB13700TXTLAYERADDR			0x0000
#define USB13700GFXLAYERADDR			0x1000

#define LIBUSB13700_VERSION				0x0001

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
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


#define MSG_QUEUE_SIZE	2048
#define MSG_QUEUE_STEP	512

typedef struct{
	unsigned char *Data;
	int Size;
	int Position;
}TLIBUSB13700QUEUE;

typedef struct {
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelFormat;
	uint32_t Mode;
    char Name[64];   
    char Username[16]; // user can give each display a unique name that makes it easy to handle 
                       // several same type displays at the same time
	uintptr_t Handle;
	uint32_t Version;
	uint32_t DispClkDiv;
	uint32_t FPSHIFTDiv;
	uint32_t BacklightControlPWM;
	uint32_t BacklightCCFLPWM;
	uint32_t BacklightConfig;
	uint32_t BacklightPWMFrequency;
	uint32_t TCRCRDiff;
	
	TLIBUSB13700QUEUE *Queue;
}DisplayInfo;




#ifdef __cplusplus
extern "C" {
#endif


int libusb13700_GetNumberOfDisplays ();
int libusb13700_GetDisplayConfiguration (uint32_t index, DisplayInfo *di);
int libusb13700_VersionInfo (DisplayInfo *di, uint32_t *fw, uint32_t *ver);
int libusb13700_OpenDisplay (DisplayInfo *di, uint32_t flags);
int libusb13700_CloseDisplay (DisplayInfo *di);
int libusb13700_DrawScreen (DisplayInfo *di, uint8_t *fb);
int libusb13700_WriteText (DisplayInfo *di, char *text, uint32_t size, uint32_t x, uint32_t y);
int libusb13700_WriteS1D13700Command (DisplayInfo *di, uint8_t cmd, uint32_t queue);
int libusb13700_WriteS1D13700Data (DisplayInfo *di, uint8_t *data, uint32_t size, uint32_t queue);
int libusb13700_WriteS1D13700DataByte (DisplayInfo *di, uint8_t data, uint32_t queue);
int libusb13700_WriteS1D13700ExecuteQueue (DisplayInfo *di);
int libusb13700_WriteS1D13700ClearQueue (DisplayInfo *di);
int libusb13700_WriteS1D13700GetQueueSize (DisplayInfo *di);

int libusb13700_ExPortSPISend (DisplayInfo *di, uint8_t *data, uint32_t size);
int libusb13700_ExPortConfig (DisplayInfo *di, uint8_t port0, uint8_t port1);
int libusb13700_ExPortIOConfig (DisplayInfo *di, uint8_t port0Dir, uint8_t port1Dir);
int libusb13700_ExPortSPIConfig (DisplayInfo *di, uint8_t frameLength, uint8_t frameFormat, 
								 uint8_t cpol, uint8_t cpha, uint8_t bitclock, uint8_t prescaler, uint8_t sselMode);
int libusb13700_ExPortIOSet (DisplayInfo *di, uint8_t port0Mask, uint8_t port0, uint8_t port1Mask, uint8_t port1);

int libusb13700_SetBacklight (DisplayInfo *di, uint32_t onOff, uint32_t brightness);
int libusb13700_GetTouchScreenSample (DisplayInfo *di, uint32_t *x, uint32_t *y);



// TODO:
int libusb13700_SetStartupBitmap (DisplayInfo *di, uint8_t *fb);
int libusb13700_ExPortIOGet (DisplayInfo *di, uint8_t *port0, uint8_t *port1);
int libusb13700_ExPortSPIReceive (DisplayInfo *di, uint32_t *rcvData);
int libusb13700_SPI8Bit (DisplayInfo *di, uint8_t dc, uint8_t txData, uint8_t *rxData);
int libusb13700_DrawRectFromScrBuf (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
int libusb13700_DrawRect (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height);


#ifdef __cplusplus
}
#endif


#endif 

