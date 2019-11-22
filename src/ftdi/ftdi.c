
// libmylcd
// An LCD framebuffer library
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




#include "mylcd.h"

#if (__BUILD_FTDI__)


#include "../device.h"
#include "../memory.h"
#include "../lstring.h"
#include "../misc.h"
#include "ftdi.h"



static int ftdi_OpenDriver (TPORTDRIVER *pd, int port);
static int ftdi_CloseDriver (TPORTDRIVER *pd);
static int ftdi_WritePort8 (const int port, const ubyte data);
static int ftdi_WritePort16 (const int port, const unsigned short data);
static int ftdi_WritePort32 (const int port, const unsigned int data);
static int ftdi_WriteBuffer (const int port, void *buffer, size_t tbytes);
static int ftdi_Flush (TPORTDRIVER *pd);
static int ftdi_setOption (TDRIVER *drv, int option, intptr_t *value);
static int ftdi_getOption (TDRIVER *drv, int option, intptr_t *value);
static ubyte ftdi_ReadPort (const int port);

struct ftdi_context ftdic;
static int isInitiated;
static int bitBangState;
static unsigned int bytesToWrite;			// queue position within buffer
static ubyte *writeBuffer;




int initFTDI (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;
	pd.open = ftdi_OpenDriver;
	pd.close = ftdi_CloseDriver;
	pd.write8 = ftdi_WritePort8;
	pd.write16 = ftdi_WritePort16;
	pd.write32 = ftdi_WritePort32;
	pd.writeBuffer = ftdi_WriteBuffer;
	pd.flush = ftdi_Flush;
	pd.read = ftdi_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = ftdi_setOption;
	pd.getOption = ftdi_getOption;
	pd.optTotal = 6;

	l_strcpy(pd.name, "FTDI");
	l_strcpy(pd.comment, "USB - FTDI via libusb");
	int drvnum = registerPortDriver(rd, &pd);
	setDefaultPortOption(rd, drvnum, lOPT_FTDI_DEVICE, 0);
	setDefaultPortOption(rd, drvnum, lOPT_FTDI_STRUCT, 0);
	setDefaultPortOption(rd, drvnum, lOPT_FTDI_BAUDRATE, DEFAULT_BAUD);
	setDefaultPortOption(rd, drvnum, lOPT_FTDI_LATENCY, 2);
	setDefaultPortOption(rd, drvnum, lOPT_FTDI_IODIR, 0xFF);
	setDefaultPortOption(rd, drvnum, lOPT_FTDI_BITBANG, 1);
	return (drvnum > 0);
}

void closeFTDI (TREGISTEREDDRIVERS *rd)
{
	isInitiated = 0;
	if (writeBuffer)
		l_free(writeBuffer);
	writeBuffer = NULL;
	return;
}

int ftdi_getTotalDevices (struct ftdi_context *ftdic)
{
	struct ftdi_device_list *devlist;
	int total = ftdi_usb_find_all(ftdic, &devlist, FT_VID, FT_PID);
	ftdi_list_free(&devlist);
	return total;
}

