
// libusb13700 - http://mylcd.sourceforge.net/
// USB13700 available from www.lcdinfo.com

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
 
//	libusb13700 version 0.80 01/10/2008 (d/m/y)


/*
	TODO:

	libusb13700_ExPortIOGet()
	libusb13700_ExPortSPIReceive()
	libusb13700_SPI8Bit()
	libusb13700_DrawRectFromScrBuf()
	libusb13700_DrawRect()

	libusb13700_SetStartupBitmap()
*/


#include "mylcd.h"

#if (__BUILD_USB13700LIBUSB__)

#include <lusb0_usb.h>		//libusb
#include "libusb13700.h"

#if 1
#include "../../memory.h"
#include "../../lstring.h"

#define calloc	l_calloc
#define malloc	l_malloc
#define realloc	l_realloc
#define free	l_free
#define strncpy	l_strncpy
#endif


#define RWTIMEOUT	5000


int libusb13700_Init ()
{
	static int initOnce = 0;

	if (!initOnce){
		initOnce = 1;
		usb_init();
	}
    usb_find_busses();
    usb_find_devices();
	return DISPLAYLIB_OK;	
}


int libusb13700_GetNumberOfDisplays ()
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    int total = 0;
    
    libusb13700_Init();

    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next){
        for (dev = usb_bus->devices; dev; dev = dev->next){
            if (dev->descriptor.idVendor == USB13700_VID && dev->descriptor.idProduct == USB13700_PID)
            	total++;
        }
    }
    return total;
}

usb_dev_handle *usb_openDevice (struct usb_device *dev)
{
	usb_dev_handle *usb_handle = usb_open(dev); 
	if (usb_handle == NULL){
		printf("error: usb_open\n");
		return NULL;
	}
		
	if (usb_set_configuration(usb_handle, 1) < 0){
		printf("error: setting config 1 failed, %s\n", usb_strerror());
		usb_close(usb_handle);
		return NULL;
	}
		
	if (usb_claim_interface(usb_handle, 0) < 0){
		printf("error: claiming interface 0 failed, %s\n", usb_strerror());
		usb_close(usb_handle);
		return NULL;
	}else{
		return usb_handle;
	}
}

void usb_closeDevice (usb_dev_handle *usb_handle)
{
	if (usb_handle != NULL){
		usb_release_interface(usb_handle, 0);
		usb_close(usb_handle);
	}
}

int libusb13700_VersionInfo (DisplayInfo *di, uint32_t *fw, uint32_t *ver)
{
	libusb13700_Init();

	if (di != NULL){
		if (di->Handle)
			if (fw) *fw = di->Version;
	}
	if (ver) *ver = LIBUSB13700_VERSION;
	return DISPLAYLIB_OK;
}

/*
set startup bitmap: 2 writes then 1 read
write 1: FE 6 A size1 size2: size = length of write2
write 2: w w h h size1 size2 nn nn nn nn.  eg: (40 1 F0 0 57 25) = 320 240 9559
read: 6 x14 1

37*256 + 128 = 9600	= full frame
37*256 + 88 = 9560 = all but one line
(40 1 F0 0 5 0 1 1 A5 7F 0 ) = blank frame
(40 1 F0 0 5 0 0 0 A5 7F FF)  = filled frame
(40 1 F0 0 B 0 1 1 A5 57 FF 1 3 0 1 23 FF ) = filled frame with 32 blank pixels from bottom left to right
(40 1 F0 0 B 0 1 1 A5 57 FF 1 4 0 1 22 FF ) = filled frame with 40 blank pixels from bottom left to right
(40 1 F0 0 D 0 1 1 A5 57 FF 1 4 0 FF 0 1  20 FF )
(40 1 F0 0 F 0 1 1 A5 57 FF 1 4 0 FF 0 FF 0  1 1E FF )
(40 1 F0 0 F 0 1 1 A5 57 FF 1 4 0 FF 0 FF 10 1 1E FF )
(40 1 F0 0 F 0 2 2 A5 57 FF 2 4 0 FF 0 FF 1  2 1E FF )
40 1 F0 0 11 0 2 2 A5 57 FF 2 4 0 FF 0 FF 1  FF 3 2 1C FF )
40 1 F0 0 11 0 3 3 A5 57 FF 3 4 0 FF 0 FF 1  FF 2 3 1C FF )


_______________********________********____*____*******************************

10100101 = 165
10100101 = 37
1010111 = 23

*/

