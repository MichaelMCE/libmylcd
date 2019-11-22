
// libusbd480 - http://mylcd.sourceforge.net/

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

#if (__BUILD_USBD480LIBUSB__ || __BUILD_USBD480__)

#include "../../memory.h"
#include "libusbd480.h"

//#define l_memcpy memcpy

#define LIBUSB_VENDERID		USBD480_VID
#define LIBUSB_PRODUCTID	USBD480_PID
#define LIBUSB_FW			0x400		/* minimum required firmware revision*/


//#define __DEBUG__

#ifdef __DEBUG__
#define umylog printf
#else
#define umylog(x, ...) 
#endif



/*

Touch calibration point 1: (3651,  796)
Touch calibration point 2: (2087,  850)
Touch calibration point 3: ( 519,  945)
Touch calibration point 4: (3562, 2039)
Touch calibration point 5: (2115, 2083)
Touch calibration point 6: ( 461, 2233)
Touch calibration point 7: (3578, 3273)
Touch calibration point 8: (2049, 3468)
Touch calibration point 9: ( 491, 3436)

usb_control_msg() 0x40 0x85/133 0x0/0 0x0/0
usb_control_msg() 0x40 0x85/133 0x1/1 0x0/0

usb_control_msg() 0x40 0xB6/182 0x4433/17459 0x0/0
ret:3 ( 0 160 15)

usb_control_msg() 0x40 0xB5/181 0x4433/17459 0x4/4
ret:3 ( 0 160 15)

usb_control_msg() 0x40 0xB1/177 0xA000/40960 0xF/15
ret:64 ( 67 14 28 3 39 8 82 3 7 2 177 3 234 13 247 7 67 8 35 8 205 1 185 8 250
13 201 12 1 8 140 13 235 1 108 13 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
 0 0 0 0)

*/



static void calibration_setNominal (TUSBD480TCAL *cal)
{
/*	Screen coordinates used for collecting calibration points

	Reference point 1: ( 39, 39)
	Reference point 2: (239, 39)
	Reference point 3: (439, 39)
	Reference point 4: ( 39, 135)
	Reference point 5: (239, 135)
	Reference point 6: (439, 135)
	Reference point 7: ( 39, 231)
	Reference point 8: (239, 231)
	Reference point 9: (439, 231)
*/

	// ideal sample coordinates for calculating the coefficients
	
	/*cal->refPoint[0][0] = 3648; cal->refPoint[0][1] = 800;
	cal->refPoint[1][0] = 2048; cal->refPoint[1][1] = 800;
	cal->refPoint[2][0] =  448; cal->refPoint[2][1] = 800;
	cal->refPoint[3][0] = 3648; cal->refPoint[3][1] = 2048;
	cal->refPoint[4][0] = 2048; cal->refPoint[4][1] = 2048;
	cal->refPoint[5][0] =  448; cal->refPoint[5][1] = 2048;
	cal->refPoint[6][0] = 3648; cal->refPoint[6][1] = 3296;
	cal->refPoint[7][0] = 2048; cal->refPoint[7][1] = 3296;
	cal->refPoint[8][0] =  448; cal->refPoint[8][1] = 3296;*/


	cal->refPoint[0].x = 3648; cal->refPoint[0].y = 800;
	cal->refPoint[1].x = 2048; cal->refPoint[1].y = 800;
	cal->refPoint[2].x =  448; cal->refPoint[2].y = 800;
	cal->refPoint[3].x = 3648; cal->refPoint[3].y = 2048;
	cal->refPoint[4].x = 2048; cal->refPoint[4].y = 2048;
	cal->refPoint[5].x =  448; cal->refPoint[5].y = 2048;
	cal->refPoint[6].x = 3648; cal->refPoint[6].y = 3296;
	cal->refPoint[7].x = 2048; cal->refPoint[7].y = 3296;
	cal->refPoint[8].x =  448; cal->refPoint[8].y = 3296;

	/*
	// direct pixel coordinates for direct conversion to screen coordinates
	//
	// One way to handle scaling to screen coordinates could be to alternatively use the reference
	// points below instead of ideal coordinates to directly get screen coordinates.
	// But it looks like using the ideal sample coordinates and then doing a separate
	// conversion to screen coordinates using To_Screen_Coordinates() is more accurate.
	cal->refPoint[0][0] =  39; cal->refPoint[0][1] = 39;
	cal->refPoint[1][0] = 239; cal->refPoint[1][1] = 39;
	cal->refPoint[2][0] = 439; cal->refPoint[2][1] = 39;
	cal->refPoint[3][0] =  39; cal->refPoint[3][1] = 135;
	cal->refPoint[4][0] = 239; cal->refPoint[4][1] = 135;
	cal->refPoint[5][0] = 439; cal->refPoint[5][1] = 135;
	cal->refPoint[6][0] =  39; cal->refPoint[6][1] = 231;
	cal->refPoint[7][0] = 239; cal->refPoint[7][1] = 231;
	cal->refPoint[8][0] = 439; cal->refPoint[8][1] = 231;
	*/



	// data sampled from touchscreen - these should have been saved to the device
	// with the calibration utility and then loaded from the device (USBD480_READPARAMSTORE request)
#if 1
	cal->samPoint[0].x = 3628; cal->samPoint[0].y = 853;
	cal->samPoint[1].x = 2061; cal->samPoint[1].y = 870;
	cal->samPoint[2].x =  414; cal->samPoint[2].y = 890;
	cal->samPoint[3].x = 3637; cal->samPoint[3].y = 2088;
	cal->samPoint[4].x = 2086; cal->samPoint[4].y = 2103;
	cal->samPoint[5].x =  487; cal->samPoint[5].y = 2112;
	cal->samPoint[6].x = 3610; cal->samPoint[6].y = 3360;
	cal->samPoint[7].x = 2017; cal->samPoint[7].y = 3351;
	cal->samPoint[8].x =  471; cal->samPoint[8].y = 3308;
#endif
}