int ftdi_listDevices (struct ftdi_context *ftdic)
{
	struct ftdi_device_list *devlist = NULL;
	struct ftdi_device_list *curdev = NULL;
	char manufacturer[128] = {0};
	char description[128] = {0};

	int total = ftdi_usb_find_all(ftdic, &devlist, FT_VID, FT_PID);
	mylog("Number of FTDI devices found: %d\n", total);

    int ret, i = 1;
	curdev = devlist;
    for (curdev = devlist; curdev != NULL; i++) {
		mylog("Checking device: %d\n", i);
		if ((ret = ftdi_usb_get_strings(ftdic, curdev->dev, manufacturer, sizeof(manufacturer), description, sizeof(description), NULL, 0)) < 0){
			mylog("ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(ftdic));
			break;
		}
		mylog("Manufacturer: %s, Description: %s\n\n", manufacturer, description);
		curdev = curdev->next;
	}
	ftdi_list_free(&devlist);
	return total;
}

struct usb_device *ftdi_getDeviceHandle (struct ftdi_context *ftdic, int deviceNumber)
{
	static struct ftdi_device_list *devlist, *curdev;
	struct usb_device *dev = NULL;

	int total = ftdi_usb_find_all(ftdic, &devlist, FT_VID, FT_PID);
	if (total >= deviceNumber){
		curdev = devlist;
    	while(deviceNumber--){
    		dev = curdev->dev;
			curdev = curdev->next;
		}
	}else{
		mylog("libmylcd: FTDI: invalid device number. requested:%i total:%i\n", deviceNumber, total);
	}
	ftdi_list_free(&devlist);
	return dev;
}

static int ftdi_OpenDriver (TPORTDRIVER *pd, int BaudRate)
{
	if (pd == NULL) return 0;

	const int deviceNumber = 1;

	ftdi_init(&ftdic);
	struct usb_device *dev = ftdi_getDeviceHandle(&ftdic, deviceNumber);
	if (dev == NULL){
		mylog("libmylcd: FTDI: device %i not available\n", deviceNumber);
		ftdi_deinit(&ftdic);
		return 0;
	}

	int ret = ftdi_usb_open_dev(&ftdic, dev);
	if (ret != FT_OK){
		mylog("libmylcd: FTDI: failed to open device, return code: %i\n", ret);
		pd->status = LDRV_CLOSED;
		ftdi_deinit(&ftdic);
		return 0;
	}else{
		writeBuffer = (ubyte *)l_calloc(sizeof(ubyte), BUFFERLENGTH);
		if (writeBuffer == NULL){
			ftdi_usb_close(&ftdic);
			ftdi_deinit(&ftdic);
			return 0;
		}

		resetDevice();
		isInitiated = 1;
		setBitMode(0xFF, FT_BB_ENABLED);
		setLatency(1);
		setBaudRate(BaudRate);
		ftdi_write_data_set_chunksize(&ftdic, BUFFERLENGTH);
		pd->write8(0,0);
		pd->status = LDRV_READY;
		return 1;
	}
}

static int ftdi_CloseDriver (TPORTDRIVER *pd)
{
	if (pd && isInitiated){
		if (pd->status != LDRV_CLOSED){
			pd->write8(0,0);
			pd->flush(pd);
			pd->status = LDRV_CLOSED;

			//ftdi_disable_bitbang(&ftdic);	// is this required?
    		ftdi_usb_close(&ftdic);
    		ftdi_deinit(&ftdic);

			if (writeBuffer)
				l_free(writeBuffer);
			writeBuffer = NULL;
			bytesToWrite = 0;
			isInitiated = 0;
			return 1;
		}
	}
	return 0;
}

static ubyte ftdi_ReadPort (const int port)
{
	if (isInitiated){
		ubyte buffer = 0;
		int ret = 0;
		if (bitBangState == FT_BB_ENABLED)
			ret = ftdi_read_pins(&ftdic, &buffer);		// use when bitbang mode is enabled
		else
			ret = ftdi_read_data(&ftdic, &buffer, 1);	// use with bitbang mode disabled
		if (ret < 0)
			mylog("libmylcd: FTDI: read error: %i\n", (int)ret);

		return buffer;
	}
	return 0;
}

static int ftdi_Flush (TPORTDRIVER *pd)
{
	if (bytesToWrite && isInitiated){
		int lpBytesWritten = ftdi_write_data(&ftdic, writeBuffer, bytesToWrite);
		//printf("bytesToWrite %i %i\n", bytesToWrite, lpBytesWritten);
		if (bytesToWrite != lpBytesWritten){
			mylog("libmylcd: FTDI: write error: %i %i\n",(int)bytesToWrite, (int)lpBytesWritten);
			bytesToWrite = 0;
		}else{
			bytesToWrite = 0;
			return 1;
		}
	}
	return 0;
}

static int ftdi_WritePort8 (const int port, const ubyte data)
{
	if (isInitiated){
		writeBuffer[bytesToWrite] = data;
		if (++bytesToWrite >= BUFFERLENGTH)
			return ftdi_Flush(NULL);
	}
	return 0;

}

static int ftdi_WritePort16 (const int port, const unsigned short data)
{
	ftdi_WritePort8(port,(ubyte)((data>>8)&0xFF));
	ftdi_WritePort8(port,(ubyte)(data&0xFF));
	return 0;
}

static int ftdi_WritePort32 (const int port, const unsigned int data)
{
	ftdi_WritePort8(port,(ubyte)((data>>24)&0xFF));
	ftdi_WritePort8(port,(ubyte)((data>>16)&0xFF));
	ftdi_WritePort8(port,(ubyte)((data>>8)&0xFF));
	ftdi_WritePort8(port,(ubyte)(data&0xFF));
	return 0;
}

static int ftdi_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	if (isInitiated){
		while(tbytes--){
			writeBuffer[bytesToWrite] = *(ubyte*)buffer++;
			if (++bytesToWrite >= BUFFERLENGTH)
				return ftdi_Flush(NULL);
		}
	}
	return 0;
}