int libusb13700_SetStartupBitmap (DisplayInfo *di, uint8_t *fb)
{
	printf("libusb13700_SetStartupBitmap() TODO\n");
	return DISPLAYLIB_ERROR;
	
#if 0
	if (di != NULL && fb != NULL){
		if (di->Handle){
			unsigned char in_buf[8];
			int i, len = di->Width * di->Height;
			int datasize = ((di->Width/8) * di->Height)+6;
			unsigned char *out_buf = (unsigned char *)calloc(sizeof(unsigned char), datasize);
			unsigned char *gfx_buf = &out_buf[6];
						
			if (out_buf != NULL){
				out_buf[0] = 0xFE;
				out_buf[1] = CMD_SET_STARTUPBITMAP;
				out_buf[2] = MSG_COMMAND;
				out_buf[3] = datasize & 0xFF;
				out_buf[4] = (datasize >> 8)&0xFF;
				
				if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, 5, RWTIMEOUT) != 5){
					printf("error: bulk_write\n");
					free(out_buf);
					return DISPLAYLIB_ERROR;
				}
			
				out_buf[0] = di->Width & 0xFF;
				out_buf[1] = (di->Width >> 8)&0xFF;
				out_buf[2] = di->Height & 0xFF;
				out_buf[3] = (di->Height >> 8)&0xFF;
				out_buf[4] = (datasize-6) & 0xFF;
				out_buf[5] = ((datasize-6) >> 8)&0xFF;
								
				for (i = 0; i < len; i+=8){
					*gfx_buf = fb[i+7] | fb[i+6]<<1 | fb[i+5]<<2 | fb[i+4]<<3 | fb[i+3]<<4 | fb[i+2]<<5 | fb[i+1]<<6 | fb[i]<<7;
					//*gfx_buf ^= 0xFF;
					gfx_buf++;
				}
			
				i = usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, datasize, RWTIMEOUT);
				free(out_buf);
				if (i != datasize){
					printf("error: bulk_write: %i\n",i);
					return DISPLAYLIB_ERROR;
				}

				if (usb_bulk_read((usb_dev_handle *)di->Handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT) < 0){
					printf("error: bulk_read\n");
					return DISPLAYLIB_ERROR;
				}
			
				if (in_buf[0] == CMD_SET_STARTUPBITMAP && in_buf[1] == MSG_REPLY && in_buf[2] == FW_CMD_SUCCESS){
					return DISPLAYLIB_OK;
				}else{
					printf("error: bad reply: %x %x %x\n", in_buf[0], in_buf[1], in_buf[2]);
					return DISPLAYLIB_ERROR;
				}
			}else{
				printf("error: alloc failed\n");
			}
		}
	}
	return DISPLAYLIB_ERROR;
#endif
}