int libusbd480_WriteCalParamStore (TUSBD480 *di, TUSBD480TCAL *cal)
{
	usb_control_msg(di->usb_handle, 0x40, 0x85, 0x0, 0, NULL, 0, 500);
	usb_control_msg(di->usb_handle, 0x40, 0x85, 0x1, 0, NULL, 0, 500);
	
	unsigned int addr = 0xA000;
	int index = 15;
	char buffer[16];
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = addr % 256;
	buffer[1] = addr / 256;
	buffer[2] = 15;
	
	usb_control_msg(di->usb_handle, 0x40, 0xB6, 0x4433, 0, buffer, 3, 500);
	usb_control_msg(di->usb_handle, 0x40, 0xB5, 0x4433, 4, buffer, 3, 500);
	
	unsigned char data[64];
	memset(data, 0, 64);
			
	int j = 0;
	for (int i = 0; i < 9; i++){
		data[j++] = cal->samPoint[i].x % 256;
		data[j++] = cal->samPoint[i].x / 256;
				
		data[j++] = cal->samPoint[i].y % 256;
		data[j++] = cal->samPoint[i].y / 256;
				
		//printf("%i: (%i,%i) %i %i, %i %i\n", i, cal->samPoint[i].x, cal->samPoint[i].y, data[j-4], data[j-3],data[j-2], data[j-1]);
	}
	
	int ret = usb_control_msg(di->usb_handle, 0x40, 0xB1, addr, index, (char*)data, 64, 500);
	return ret == 64;
}

void libusbd480_ReadCalParamStore (TUSBD480 *di, TUSBD480TCAL *cal)
{
	const unsigned int paramIndex = 0;
	const unsigned int paramSize = 36;
	char databuf[64];
	memset(databuf, 0, sizeof(databuf));
		
	usb_control_msg(di->usb_handle, 0xC0, USBD480_READ_PARAM_STORE, paramIndex&0xffff, paramSize&0xff, databuf, paramSize, 500);
#if 0
	for(int i = 0; i < paramSize; i++)
		printf("%x, ", (unsigned char)databuf[i]);
	printf("\n");
#endif
	for (int i = 0; i < TOUCHCALPOINTS; i++){
		int x = (unsigned int)((unsigned char)databuf[i*4]   | ((unsigned char)databuf[i*4+1]<<8));
		int y = (unsigned int)((unsigned char)databuf[i*4+2] | ((unsigned char)databuf[i*4+3]<<8));
		cal->samPoint[i].x = x;
		cal->samPoint[i].y = y;
		//printf("Calibration point %d: %d,%d\n", i+1, x, y);
	}
}


// do calibration for point (Px, Py) using the calculated coefficients
static void calibration_calibratePoint (TUSBD480TCAL *cal, int *Px, int *Py)
{
	*Px = (int)(cal->KX1 * (*Px)+cal->KX2 * (*Py)+cal->KX3 + 0.5);
	*Py = (int)(cal->KY1 * (*Px)+cal->KY2 * (*Py)+cal->KY3 + 0.5);
}