static int ftdi_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	intptr_t *opt = drv->pd->opt;

	switch(option){
		case lOPT_FTDI_BAUDRATE :
			if (value){
				if (setBaudRate(*value)){
					opt[lOPT_FTDI_BAUDRATE] = *value;
					return 1;
				}
			}
			return 0;
		case lOPT_FTDI_LATENCY :
			if (*value&0xFE){  						// range = 2ms to 255ms
				if (setLatency(*value)){
					opt[lOPT_FTDI_LATENCY] = *value;
					return 1;
				}
			}
			return 0;
		case lOPT_FTDI_IODIR :
			if (setBitMode(*value&0xFF, opt[lOPT_FTDI_BITBANG])){
				opt[lOPT_FTDI_IODIR] = *value;
				return 1;
			}
			return 0;
		case lOPT_FTDI_BITBANG :
			if (setBitMode(opt[lOPT_FTDI_IODIR], *value&0xFF)){
				opt[lOPT_FTDI_BITBANG] = *value;
				return 1;
			}
			return 0;
		default :
			mylog("libmylcd: FTDI: setOption(): invalid option: %i:%i\n",option, value);
			return 0;
	};
	return 0;
}

static int ftdi_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv || !value)	return 0;
	if (option >= 6) return 0;

	int *opt = (int *)drv->pd->opt;
	*value = opt[option];
	return 1;
}

int setBaudRate (int baud)
{
	if (!baud) baud = DEFAULT_BAUD;
	int ret = 	ftdi_set_baudrate(&ftdic, baud);
	if (ret != FT_OK){
		mylog("libmylcd: FTDI: unable to set requested baud rate:%i %i\n",baud, ret);
		return 0;
	}else{
		//mylog("FTDI: baud rate set to %i\n",baud);
		return 1;
	}
}

int setLatency (ubyte ucLatency)
{
	int ret = ftdi_set_latency_timer(&ftdic, ucLatency);
	if (ret != FT_OK){
		mylog("libmylcd: FTDI: unable to set requested latency rate:%i %i\n",(int)ucLatency, ret);
		return 0;
	}else{
		return 1;
	}
}

int setBitMode (ubyte mask, ubyte mode)
{
	bitBangState = mode;
	int ret = ftdi_enable_bitbang(&ftdic, mask);
	if (ret != FT_OK){
		mylog("libmylcd: FTDI: unable to set bit bang mode:%i %i %i\n",(int)mask, (int)mode, ret);
		return 0;
	}else
		return 1;
}

int resetDevice ()
{
	int ret = ftdi_usb_reset(&ftdic);
	if (ret != FT_OK){
		mylog("libmylcd: FTDI: unable to reset FT device:%i\n", ret);
		return 0;
	}else
		return 1;
}

#else

int initFTDI(void *rd){return 1;}
void closeFTDI(void *rd){return;}

#endif