int libusb13700_GetTouchScreenSample (DisplayInfo *di, uint32_t *x, uint32_t *y)
{
	if (di != NULL){
		if (di->Handle){
			unsigned char out_buf[3];
			unsigned char in_buf[8];

			out_buf[0] = 0xFE;
			out_buf[1] = CMD_GET_TOUCHSCREEN_SAMPLE;
			out_buf[2] = MSG_COMMAND;
	
			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
				printf("error: bulk_write\n");
				return DISPLAYLIB_ERROR;
			}
			
			int bytes_returned = usb_bulk_read((usb_dev_handle *)di->Handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT);
			if (bytes_returned < 0){
				printf("error: bulk_read\n");
				return DISPLAYLIB_ERROR;
			}
			
			if (in_buf[0] == CMD_GET_TOUCHSCREEN_SAMPLE && in_buf[1] == MSG_REPLY){
				if (x) *x = in_buf[3] | (in_buf[4] << 8);
				if (y) *y = in_buf[5] | (in_buf[6] << 8);
				return DISPLAYLIB_OK;
			}else{
				printf("error: bad reply\n");
				return DISPLAYLIB_ERROR;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_SetBacklight (DisplayInfo *di, uint32_t onOff, uint32_t brightness)
{
	if (di != NULL){
		if (di->Handle){
			unsigned char out_buf[5];
			unsigned char in_buf[8];

			out_buf[0] = 0xFE;
			out_buf[1] = CMD_SET_BACKLIGHT;
			out_buf[2] = MSG_COMMAND;
			out_buf[3] = onOff&0xFF;
			out_buf[4] = brightness&0xFF;
	
			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
				printf("error: bulk_write\n");
				return DISPLAYLIB_ERROR;
			}
			
			int bytes_returned = usb_bulk_read((usb_dev_handle *)di->Handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT);
			if (bytes_returned < 0){
				printf("error: bulk_read\n");
				return DISPLAYLIB_ERROR;
			}
			
			if (in_buf[0] == CMD_SET_BACKLIGHT && in_buf[1] == MSG_REPLY && in_buf[2] == FW_CMD_SUCCESS){
				return DISPLAYLIB_OK;
			}else{
				printf("error: bad reply\n");
				return DISPLAYLIB_ERROR;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int get13700BacklightConfigData (usb_dev_handle *usb_handle, DisplayInfo *di)
{
	unsigned char out_buf[3];
	unsigned char in_buf[8];

	out_buf[0] = 0xFE;
	out_buf[1] = CMD_GET_BACKLIGHTCONFIGDATA;
	out_buf[2] = MSG_COMMAND;
	
	if (usb_bulk_write(usb_handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
		printf("error: bulk_write\n");
		return DISPLAYLIB_ERROR;
	}

	int bytes_returned = usb_bulk_read(usb_handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT);
	if (bytes_returned < 0){
		printf("error: bulk_read\n");
		return DISPLAYLIB_ERROR;
	}
	
	if (in_buf[0] == CMD_GET_BACKLIGHTCONFIGDATA && in_buf[1] == MSG_REPLY){
		di->BacklightControlPWM = in_buf[2];
		di->BacklightCCFLPWM = in_buf[3];
		di->BacklightConfig = in_buf[4];
		di->BacklightPWMFrequency = in_buf[5] | (in_buf[6]<<8);
		return DISPLAYLIB_OK;
	}else{
		printf("error: bad reply\n");
		return DISPLAYLIB_ERROR;
	}
}

int libusb13700_ExPortConfig (DisplayInfo *di, uint8_t port0, uint8_t port1)
{
	if (di != NULL){
		if (di->Handle){
			unsigned char out_buf[7];
			unsigned char in_buf[8];
			
			out_buf[0] = 0xFE;
			out_buf[1] = CMD_WRITE_EXPORTCONFIG;
			out_buf[2] = MSG_COMMAND;
			out_buf[3] = port0;
			out_buf[4] = port1;
			out_buf[5] = port0;
			out_buf[6] = port1;
			
			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
				printf("error: bulk_write\n");
				return DISPLAYLIB_ERROR;
			}
			
			int bytes_returned = usb_bulk_read((usb_dev_handle *)di->Handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT);
			if (bytes_returned < 0){
				printf("error: bulk_read\n");
				return DISPLAYLIB_ERROR;
			}
			
			if (in_buf[0] == CMD_WRITE_EXPORTCONFIG && in_buf[1] == MSG_REPLY && in_buf[2] == FW_CMD_SUCCESS){
				return DISPLAYLIB_OK;
			}else{
				printf("error: bad reply\n");
				return DISPLAYLIB_ERROR;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_ExPortIOConfig (DisplayInfo *di, uint8_t port0Dir, uint8_t port1Dir)
{
	if (di != NULL){
		if (di->Handle){
			unsigned char out_buf[7];
			unsigned char in_buf[8];
			
			out_buf[0] = 0xFE;
			out_buf[1] = CMD_WRITE_EXPORTIOCONFIG;
			out_buf[2] = MSG_COMMAND;
			out_buf[3] = port0Dir;
			out_buf[4] = port1Dir;
			out_buf[5] = port0Dir;
			out_buf[6] = port1Dir;
			
			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
				printf("error: bulk_write\n");
				return DISPLAYLIB_ERROR;
			}
			
			int bytes_returned = usb_bulk_read((usb_dev_handle *)di->Handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT);
			if (bytes_returned < 0){
				printf("error: bulk_read\n");
				return DISPLAYLIB_ERROR;
			}
			
			if (in_buf[0] == CMD_WRITE_EXPORTIOCONFIG && in_buf[1] == MSG_REPLY && in_buf[2] == FW_CMD_SUCCESS){
				return DISPLAYLIB_OK;
			}else{
				printf("error: bad reply\n");
				return DISPLAYLIB_ERROR;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_ExPortSPIConfig (DisplayInfo *di, uint8_t frameLength, uint8_t frameFormat, 
								 uint8_t cpol, uint8_t cpha, uint8_t bitclock, uint8_t prescaler, uint8_t sselMode)
{
	if (di != NULL){
		if (di->Handle){
			unsigned char out_buf[9];
			unsigned char in_buf[8];
			
			out_buf[0] = 0xFE;
			out_buf[1] = CMD_WRITE_EXPORTSPICONFIG;
			out_buf[2] = MSG_COMMAND;
			out_buf[3] = frameLength;
			out_buf[4] = bitclock;
			out_buf[5] = prescaler;
			out_buf[6] = sselMode;
			out_buf[7] = 0;
			out_buf[8] = 1;
			
			if (frameFormat == 1) out_buf[3] |= 0x10;
			if (frameFormat == 2) out_buf[3] |= 0x20;
			if (cpol) out_buf[3] |= 0x40;
			if (cpha) out_buf[3] |= 0x80;

			
			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
				printf("error: bulk_write\n");
				return DISPLAYLIB_ERROR;
			}
			
			int bytes_returned = usb_bulk_read((usb_dev_handle *)di->Handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT);
			if (bytes_returned < 0){
				printf("error: bulk_read\n");
				return DISPLAYLIB_ERROR;
			}
			
			if (in_buf[0] == CMD_WRITE_EXPORTSPICONFIG && in_buf[1] == MSG_REPLY && in_buf[2] == FW_CMD_SUCCESS){
				return DISPLAYLIB_OK;
			}else{
				printf("error: bad reply\n");
				return DISPLAYLIB_ERROR;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_ExPortIOSet (DisplayInfo *di, uint8_t port0Mask, uint8_t port0, uint8_t port1Mask, uint8_t port1)
{
	if (di != NULL){
		if (di->Handle){
			unsigned char out_buf[7];
			
			out_buf[0] = 0xFE;
			out_buf[1] = CMD_SET_EXPORTIO;
			out_buf[2] = MSG_COMMAND;
			out_buf[3] = port0Mask;
			out_buf[4] = port0;
			out_buf[5] = port1Mask;
			out_buf[6] = port1;
			
			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
				printf("error: bulk_write\n");
				return DISPLAYLIB_ERROR;
			}else{
				return DISPLAYLIB_OK;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

/*
# DisplayLib_ExPortSPISend() size: 0xC61E/50718
usb_bulk_write() ep:2 size:50723 (FE 14 A 1E C6 25 0 75 0 2 1 8
*/
int libusb13700_ExPortSPISend (DisplayInfo *di, uint8_t *data, uint32_t size)
{
	if (di != NULL){
		if (di->Handle){
			int dlen = (size+5) * sizeof(unsigned char);
			unsigned char *out_buf = (unsigned char *)malloc(dlen);
			
			out_buf[0] = 0xFE;
			out_buf[1] = CMD_WRITE_EXPORTSPI;
			out_buf[2] = MSG_COMMAND;
			out_buf[3] = size & 0xFF;
			out_buf[4] = (size >> 8)&0xFF;

			memcpy(&out_buf[5], data, size);
			
			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, dlen, RWTIMEOUT) != dlen){
				free(out_buf);
				printf("error: bulk_write\n");
				return DISPLAYLIB_ERROR;
			}else{
				free(out_buf);
				return DISPLAYLIB_OK;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int get13700BasicConfigData (usb_dev_handle *usb_handle, DisplayInfo *di)
{
	unsigned char out_buf[3];
	unsigned char in_buf[64];

	out_buf[0] = 0xFE;
	out_buf[1] = CMD_GET_BASICCONFIGDATA;
	out_buf[2] = MSG_COMMAND;
	
	if (usb_bulk_write(usb_handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
		printf("error: bulk_write\n");
		return DISPLAYLIB_ERROR;
	}

	int bytes_returned = usb_bulk_read(usb_handle, EP_IN, (char*)in_buf, sizeof(in_buf), RWTIMEOUT);
	if (bytes_returned < 0){
		printf("error: bulk_read\n");
		return DISPLAYLIB_ERROR;
	}

	if (in_buf[0] == CMD_GET_BASICCONFIGDATA && in_buf[1] == MSG_REPLY){
		di->Width = in_buf[2] | (in_buf[3] << 8);
		di->Height = in_buf[4] | (in_buf[5] << 8);

		// 32 bytes, device name string ("USB13700")
		memcpy(di->Name, (char*)&in_buf[6], 32);

		// 16 bytes, username string
		memcpy(di->Username, (char*)&in_buf[38], 15);
		
		// 1 byte, mode (should be MODE_NORMAL)
		di->Mode = in_buf[54];
		
		// 2 bytes, firmware version
		di->Version = in_buf[55] | (in_buf[56] << 8);
		
		// 1 byte, dispclkdiv
		di->DispClkDiv = in_buf[57];
		
		// 1 byte, fpshiftdiv
		di->FPSHIFTDiv = in_buf[58];
		
		// 1 byte, tcrcrdiff
		di->TCRCRDiff = in_buf[59];

		di->Handle = (uintptr_t)usb_handle;
		di->Queue = NULL;
		return DISPLAYLIB_OK;
	}else{
		printf("error: bad reply\n");
		return DISPLAYLIB_ERROR;
	}
}

int get_display_configuration (usb_dev_handle *usb_handle, DisplayInfo *di)
{
	return (get13700BasicConfigData(usb_handle, di) &&
			get13700BacklightConfigData(usb_handle, di));
}

int libusb13700_GetDisplayConfiguration (uint32_t index, DisplayInfo *di)
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    
    libusb13700_Init();

    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next){
        for (dev = usb_bus->devices; dev; dev = dev->next){
            if (dev->descriptor.idVendor == USB13700_VID && dev->descriptor.idProduct == USB13700_PID){
            	if (index-- == 0){
           			usb_dev_handle *usb_handle = usb_openDevice(dev);
           			if (usb_handle != NULL){
        				int ret = get_display_configuration(usb_handle, di);
           				usb_closeDevice(usb_handle);
           				return ret;
           			}
            	}
            }
        }
    }
    return DISPLAYLIB_ERROR;
}

int libusb13700_OpenDisplay (DisplayInfo *di, uint32_t index)
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    
    libusb13700_Init();
    
    if (di == NULL) return DISPLAYLIB_ERROR;

    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next){
        for (dev = usb_bus->devices; dev; dev = dev->next){
            if (dev->descriptor.idVendor == USB13700_VID && dev->descriptor.idProduct == USB13700_PID){
            	if (index-- == 0){
           			usb_dev_handle *usb_handle = usb_openDevice(dev);
           			if (usb_handle != NULL){
           				di->Handle = (uintptr_t)usb_handle;
           				di->Queue = (TLIBUSB13700QUEUE*)malloc(sizeof(TLIBUSB13700QUEUE));
           				if (di->Queue != NULL){
           					di->Queue->Data = (unsigned char*)calloc(MSG_QUEUE_SIZE, sizeof(unsigned char));
           					if (di->Queue->Data != NULL){
		           				di->Queue->Size = MSG_QUEUE_SIZE;
        		   				di->Queue->Position = 0;
           						return DISPLAYLIB_OK;
           					}
           					free(di->Queue);
           					usb_closeDevice(usb_handle);
           				}
           			}
            	}
            }
        }
    }
    return DISPLAYLIB_ERROR;
}

int libusb13700_CloseDisplay (DisplayInfo *di)
{
	if (di != NULL){
		if (di->Handle){
			usb_dev_handle *usb_handle = (usb_dev_handle*)di->Handle;
			usb_closeDevice(usb_handle);
			di->Handle = 0;
			if (di->Queue){
				if (di->Queue->Data)
					free(di->Queue->Data);
				free(di->Queue);
				di->Queue = NULL;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_DrawScreen (DisplayInfo *di, uint8_t *fb)
{
	if (di != NULL && fb != NULL){
		if (di->Handle){
			int databytes = (di->Width/8) * di->Height;
			unsigned char *out_buf = (unsigned char *)malloc(sizeof(char)* (databytes+5));
			if (out_buf != NULL){
				out_buf[0] = 0xFE;
				out_buf[1] = CMD_WRITE_FULLSCREEN;
				out_buf[2] = MSG_COMMAND;
				out_buf[3] = databytes & 0xFF;
				out_buf[4] = (databytes >> 8)&0xFF;
				
				unsigned char *gfx_buf = &out_buf[5];
				int len = di->Width * di->Height;
				int i;
								
				for (i = 0; i < len; i+=8){
					*gfx_buf++ = fb[i+7] | fb[i+6]<<1 | fb[i+5]<<2 | fb[i+4]<<3 | fb[i+3]<<4 | fb[i+2]<<5 | fb[i+1]<<6 | fb[i]<<7;
					//*gfx_buf++ = (fb[i+7]&0x01) | (fb[i+6]<<1&0x02) | ((fb[i+5]<<2)&0x04) | (fb[i+4]<<3&0x08) | (fb[i+3]<<4&0x10) | (fb[i+2]<<5&0x20) | (fb[i+1]<<6&0x40) | (fb[i]<<7&0x80);
				}

				if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, databytes+5, RWTIMEOUT) != databytes+5){
					printf("error: usb_bulk_write\n");
					free(out_buf);
					return DISPLAYLIB_ERROR;
				}else{
					free(out_buf);
					return DISPLAYLIB_OK;
				}
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_WriteText (DisplayInfo *di, char *text, uint32_t size, uint32_t x, uint32_t y)
{

	if (di != NULL){
		if (di->Handle){
			uint32_t len = 0;
			while(len < size){
				if (!text[len++])
					break;
			}
			if (len < size) size = len;
			int databytes = size+7;
			
			unsigned char *out_buf = (unsigned char *)malloc(sizeof(char)* databytes);
			if (out_buf != NULL){
				out_buf[0] = 0xFE;
				out_buf[1] = CMD_WRITE_TEXT;
				out_buf[2] = MSG_COMMAND;
				out_buf[3] = size & 0xFF;
				out_buf[4] = (size >> 8)&0xFF;
				out_buf[5] = x & 0xFF;
				out_buf[6] = y & 0xFF;
				
				memcpy(&out_buf[7], text, size);
				
				if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, databytes, RWTIMEOUT) != databytes){
					printf("error: usb_bulk_write\n");
					free(out_buf);
					return DISPLAYLIB_ERROR;
				}else{
					free(out_buf);
					return DISPLAYLIB_OK;
				}
			}
		}
	}
	return DISPLAYLIB_ERROR;	
}

static int send13700DataCmd (usb_dev_handle *usb_handle, int msg, int data)
{
	unsigned char out_buf[4] = {0xFE, msg, MSG_COMMAND, data};
	if (usb_bulk_write(usb_handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != sizeof(out_buf)){
		printf("error: usb_bulk_write\n");
		return DISPLAYLIB_ERROR;
	}else{
		return DISPLAYLIB_OK;
	}
}

static int alloc13700DataCmd (DisplayInfo *di, uint32_t size)
{
	if (di->Queue->Position+size+8 > (uint32_t)di->Queue->Size){
		int datasize = di->Queue->Size+abs((di->Queue->Position+size+8) - di->Queue->Size)+MSG_QUEUE_STEP;
		di->Queue->Data = (unsigned char *)realloc(di->Queue->Data, datasize);
		if (di->Queue->Data != NULL){
			di->Queue->Size = datasize;
			return DISPLAYLIB_OK;
		}else{
			di->Queue->Position = 0;
			di->Queue->Size = 0;
			di->Queue->Data = malloc(4);
			printf("alloc13700DataCmd failed 1\n");
			return DISPLAYLIB_ERROR;
		}
	}

	return DISPLAYLIB_OK;
}

int libusb13700_WriteS1D13700Command (DisplayInfo *di, uint8_t cmd, uint32_t queue)
{
	
	if (di != NULL){
		if (di->Handle){
			if (queue){
				if (alloc13700DataCmd(di, 4)){
					unsigned char *out_buf = (unsigned char*)&di->Queue->Data[di->Queue->Position];
					di->Queue->Position += 4;
					out_buf[0] = 0xFE;
					out_buf[1] = CMD_WRITE_S1D13700_COMMAND;
					out_buf[2] = MSG_COMMAND;
					out_buf[3] = cmd;
					return DISPLAYLIB_OK;
				}
			}
			return send13700DataCmd((usb_dev_handle *)di->Handle, CMD_WRITE_S1D13700_COMMAND, cmd);
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_WriteS1D13700DataByte (DisplayInfo *di, uint8_t data, uint32_t queue)
{
	
	if (di != NULL){
		if (di->Handle){
			if (queue){
				if (alloc13700DataCmd(di, 4)){
					unsigned char *out_buf = (unsigned char*)&di->Queue->Data[di->Queue->Position];
					di->Queue->Position += 4;
					out_buf[0] = 0xFE;
					out_buf[1] = CMD_WRITE_S1D13700_DATA_BYTE;
					out_buf[2] = MSG_COMMAND;
					out_buf[3] = data;
					return DISPLAYLIB_OK;
				}
			}
			return send13700DataCmd((usb_dev_handle *)di->Handle, CMD_WRITE_S1D13700_DATA_BYTE, data);
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_WriteS1D13700Data (DisplayInfo *di, uint8_t *data, uint32_t size, uint32_t queue)
{
	
	if (di != NULL){
		if (di->Handle){
			if (queue){
				if (size > 0xFFFF){
					printf("error: data too large\n");
					return DISPLAYLIB_ERROR;
				}

				if (alloc13700DataCmd(di, size+5)){
					unsigned char *out_buf = (unsigned char*)&di->Queue->Data[di->Queue->Position];
					di->Queue->Position += (5 + size);
					out_buf[0] = 0xFE;
					out_buf[1] = CMD_WRITE_S1D13700_DATA;
					out_buf[2] = MSG_COMMAND;
					out_buf[3] = size & 0xFF;
					out_buf[4] = (size >> 8)&0xFF;
					memcpy(&out_buf[5], data, size);
					return DISPLAYLIB_OK;
				}
			}
			unsigned char out_buf[size+5];
			out_buf[0] = 0xFE;
			out_buf[1] = CMD_WRITE_S1D13700_DATA;
			out_buf[2] = MSG_COMMAND;
			out_buf[3] = size & 0xFF;
			out_buf[4] = (size >> 8)&0xFF;
			memcpy(&out_buf[5], data, size);

			if (usb_bulk_write((usb_dev_handle *)di->Handle, EP_OUT, (char*)out_buf, sizeof(out_buf), RWTIMEOUT) != (int)sizeof(out_buf)){
				printf("error: usb_bulk_write\n");
				return DISPLAYLIB_ERROR;
			}else{
				return DISPLAYLIB_OK;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_WriteS1D13700ExecuteQueue (DisplayInfo *di)
{
	
	if (di != NULL){
		if (di->Handle){
			if (!di->Queue->Position)
				return DISPLAYLIB_OK;

			int ret = usb_bulk_write((usb_dev_handle*)di->Handle, EP_OUT, (char*)di->Queue->Data, di->Queue->Position, RWTIMEOUT);
			if (ret != di->Queue->Position){
				di->Queue->Position = 0;
				printf("error: usb_bulk_write, ret:%i %s\n", ret, usb_strerror());
				return DISPLAYLIB_ERROR;
			}else{
				di->Queue->Position = 0;
				return DISPLAYLIB_OK;
			}
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_WriteS1D13700ClearQueue (DisplayInfo *di)
{
	if (di != NULL){
		if (di->Queue){
			di->Queue->Position = 0;
			return DISPLAYLIB_OK;
		}
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_WriteS1D13700GetQueueSize (DisplayInfo *di)
{
	if (di != NULL){
		if (di->Queue)
			return di->Queue->Position;
	}
	return DISPLAYLIB_ERROR;
}

int libusb13700_ExPortSPIReceive (DisplayInfo *di, uint32_t *rcvData)
{
	printf("libusb13700_ExPortSPIReceive() TODO\n");
	return DISPLAYLIB_ERROR;
}

int libusb13700_ExPortIOGet (DisplayInfo *di, uint8_t *port0, uint8_t *port1)
{
	printf("libusb13700_ExPortIOGet() TODO\n");
	return DISPLAYLIB_ERROR;
}

int libusb13700_DrawRect (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	printf("libusb13700_DrawRect() TODO\n");
	return DISPLAYLIB_ERROR;
}

int libusb13700_DrawRectFromScrBuf (DisplayInfo *di, uint8_t *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	printf("libusb13700_DrawRectFromScrBuf() TODO\n");
	return DISPLAYLIB_ERROR;
}

int libusb13700_SPI8Bit (DisplayInfo *di, uint8_t dc, uint8_t txData, uint8_t *rxData)
{
	printf("libusb13700_SPI8Bit() TODO\n");
	return DISPLAYLIB_ERROR;
}


#endif