// calculate the coefficients for calibration algorithm: KX1, KX2, KX3, KY1, KY2, KY3
static int calibration_calcCoefficients (TUSBD480TCAL *cal)
{
	const int Points = TOUCHCALPOINTS;
	double a[3],b[3],c[3],d[3];

	if (Points < 3){
		return 0;

	}else{
		if (Points == 3){
			for(int i = 0; i < Points; i++){
				a[i] = (double)(cal->samPoint[i].x);
				b[i] = (double)(cal->samPoint[i].y);
				c[i] = (double)(cal->refPoint[i].x);
				d[i] = (double)(cal->refPoint[i].y);
			}
		}else if (Points > 3){
			for (int i = 0; i < 3; i++){
				a[i] = 0;
				b[i] = 0;
				c[i] = 0;
				d[i] = 0;
			}

			for (int i = 0; i < Points; i++){
				a[2] = a[2] + (double)(cal->samPoint[i].x);
				b[2] = b[2] + (double)(cal->samPoint[i].y);
				c[2] = c[2] + (double)(cal->refPoint[i].x);
				d[2] = d[2] + (double)(cal->refPoint[i].y);
				a[0] = a[0] + (double)(cal->samPoint[i].x)*(double)(cal->samPoint[i].x);
				a[1] = a[1] + (double)(cal->samPoint[i].x)*(double)(cal->samPoint[i].y);
				b[0] = a[1];
				b[1] = b[1] + (double)(cal->samPoint[i].y)*(double)(cal->samPoint[i].y);
				c[0] = c[0] + (double)(cal->samPoint[i].x)*(double)(cal->refPoint[i].x);
				c[1] = c[1] + (double)(cal->samPoint[i].y)*(double)(cal->refPoint[i].x);
				d[0] = d[0] + (double)(cal->samPoint[i].x)*(double)(cal->refPoint[i].y);
				d[1] = d[1] + (double)(cal->samPoint[i].y)*(double)(cal->refPoint[i].y);
			}

			a[0] = a[0] / a[2];
			a[1] = a[1] / b[2];
			b[0] = b[0] / a[2];
			b[1] = b[1] / b[2];
			c[0] = c[0] / a[2];
			c[1] = c[1] / b[2];
			d[0] = d[0] / a[2];
			d[1] = d[1] / b[2];
			a[2] = a[2] / Points;
			b[2] = b[2] / Points;
			c[2] = c[2] / Points;
			d[2] = d[2] / Points;
		}

		const double k = (a[0]-a[2]) * (b[1]-b[2]) - (a[1]-a[2]) * (b[0] - b[2]);
		cal->KX1 = ((c[0]-c[2]) * (b[1]-b[2]) - (c[1]-c[2]) * (b[0] - b[2])) / k;
		cal->KX2 = ((c[1]-c[2]) * (a[0]-a[2]) - (c[0]-c[2]) * (a[1] - a[2])) / k;
		cal->KX3 = (b[0] * (a[2] * c[1]-a[1] * c[2])+b[1] * (a[0] * c[2] - a[2] * c[0])+b[2] * (a[1] * c[0]-a[0] * c[1])) / k;
		cal->KY1 = ((d[0]-d[2]) * (b[1]-b[2]) - (d[1]-d[2]) * (b[0] - b[2])) / k;
		cal->KY2 = ((d[1]-d[2]) * (a[0]-a[2]) - (d[0]-d[2]) * (a[1] - a[2])) / k;
		cal->KY3 = (b[0] * (a[2] * d[1]-a[1] * d[2])+b[1] * (a[0] * d[2] - a[2] * d[0])+b[2] * (a[1] * d[0]-a[0] * d[1])) / k;

		return Points;
	}
}

static void calibration_convertPoint (TUSBD480TCAL *cal, int *x, int *y)
{
/*
	***************************************************************
	* (3968, 280)                                                 *
	*                                                             *
	*                                                             *
	*                                                             *
	*                         (2048, 2048)                        *
	*                              +                              *
	*                                                             *
	*                                                             *
	*                                                             *
	*                                                             *
	*                                                 (128, 3816) *
	***************************************************************
*/

	double x_scale = 8;
	double y_scale = 13;

	int xt = (4096 - *x) - 128;
	int yt = *y - 280;
	int x_scr = (int)(((double)xt / x_scale)+0.5);
	int y_scr = (int)(((double)yt / y_scale)+0.5);

	*x = x_scr-1;
	*y = y_scr-1;
}

static uint64_t getTicks ()
{
#ifdef __WIN32__

	static float resolution;
	static uint64_t tStart;
	static uint64_t freq;
	
	
	if (!tStart){
		QueryPerformanceCounter((LARGE_INTEGER*)&tStart);
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		resolution = 1.0 / (float)freq;
	}
	uint64_t t1 = tStart;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((float)((uint64_t)(t1 - tStart) * resolution) * 1000.0);
	
#else
	return clock();
#endif
}

int libusbd480_Init ()
{
	//printf("libusbd480_Init\n");
	
	static int initOnce = 0;
	
	if (!initOnce){
		initOnce = 1;
		usb_init();
	}
    usb_find_busses();
    usb_find_devices();
	return 1;	
}

void cleanQueue (TTOUCH *touch)
{
	for (int i = 0; i <= TBINPUTLENGTH; i++){
		touch->tin[i].mark = 2;
		touch->tin[i].loc.x = -1;
		touch->tin[i].loc.y = -1;
		touch->tin[i].loc.pen = -1;
		touch->tin[i].loc.pressure = -1;
	}
	touch->dragState = 0;
	touch->dragStartCt = 0;
    touch->last.x = -2;
    touch->last.y = -2;
    touch->last.pen = -1;
	touch->last.time = getTicks();
	touch->pos.time = touch->last.time;
}

void libusbd480_CleanQueue (TUSBD480 *di)
{
	cleanQueue(&di->touch);
}

void initTouch (TUSBD480 *di, TTOUCH *touch)
{
	cleanQueue(touch);
	touch->count = 0;
#if 0
	touch->cal.left = 210;		// your panel may differ 
	touch->cal.right = 210;
	touch->cal.top = 255;
	touch->cal.bottom = 300;
#else
	touch->cal.left = 1;
	touch->cal.right = 1;
	touch->cal.top = 1;
	touch->cal.bottom = 1;
#endif

	touch->cal.hrange = (4095-touch->cal.left)-touch->cal.right;
	touch->cal.vrange = (4095-touch->cal.top)-touch->cal.bottom;
	touch->cal.hfactor = di->Width/touch->cal.hrange;
	touch->cal.vfactor = di->Height/touch->cal.vrange;
}

int libusbd480_GetDisplayTotal ()
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    int ct = 0;
    
    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next){
        for (dev = usb_bus->devices; dev; dev = dev->next){
            if ((dev->descriptor.idVendor == LIBUSB_VENDERID) && (dev->descriptor.idProduct == LIBUSB_PRODUCTID)){
				if (dev->config->interface->altsetting->bInterfaceNumber == 0){
					ct++;
                }
			}
        }
    }
    return ct;
}

struct usb_device *find_usbd480 (int index)
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    
    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next) {
        for (dev = usb_bus->devices; dev; dev = dev->next) {
            if ((dev->descriptor.idVendor == LIBUSB_VENDERID) && (dev->descriptor.idProduct == LIBUSB_PRODUCTID)){
            	//printf("dev->config->interface->altsetting->bInterfaceNumber %i\n", dev->config->interface->altsetting->bInterfaceNumber);
				if (dev->config->interface->altsetting->bInterfaceNumber == 0){
            		if (index-- == 0){
						umylog("libusbd480: device found:  0x%X:0x%X '%s'\n", dev->descriptor.idVendor, dev->descriptor.idProduct, dev->filename);
                		return dev;
                	}
                }
			}
        }
    }
    return NULL;
}
/*
int libusbd480_SaveConfig (TUSBD480 *di)
{
	int ret = 0;
	if (di){
		if (di->usb_handle)
			ret = usb_control_msg(di->usb_handle, 0x40, USBD480_SAVE_CONFIG, 0x8877, 0, NULL, 0, 500);
	}
	return ret;
}

int libusbd480_GetSavedConfigValue (TUSBD480 *di, int cfg, int value)
{
	int ret = 0;
	if (di){
		if (di->usb_handle){
			char val = value&0xFF;
			ret = usb_control_msg(di->usb_handle, 0x40, USBD480_GET_SAVED_CONFIG_VALUE, cfg, 0, &val, 1, 500);
		}
	}
	return ret;
}*/

int libusbd480_SetConfigValue (TUSBD480 *di, int cfg, int value)
{
	int ret = 0;
	if (di){
		if (di->usb_handle){
			char val = value&0xFF;
			ret = usb_control_msg(di->usb_handle, 0x40, USBD480_SET_CONFIG_VALUE, cfg, 0, &val, 1, 500);
		}
	}
	return ret;
}

int libusbd480_GetConfigValue (TUSBD480 *di, int cfg, int *value)
{
	int ret = 0;
	if (di && value){
		if (di->usb_handle){
			char val = *value&0xFF;
			ret = usb_control_msg(di->usb_handle, 0xC0, USBD480_GET_CONFIG_VALUE, cfg, 0, &val, 1, 500);
			*value = val;
		}
	}
	return ret;
}

int setConfigDefaults (TUSBD480 *di)
{
	int ret = libusbd480_SetConfigValue(di, CFG_TOUCH_MODE, 0);
	libusbd480_SetConfigValue(di, CFG_TOUCH_DEBOUNCE_VALUE, 20);
	libusbd480_SetConfigValue(di, CFG_TOUCH_SKIP_SAMPLES, 20);
	libusbd480_SetConfigValue(di, CFG_TOUCH_PRESSURE_LIMIT_LO, 50);
	libusbd480_SetConfigValue(di, CFG_TOUCH_PRESSURE_LIMIT_HI, 120);

	//libusbd480_SetConfigValue(di, CFG_USB_ENUMERATION_MODE, 0);
	
	//char val = 200;
	//usb_control_msg(di->usb_handle, 0x40, USBD480_EEPROM_PREPARE_WRITE, 0x6655, 0, 0, 0, 500);
    //usb_control_msg(di->usb_handle, 0x40, USBD480_SAVE_CONFIG, CFG_BACKLIGHT_BRIGHTNESS, 0, &val, 1, 500);
	
	libusbd480_SetBrightness (di, 255);

#if 0
	// read back values
	int i;
	int value = 0;
	for (i = 0; i < 23; i++){
		libusbd480_GetConfigValue(di, i, &value);
		printf("config: %d value: %i\n", i, (unsigned char)value);
	}
#endif
	return ret;
}

int libusbd480_GetDeviceDetails (TUSBD480 *di, char *name, uint32_t *width, uint32_t *height, uint32_t *version, char *serial)
{
	char data[256];
	memset(data, 0, sizeof(data));
	const unsigned char rsize = 64;
	
	int result = usb_control_msg(di->usb_handle, 0xC0, USBD480_GET_DEVICE_DETAILS, 0, 0, data, rsize, 1000);
	if (result > 0){
		if (name) memcpy(name, data, DEVICE_NAMELENGTH-1);
		if (serial) memcpy(serial, &data[26], DEVICE_SERIALLENGTH-1);
		if (width) *width = (unsigned char)data[20] | ((unsigned char)data[21] << 8);
		if (height) *height = (unsigned char)data[22] | ((unsigned char)data[23] << 8);
		if (version) *version = (unsigned char)data[24] | ((unsigned char)data[25] << 8);
	}
	return result;
}

int libusbd480_OpenDisplay (TUSBD480 *di, uint32_t index)
{
	
	//printf("libusbd480_OpenDisplay\n");
	
	if (di == NULL) return -1;
		
	memset(di, 0, sizeof(TUSBD480));
	
	libusbd480_Init();
	umylog("libusbd480_OpenDisplay: displays found: %i\n", libusbd480_GetDisplayTotal());

	di->usbdev = find_usbd480(index);
	if (di->usbdev){
		di->usb_handle = usb_open(di->usbdev); 
		if (di->usb_handle == NULL){
			umylog("libusbd480_OpenDisplay(): error: usb_open\n");
			return 0;
		}

		if (usb_set_configuration(di->usb_handle, 1) < 0){
			umylog("libusbd480_OpenDisplay(): error: setting config 1 failed\n");
			umylog("%s\n", usb_strerror());
			usb_close(di->usb_handle);
			return 0;
		}
		
		if (usb_claim_interface(di->usb_handle, 0) < 0){
			umylog("libusbd480_OpenDisplay(): error: claiming interface 0 failed\n");
			umylog("%s\n", usb_strerror());
			usb_close(di->usb_handle);
			return 0;
		}

		if (libusbd480_GetDeviceDetails(di, di->Name, &di->Width, &di->Height, &di->DeviceVersion, di->Serial)){
			umylog("libusbd480: name:'%s' width:%i height:%i version:0x%X serial:'%s'\n", di->Name, di->Width, di->Height, di->DeviceVersion, di->Serial);
			if (di->DeviceVersion < LIBUSB_FW){
				printf("libusbd480: firmware version 0x%X < 0x%X\n", di->DeviceVersion, LIBUSB_FW);
				printf("libusbd480 requires firmware 0x%X or later to operate effectively\n", LIBUSB_FW);
			}
			
			di->tmpBuffer = (char*)malloc((di->Width*di->Height*sizeof(short))+1024); //framesize + headerspace + alignment
			if (di->tmpBuffer != NULL){
    			di->PixelFormat = BPP_16BPP_RGB565;
    			di->currentPage = 0;
				initTouch(di, &di->touch);
				//setConfigDefaults(di);
				
				if (di->DeviceVersion >= 0x800){
					calibration_setNominal(&di->cal);
					//libusbd480_WriteCalParamStore(di, &di->cal);
					//libusbd480_ReadCalParamStore(di, &di->cal);
					calibration_calcCoefficients(&di->cal);
				}
					
				libusbd480_SetFrameAddress(di, 0);
				libusbd480_SetWriteAddress(di, 0);
				if (di->DeviceVersion >= 0x700){
					libusbd480_EnableStreamDecoder(di);
					libusbd480_SetBaseAddress(di, 0);
					libusbd480_SetLineLength(di, di->Width);
					
				}
				return 1;
			}
		}else{
			umylog("libusbd480_GetDeviceDetails() failed\n");
		}
		libusbd480_CloseDisplay(di);
	}
	return 0;
}
                    
int libusbd480_CloseDisplay (TUSBD480 *di)
{
	//printf("libusbd480_CloseDisplay\n");
	
	if (di){
		if (di->usb_handle){
			usb_release_interface(di->usb_handle, 0);
			usb_close(di->usb_handle);
			di->usb_handle = NULL;
			if (di->tmpBuffer != NULL)
				free(di->tmpBuffer);
			di->tmpBuffer = NULL;
			return 1;
		}
	}
	return 0;
}

int libusbd480_SetTouchMode (TUSBD480 *di, unsigned int mode)
{
	return usb_control_msg(di->usb_handle, 0x40, USBD480_TOUCH_MODE, mode, 0, NULL, 0, 500);
}

int libusbd480_DisableStreamDecoder (TUSBD480 *di)
{
	return usb_control_msg(di->usb_handle, 0x40, USBD480_SET_STREAM_DECODER, 0x00, 0, NULL, 0, 500);	
}

int libusbd480_EnableStreamDecoder (TUSBD480 *di)
{
	return usb_control_msg(di->usb_handle, 0x40, USBD480_SET_STREAM_DECODER, 0x06, 0, NULL, 0, 500);
}

int libusbd480_SetLineLength (TUSBD480 *di, int length)
{
	char data[4];
	data[0] = 0x43; // wrap length
	data[1] = 0x5B;
	data[2] = (--length)&0xFF;
	data[3] = (length>>8)&0xFF;
	const int dsize = 4;
	int result;

	if ((result = libusbd480_WriteData(di, data, dsize)) != dsize){
		umylog("bulk_write error %i\n", result);
	}
	return result;
}

int libusbd480_SetBaseAddress (TUSBD480 *di, unsigned int address)
{
	char data[6];
	data[0] = 0x42; // base address
	data[1] = 0x5B;
	data[2] = address&0xFF;
	data[3] = (address>>8)&0xFF;
	data[4] = (address>>16)&0xFF;
	data[5] = (address>>24)&0xFF;
	const int dsize = 6;
	int result;
	
	if ((result = libusbd480_WriteData(di, data, dsize)) != dsize){
		umylog("bulk_write error %i\n", result);
	}
	return result;
}

int libusbd480_SetFrameAddress (TUSBD480 *di, unsigned int addr)
{
	return usb_control_msg(di->usb_handle, 0x40, USBD480_SET_FRAME_START_ADDRESS, addr, (addr>>16)&0xFFFF, NULL, 0, 1000);
}

int libusbd480_SetWriteAddress (TUSBD480 *di, unsigned int addr)
{
	return usb_control_msg(di->usb_handle, 0x40, USBD480_SET_ADDRESS, addr, (addr>>16)&0xFFFF, NULL, 0, 1000);
}

int libusbd480_WriteData (TUSBD480 *di, void *data, size_t size)
{
	return usb_bulk_write(di->usb_handle, 0x02, (char*)data, size, 1000);
}

static int libusbd480_DrawScreenX400 (TUSBD480 *di, uint8_t *fb, size_t size)
{
	if (di->usb_handle){
		unsigned int address = 0;
#if 0
		di->currentPage ^= 0x01;
		if (di->currentPage)
			address = 0;
		else
			address = 0 + (size>>1);
#endif
		libusbd480_SetWriteAddress(di, address);
		if ((size == (size_t)libusbd480_WriteData(di, fb, size))){
			return libusbd480_SetFrameAddress(di, address) == 0;
		}else{
			umylog("usbd480_DrawScreen(): usb_bulk_write() error\n");
			return 0;
		}
	}
	return 0;
}

int libusbd480_DrawScreenAreaX700 (TUSBD480 *di, uint8_t *fb, size_t size, TLPOINTEX *region)
{
	int ret = 0;
	char *data = di->tmpBuffer;
	const size_t writelength = (region->x2 - region->x1);
	size_t sizeTotal = 0;

	size_t pitch = di->Width<<1;
	fb += ((region->y1-1)*pitch) + (region->x1<<1);
	
	for (int y = region->y1; y <= region->y2; y++){
		size_t writeaddress = (y*di->Width) + region->x1;
		
		data[0] = 0x41; // pixel write
		data[1] = 0x5B;
		data[2] = writeaddress&0xFF;
		data[3] = (writeaddress>>8)&0xFF;
		data[4] = (writeaddress>>16)&0xFF;
		data[5] = (writeaddress>>24)&0xFF;
		data[6] = writelength&0xFF;
		data[7] = (writelength>>8)&0xFF;
		data[8] = (writelength>>16)&0xFF;
		data[9] = (writelength>>24)&0xFF;
		
		fb += pitch;
		size_t dsize = (writelength+1)<<1;
		l_memcpy(&data[10], fb, dsize);
		dsize += 10;
		data += dsize;
		sizeTotal += dsize;
	}

	if ((sizeTotal != (size_t)libusbd480_WriteData(di, di->tmpBuffer, sizeTotal))){
		umylog("libusbd480_DrawScreenAreaX700(): usb_bulk_write() error %i\n", (int)sizeTotal);
		ret = sizeTotal;
	}
	return ret;
}

static inline int libusbd480_DrawScreenX700 (TUSBD480 *di, uint8_t *fb, uint32_t size)
{
	if (di->usb_handle){
		char *data = di->tmpBuffer;
		uint32_t writeaddress = 0;
#if 0
		di->currentPage ^= 0x01;
		if (di->currentPage)
			writeaddress = 0;
		else
			writeaddress = size>>1;
#endif
		const uint32_t writelength = (size>>1)-1;
				
		data[0] = 0x41; // pixel write
		data[1] = 0x5B;
		data[2] = writeaddress&0xFF;
		data[3] = (writeaddress>>8)&0xFF;
		data[4] = (writeaddress>>16)&0xFF;
		data[5] = (writeaddress>>24)&0xFF;
		data[6] = writelength&0xFF;
		data[7] = (writelength>>8)&0xFF;
		data[8] = (writelength>>16)&0xFF;
		data[9] = (writelength>>24)&0xFF;

		uint32_t dsize = (writelength+1)<<1;
		l_memcpy(&data[10], fb, dsize);
		dsize += 10;

		if ((dsize == (size_t)libusbd480_WriteData(di, data, dsize))){
			return 1;
		}else{
			umylog("usbd480_DrawScreen(): usb_bulk_write() error %i\n", dsize);
		}
	}
	return 0;
}

int libusbd480_DrawScreen (TUSBD480 *di, uint8_t *fb, size_t size)
{
	//printf("di->DeviceVersion %i\n", di->DeviceVersion);
	if (di->DeviceVersion >= 0x700)
		return libusbd480_DrawScreenX700(di, fb, size);
	else
		return libusbd480_DrawScreenX400(di, fb, size);
}

int libusbd480_DrawScreenIL (TUSBD480 *di, uint8_t *fb, size_t size)
{
	static int first = 0;

	if (di){
		if (di->usb_handle){
			unsigned int i;
			const int pitch = di->Width*2;
			char data[pitch*2];
			libusbd480_SetFrameAddress(di, 0);

			first ^= 0x01;
			for (i = first; i < di->Height; i += 2){
				l_memcpy(data, fb+(i*pitch), pitch+64);
				libusbd480_SetWriteAddress(di, i*di->Width);
				libusbd480_WriteData(di, data, pitch+64);
			}
			return 1;
		}
	}
	return 0;
}

int libusbd480_SetBrightness (TUSBD480 *di, uint8_t level)
{	
	return libusbd480_SetConfigValue(di, CFG_BACKLIGHT_BRIGHTNESS, level);
}
/*
// convert from touch panel location to display coordinates
int filter0 (TUSBD480 *di, TTOUCH *touch, TTOUCHCOORD *pos)
{
	if ((pos->y > touch->cal.top) && (pos->y < 4095-touch->cal.bottom) && (pos->x > touch->cal.left) && (pos->x < 4095-touch->cal.right)){
		pos->x = di->Width-(touch->cal.hfactor*(pos->x - touch->cal.right));
		pos->y = touch->cal.vfactor*(pos->y - touch->cal.top);
		touch->pos.x = pos->x;
		touch->pos.y = pos->y;
		return 1;
	}else{
		return 0;
	}
}
*/

// detect and remove erroneous coordinates plus apply a little smoothing
static int filter1 (TUSBD480 *di, TTOUCH *touch)
{
	TTOUCHCOORD *pos = &touch->pos;
	l_memcpy(&touch->last, pos, sizeof(TTOUCHCOORD));

	// check current with next (pos)
	int dx = abs(touch->tin[TBINPUTLENGTH-1].loc.x - pos->x);
	int dy = abs(touch->tin[TBINPUTLENGTH-1].loc.y - pos->y);
	if (dx > PIXELDELTAX(di, POINTDELTA) || dy > PIXELDELTAY(di, POINTDELTA))
		touch->tin[TBINPUTLENGTH].mark = 1;
	else
		touch->tin[TBINPUTLENGTH].mark = 0;
	l_memcpy(&touch->tin[TBINPUTLENGTH].loc, pos, sizeof(TTOUCHCOORD));

	// check current with previous
	if (touch->tin[TBINPUTLENGTH-1].mark != 2 && touch->tin[TBINPUTLENGTH-2].mark != 2){
		dx = abs(touch->tin[TBINPUTLENGTH-1].loc.x - touch->tin[TBINPUTLENGTH-2].loc.x);
		dy = abs(touch->tin[TBINPUTLENGTH-1].loc.y - touch->tin[TBINPUTLENGTH-2].loc.y);
		if (dx > PIXELDELTAX(di, POINTDELTA) || dy > PIXELDELTAY(di, POINTDELTA))
			touch->tin[TBINPUTLENGTH-1].mark = 1;
		else
			touch->tin[TBINPUTLENGTH-1].mark = 0;
	}

	for (int i = 0; i < TBINPUTLENGTH; i++)
		l_memcpy(&touch->tin[i], &touch->tin[i+1], sizeof(TIN));

	if (touch->tin[TBINPUTLENGTH].mark != 0)
		return 0;

	int total = 0;
	pos->x = 0;
	pos->y = 0;
	
	for (int i = 0; i < TBINPUTLENGTH; i++){
		if (touch->tin[i].mark){
			pos->x = 0;
			pos->y = 0;
			total = 0;
		}else if (/*!touch->tin[i].mark &&*/ touch->tin[i].loc.y){
			pos->x += touch->tin[i].loc.x;
			pos->y += touch->tin[i].loc.y;
			total++;
		}
	}
	if (total){
		const uint64_t t = getTicks();
		pos->dt = (int)(t - pos->time);
		pos->time = t;
		pos->x /= (float)total;
		pos->y /= (float)total;
		pos->count = touch->count++;
		return total;
	}else{
		return 0;
	}
}

static int detectdrag (TUSBD480 *di, TTOUCH *touch)
{
	if (touch->pos.dt < DRAGDEBOUNCETIME){
		if (touch->dragState)
			return 1;
		
		const int dtx = abs(touch->pos.x - touch->last.x);
		const int dty = abs(touch->pos.y - touch->last.y);
		if ((dtx > PIXELDELTAX(di, 0) || dty > PIXELDELTAY(di, 0)) && dtx && dty){
			if (++touch->dragStartCt == 2){
				touch->dragState = 1;
				return 1;
			}
		}
	}
	return 0;
}

int process_inputReport (TUSBD480 *di, TUSBD480TOUCHCOORD16 *upos, TTOUCHCOORD *pos)
{
	int x = upos->x;
	int y = upos->y;
	calibration_calibratePoint(&di->cal, &x, &y);
	upos->x = x;
	upos->y = y;
	
	pos->x = upos->x;
	pos->y = upos->y;
	pos->z1 = upos->z1;
	pos->z2 = upos->z2;
	pos->pen = upos->pen;
	pos->pressure = upos->pressure;
	l_memcpy(&di->touch.pos, pos, sizeof(TTOUCHCOORD));


	if (!pos->pressure && di->touch.last.pen == di->touch.pos.pen){
		cleanQueue(&di->touch);
		return -2;
	}else if (pos->pen == 1 && di->touch.last.pen == 0){	// pen up
		cleanQueue(&di->touch);
		return 3;
	}else if (pos->pen == 0 && di->touch.last.pen != 0){	// pen down
		cleanQueue(&di->touch);
	}

	if (filter1(di, &di->touch)){
		if (detectdrag(di, &di->touch)){
			//if (filter0(di, &di->touch, &di->touch.pos)){
				calibration_convertPoint(&di->cal, &di->touch.pos.x, &di->touch.pos.y);
			
				pos->x = di->touch.pos.x;
				pos->y = di->touch.pos.y;
				pos->dt = di->touch.pos.dt;
				pos->time = di->touch.pos.time;
				pos->z1 = di->touch.pos.z1;
				pos->z2 = di->touch.pos.z2;
				pos->pen = di->touch.pos.pen;
				pos->pressure = di->touch.pos.pressure;
				pos->count = di->touch.pos.count;
				return 2;
			//}
		}else if (DEBOUNCE(&di->touch)){
			//if (filter0(di, &di->touch, &di->touch.pos)){
				calibration_convertPoint(&di->cal, &di->touch.pos.x, &di->touch.pos.y);
				
				pos->x = di->touch.pos.x;
				pos->y = di->touch.pos.y;
				pos->dt = di->touch.pos.dt;
				pos->time = di->touch.pos.time;
				pos->z1 = di->touch.pos.z1;
				pos->z2 = di->touch.pos.z2;
				pos->pen = di->touch.pos.pen;
				pos->pressure = di->touch.pos.pressure;
				pos->count = di->touch.pos.count;
				return 1;
			//}
		}
	}
	return -2;
}

int libusbd480_GetTouchPosition (TUSBD480 *di, TTOUCHCOORD *pos)
{	
	if (di){
		if (di->usb_handle){
			int result = usb_interrupt_read(di->usb_handle, 0x81, (char*)&di->upos, 16, 500);
			if (result != 16){
				if (result == -116){
					return -1;
				}else{
					//printf("libusbd480_GetTouchPosition(): usb_interrupt_read() error: %d\n", result);
					return 0;
				}
			}else{
				return process_inputReport(di, &di->upos, pos);
			}
		}
	}
	return 0;
}

#endif

