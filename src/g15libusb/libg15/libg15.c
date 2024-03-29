/*
    This file is part of g15tools.

    g15tools is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    g15tools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libg15; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    (c) 2006-2007 The G15tools Project - g15tools.sf.net
    
    $Revision: 282 $ -  $Date: 2008-01-20 06:32:00 +1030 (Sun, 20 Jan 2008) $ $Author: mlampard $
*/


#include "mylcd.h"

#if (__BUILD_G15LIBUSB__)

#ifndef LIBUSB_BLOCKS
#include "../../sync.h"
#else
#include <errno.h>
#ifndef ETIMEDOUT
#  define ETIMEDOUT 10060     /* This is the value in winsock.h. */
#endif
#endif

#include <lusb0_usb.h>		//libusb
#include "libg15.h"


static usb_dev_handle *keyboard_device = 0;
static int libg15_debugging_enabled = 0;
static int enospc_slowdown = 0;
static int found_devicetype = -1;
static int shared_device = 0;
static int g15_keys_endpoint = 0;
static int g15_lcd_endpoint = 0;

#ifndef LIBUSB_BLOCKS
static TTHRDSYNCCTRL tsc;
#endif


//#define PACKAGE_NAME "libg15"
//#define PACKAGE_TARNAME "libg15"
//#define PACKAGE_VERSION "1.2.6"
#define PACKAGE_STRING "libg15 1.2.6.libmylcd"
//#define PACKAGE_BUGREPORT "mlampard@users.sourceforge.net"


/* to add a new device, simply create a new DEVICE() in this list */
/* Fields are: "Name",VendorID,ProductID,Capabilities */
const libg15_devices_t g15_devices[] = {
    DEVICE("Logitech G15",		0x46d,0xc222,G15_LCD|G15_KEYS),
    DEVICE("Logitech G11",		0x46d,0xc225,		 G15_KEYS),
    DEVICE("Logitech Z-10",		0x46d,0x0a07,G15_LCD|G15_KEYS|G15_DEVICE_IS_SHARED),
    DEVICE("Logitech G15 v2",	0x46d,0xc227,G15_LCD|G15_KEYS|G15_DEVICE_5BYTE_RETURN),
    DEVICE("Logitech Gamepanel",0x46d,0xc251,G15_LCD|G15_KEYS|G15_DEVICE_IS_SHARED),
    DEVICE(NULL,0,0,0)
};


				
static void libusb_lock ()
{
#ifndef LIBUSB_BLOCKS
	lock(&tsc);
#endif
}

static void libusb_unlock ()
{
#ifndef LIBUSB_BLOCKS
	unlock(&tsc);
#endif
}

static void libusb_lock_create ()
{
#ifndef LIBUSB_BLOCKS
    lock_create(&tsc);
#endif
}

static void libusb_lock_delete()
{
#ifndef LIBUSB_BLOCKS
	lock_delete(&tsc);
#endif
}

static void u_sleep (int n)
{
	Sleep(n/1000);
}


/* return device capabilities */
int g15DeviceCapabilities() {
    if(found_devicetype>-1)
        return g15_devices[found_devicetype].caps;
    else
        return -1;
}


/* enable or disable debugging */
void libg15Debug(int option) {

    libg15_debugging_enabled = option;
    usb_set_debug(option);
}

/* debugging wrapper */
static int g15_log (FILE *fd, unsigned int level, const char *fmt, ...) {

    if (libg15_debugging_enabled && libg15_debugging_enabled>=level) {
        fprintf(fd,"libg15: ");
        va_list argp;
        va_start(argp, fmt);
        vfprintf(fd,fmt,argp);
        va_end(argp);
    }

    return 0;
}

/* return number of connected and supported devices */
int g15NumberOfConnectedDevices ()
{
    struct usb_bus *bus = 0;
    struct usb_device *dev = 0;
    int i=0;
    unsigned int found = 0;

    for (i=0; g15_devices[i].name !=NULL;i++)
        for (bus = usb_busses; bus; bus = bus->next) 
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if ((dev->descriptor.idVendor == g15_devices[i].vendorid && dev->descriptor.idProduct == g15_devices[i].productid)) 
                found++;
        }
    }
   
    g15_log(stderr,G15_LOG_INFO,"Found %i supported devices\n",found);
    return found;
}

static int initLibUsb()
{
    usb_init();
  /**
     *  usb_find_busses and usb_find_devices both report the number of devices
     *  / busses added / removed since the last call. since this is the first
   *  call we have to return values != 0 or else we didnt find anything */
     
    if (!usb_find_busses())
        return G15_ERROR_OPENING_USB_DEVICE;

    if (!usb_find_devices())
        return G15_ERROR_OPENING_USB_DEVICE;

    return G15_NO_ERROR;
}

static usb_dev_handle * findAndOpenDevice(libg15_devices_t handled_device, int device_index)
{
    struct usb_bus *bus = 0;
    struct usb_device *dev = 0;
    int retries=0;
    int j,i,k,l;
    //int interface=0;
  
    for (bus = usb_busses; bus; bus = bus->next){
        for (dev = bus->devices; dev; dev = dev->next){
            if ((dev->descriptor.idVendor == handled_device.vendorid && dev->descriptor.idProduct == handled_device.productid)) {
                int ret=0;
                usb_dev_handle *devh = 0;
                found_devicetype = device_index;
                g15_log(stderr,G15_LOG_INFO,"Found %s, trying to open it\n",handled_device.name);
#if 0
                devh = usb_open(dev);
                usb_reset(devh);
                u_sleep(5*1000);
                usb_close(devh);
#endif
                devh = usb_open(dev);
                if (!devh){
                    g15_log(stderr,G15_LOG_INFO, "Error, could not open the keyboard\n");
                    g15_log(stderr,G15_LOG_INFO, "Perhaps you dont have enough permissions to access it\n");
                    return 0;
                }

                u_sleep(5*1000);
                //usb_set_configuration(devh, 1);
                //u_sleep(5*1000);

                g15_log(stderr, G15_LOG_INFO, "Device has %i possible configurations\n",dev->descriptor.bNumConfigurations);

                /* if device is shared with another driver, such as the Z-10 speakers sharing with alsa, we have to disable some calls */
                if (g15DeviceCapabilities() & G15_DEVICE_IS_SHARED)
                  shared_device = 1;

                for (j = 0; j<dev->descriptor.bNumConfigurations;j++){
                    struct usb_config_descriptor *cfg = &dev->config[j];

                    for (i=0;i<cfg->bNumInterfaces; i++){
                        struct usb_interface *ifp = &cfg->interface[i];
                        /* if endpoints are already known, finish up */
                        if(g15_keys_endpoint && g15_lcd_endpoint)
                          break;
                        g15_log(stderr, G15_LOG_INFO, "Device has %i Alternate Settings\n", ifp->num_altsetting);

                        for(k=0;k<ifp->num_altsetting;k++){
                            struct usb_interface_descriptor *as = &ifp->altsetting[k];
                            /* verify that the interface is for a HID device */
                            if(as->bInterfaceClass==USB_CLASS_HID){
                                g15_log(stderr, G15_LOG_INFO, "Interface %i has %i Endpoints\n", i, as->bNumEndpoints);
                                u_sleep(5*1000);
        						/* libusb functions ending in _np are not portable between OS's 
                                * Non-linux users will need some way to detach the HID driver from
                                * the G15 until we work out how to do this for other OS's automatically. 
                                * For the moment, we just skip this code..
                                */
#ifdef LIBUSB_HAS_GET_DRIVER_NP
                				char name_buffer[65535];
                				memset(name_buffer, 0, sizeof(name_buffer));
                				
                                ret = usb_get_driver_np(devh, i, name_buffer, 65535);
        						/* some kernel versions say that a driver is attached even though there is none
                                in this case the name buffer has not been changed
                                thanks to RobEngle for pointing this out */
                                if (!ret && name_buffer[0]){
                                    g15_log(stderr,G15_LOG_INFO,"Trying to detach driver currently attached: \"%s\"\n",name_buffer);

                                    ret = usb_detach_kernel_driver_np(devh, i);
                                    if (!ret){
                                        g15_log(stderr,G15_LOG_INFO,"Success, detached the driver\n");
                                    }else{
                                        g15_log(stderr,G15_LOG_INFO,"Sorry, I could not detach the driver, giving up\n");
                                        return 0;
                                    }

                                }
#endif  
                                /* don't set configuration if device is shared */
                                if(0 == shared_device){
                                  ret = usb_set_configuration(devh, 1);
                                  if (ret){
                                    g15_log(stderr,G15_LOG_INFO,"Error setting the configuration, this is fatal\n");
                                    return 0;
                                  }
                                }
                                u_sleep(5*1000);
                                while((ret = usb_claim_interface(devh,i)) && retries <10) {
                                    u_sleep(5*1000);
                                    retries++;
                                    g15_log(stderr,G15_LOG_INFO,"Trying to claim interface\n");
                                }

                                if (ret){
                                    g15_log(stderr,G15_LOG_INFO,"Error claiming interface, good day cruel world\n");
                                    return 0;
                                }

                                for (l=0; l< as->bNumEndpoints;l++){
                                    struct usb_endpoint_descriptor *ep=&as->endpoint[l];
                                    g15_log(stderr, G15_LOG_INFO, "Found %s endpoint %i with address 0x%X maxtransfersize=%i \n",
                                            0x80&ep->bEndpointAddress?"\"Extra Keys\"":"\"LCD\"",
                                            ep->bEndpointAddress&0x0f,ep->bEndpointAddress, ep->wMaxPacketSize
                                           );

                                    if(0x80 & ep->bEndpointAddress) {
                                        g15_keys_endpoint = ep->bEndpointAddress;
                                    } else {
                                        g15_lcd_endpoint = ep->bEndpointAddress; 
                                    }
#if 0
                                    usb_resetep(devh,ep->bEndpointAddress);
#endif
                                }

                                if (ret){
                                    g15_log(stderr, G15_LOG_INFO, "Error setting Alternate Interface\n");
                                }
                            }
                        }
                    }
                }


                g15_log(stderr,G15_LOG_INFO,"Done opening the keyboard\n");
                u_sleep(5*1000); // sleep a bit for good measure 
                return devh;
            }
        }  
    }
    return 0;
}


static usb_dev_handle * findAndOpenG15()
{
    int i;
    for (i=0; g15_devices[i].name !=NULL  ;i++){
        g15_log(stderr,G15_LOG_INFO,"Trying to find %s\n",g15_devices[i].name);
        if((keyboard_device = findAndOpenDevice(g15_devices[i],i))){
            break;
        }else{
            g15_log(stderr,G15_LOG_INFO,"%s not found\n",g15_devices[i].name);
        }
    }
    return keyboard_device;
}


int re_initLibG15()
{

    usb_init();

  /**
     *  usb_find_busses and usb_find_devices both report the number of devices
     *  / busses added / removed since the last call. since this is the first
   *  call we have to return values != 0 or else we didnt find anything */
     
    if (!usb_find_devices())
        return G15_ERROR_OPENING_USB_DEVICE;
  
    keyboard_device = findAndOpenG15();
    if (!keyboard_device)
        return G15_ERROR_OPENING_USB_DEVICE;
 
    return G15_NO_ERROR;
}

int initLibG15()
{
    int retval = G15_NO_ERROR;
    retval = initLibUsb();
    if (retval)
        return retval;

    g15_log(stderr,G15_LOG_INFO,"%s\n",PACKAGE_STRING);
    
#ifdef SUN_LIBUSB
    g15_log(stderr,G15_LOG_INFO,"Using Sun libusb.\n");
#endif

    /*int found = */g15NumberOfConnectedDevices();
	//printf("devices found: %i\n", found);
  
    keyboard_device = findAndOpenG15();
     
	if (!keyboard_device)
		return G15_ERROR_OPENING_USB_DEVICE;
	libusb_lock_create();

    return retval;
}

/* reset the keyboard, returning it to a known state */
int exitLibG15()
{
    if (keyboard_device){
#ifndef SUN_LIBUSB
        int retval = usb_release_interface (keyboard_device, 0);
        u_sleep(5*1000);
#endif
#if 0
        retval = usb_reset(keyboard_device);
        u_sleep(5*1000);
#endif
        usb_close(keyboard_device);
        keyboard_device=0;
        libusb_lock_delete();
        return retval;
    }
    return -1;
}


static void dumpPixmapIntoLCDFormat(unsigned char *lcd_buffer, unsigned char const *data)
{
/*

  For a set of bytes (A, B, C, etc.) the bits representing pixels will appear on the LCD like this:
	
	A0 B0 C0
	A1 B1 C1
	A2 B2 C2
	A3 B3 C3 ... and across for G15_LCD_WIDTH bytes
	A4 B4 C4
	A5 B5 C5
	A6 B6 C6
	A7 B7 C7
	
	A0
	A1  <- second 8-pixel-high row starts straight after the last byte on
	A2     the previous row
	A3
	A4
	A5
	A6
	A7
	A8

	A0
	...
	A0
	...
	A0
	...
	A0
	A1 <- only the first three bits are shown on the bottom row (the last three
	A2    pixels of the 43-pixel high display.)
	

*/

    unsigned int output_offset = G15_LCD_OFFSET;
    unsigned int base_offset = 0;
    unsigned int curr_row = 0;
    unsigned int curr_col = 0;

    /* Five 8-pixel rows + a little 3-pixel row.  This formula will calculate
       the minimum number of bytes required to hold a complete column.  (It
       basically divides by eight and rounds up the result to the nearest byte,
       but at compile time.
      */

#define G15_LCD_HEIGHT_IN_BYTES  ((G15_LCD_HEIGHT + ((8 - (G15_LCD_HEIGHT % 8)) % 8)) / 8)

    for (curr_row = 0; curr_row < G15_LCD_HEIGHT_IN_BYTES; ++curr_row){
        for (curr_col = 0; curr_col < G15_LCD_WIDTH; ++curr_col){
            unsigned int bit = curr_col % 8;
		/* Copy a 1x8 column of pixels across from the source image to the LCD buffer. */
		
            lcd_buffer[output_offset] =
			(((data[base_offset                        ] << bit) & 0x80) >> 7) |
			(((data[base_offset +  G15_LCD_WIDTH/8     ] << bit) & 0x80) >> 6) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 2)] << bit) & 0x80) >> 5) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 3)] << bit) & 0x80) >> 4) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 4)] << bit) & 0x80) >> 3) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 5)] << bit) & 0x80) >> 2) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 6)] << bit) & 0x80) >> 1) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 7)] << bit) & 0x80) >> 0);
            ++output_offset;
            if (bit == 7)
              base_offset++;
        }
	/* Jump down seven pixel-rows in the source image, since we've just
	   done a row of eight pixels in one pass (and we counted one pixel-row
  	   while we were going, so now we skip the next seven pixel-rows.) */
		base_offset += G15_LCD_WIDTH - (G15_LCD_WIDTH / 8);
    }
}

int handle_usb_errors (const char *prefix, int ret)
{

    switch (ret){
        case -ETIMEDOUT:
            return G15_ERROR_READING_USB_DEVICE;  /* backward-compatibility */
            break;
            case -ENOSPC: /* the we dont have enough bandwidth, apparently.. something has to give here.. */
                g15_log(stderr,G15_LOG_INFO,"usb error: ENOSPC.. reducing speed\n");
                enospc_slowdown = 1;
                break;
            case -ENODEV: /* the device went away - we probably should attempt to reattach */
            case -ENXIO: /* host controller bug */
            case -EINVAL: /* invalid request */
            case -EAGAIN: /* try again */
            case -EFBIG: /* too many frames to handle */
            //case -EMSGSIZE: /* msgsize is invalid */
                 g15_log(stderr,G15_LOG_INFO,"usb error: %s %s (%i)\n",prefix,usb_strerror(),ret);     
                 break;
            case -EPIPE: /* endpoint is stalled */
                 g15_log(stderr,G15_LOG_INFO,"usb error: %s EPIPE! clearing...\n",prefix);     
                 libusb_lock();
                 usb_clear_halt(keyboard_device, 0x81);
                 libusb_unlock();
                 break;
            default: /* timed out */
                 g15_log(stderr,G15_LOG_INFO,"Unknown usb error: %s !! (err is %i (%s))\n",prefix,ret,usb_strerror());
    }
    return ret;
}

int writePixmapToLCD (unsigned char const *data)
{
	int ret = 0;

    unsigned char lcd_buffer[G15_BUFFER_LEN];
    
    /* The pixmap conversion function will overwrite everything after G15_LCD_OFFSET, so we only need to blank
       the buffer up to this point.  (Even though the keyboard only cares about bytes 0-23.) */
    memset(lcd_buffer, 0, G15_LCD_OFFSET);  /* G15_BUFFER_LEN); */
    dumpPixmapIntoLCDFormat(lcd_buffer, data);
    
    if(!(g15_devices[found_devicetype].caps & G15_LCD))
        return 0;
  
    /* the keyboard needs this magic byte */
    lcd_buffer[0] = 0x03;
  /* in an attempt to reduce peak bus utilisation, we break the transfer into 32 byte chunks and sleep a bit in between.
    It shouldnt make much difference, but then again, the g15 shouldnt be flooding the bus enough to cause ENOSPC, yet 
    apparently does on some machines...
    I'm not sure how successful this will be in combatting ENOSPC, but we'll give it try in the real-world. */

    if (enospc_slowdown != 0){
		libusb_lock();
		const int chunksize = 64;
        for(int transfercount = 0; transfercount <= 15; transfercount++){
            ret = usb_bulk_write(keyboard_device, g15_lcd_endpoint, (char*)lcd_buffer+(chunksize*transfercount), chunksize, 1000);
            if (ret != chunksize){
				libusb_unlock();
                handle_usb_errors ("LCDPixmap Slow Write",ret);
                return G15_ERROR_WRITING_PIXMAP;
            }
            //u_sleep(10);
         //   Sleep(1);
        }
		libusb_unlock();
    }else{
        /* transfer entire buffer in one hit */
		libusb_lock();
        ret = usb_bulk_write(keyboard_device, g15_lcd_endpoint, (char*)lcd_buffer, G15_BUFFER_LEN, sizeof(lcd_buffer));
		libusb_unlock();
        if (ret != G15_BUFFER_LEN){
            handle_usb_errors ("writePixmapToLCD Write",ret);
            return G15_ERROR_WRITING_PIXMAP;
        }
      //  u_sleep(10);
        //Sleep(1);
    }

    return 0;
}

// 3 levels, 0, 1 and 2
int setLCDContrast (unsigned int level)
{
    unsigned char usb_data[] = { 2, 32, 129, 0 };
  
    if (shared_device>0)
        return G15_ERROR_UNSUPPORTED;
  
    switch(level){
        case 1: 
            usb_data[3] = 22; 
            break;
        case 2: 
            usb_data[3] = 26;
            break;
        default:
            usb_data[3] = 18;
    }  
	libusb_lock();
	int retval = usb_control_msg(keyboard_device, USB_TYPE_CLASS + USB_RECIP_INTERFACE, 9, 0x302, 0, (char*)usb_data, 4, 10000);
	libusb_unlock();
    return retval;
}

int setLEDs (unsigned int leds)
{
    unsigned char m_led_buf[4] = { 2, 4, 0, 0 };
    m_led_buf[2] = ~(unsigned char)leds;
  
	if (shared_device > 0)
		return G15_ERROR_UNSUPPORTED;
	
	libusb_lock();
	int retval = usb_control_msg(keyboard_device, USB_TYPE_CLASS + USB_RECIP_INTERFACE, 9, 0x302, 0, (char*)m_led_buf, 4, 10000); 
	libusb_unlock();
    return retval;
}

int setLCDBrightness (unsigned int level)
{
    unsigned char usb_data[] = { 2, 2, 0, 0 };
  
    if (shared_device>0)
        return G15_ERROR_UNSUPPORTED;
  
    switch(level){
        case 1 : 
            usb_data[2] = 0x10; 
            break;
        case 2 : 
            usb_data[2] = 0x20; 
            break;
        default:
            usb_data[2] = 0x00;
    }

	libusb_lock();
	int retval = usb_control_msg(keyboard_device, USB_TYPE_CLASS + USB_RECIP_INTERFACE, 9, 0x302, 0, (char*)usb_data, 4, 10000); 
	libusb_unlock();
    return retval;
}

/* set the keyboard backlight. doesnt affect lcd backlight. 0==off,1==medium,2==high */
int setKBBrightness (unsigned int level)
{
    unsigned char usb_data[] = { 2, 1, 0, 0 };
  
    if(shared_device>0)
        return G15_ERROR_UNSUPPORTED;

    switch(level){
        case 1 : 
            usb_data[2] = 0x1; 
            break;
        case 2 : 
            usb_data[2] = 0x2; 
            break;
        default:
            usb_data[2] = 0x0;
    }

	libusb_lock();
	int retval = usb_control_msg(keyboard_device, USB_TYPE_CLASS + USB_RECIP_INTERFACE, 9, 0x302, 0, (char*)usb_data, 4, 10000); 
	libusb_unlock();
    return retval;
}

unsigned char g15KeyToLogitechKeyCode(int key)
{
   // first 12 G keys produce F1 - F12, thats 0x3a + key
    if (key < 12)
    {
        return 0x3a + key;
    }
   // the other keys produce Key '1' (above letters) + key, thats 0x1e + key
    else
    {
        return 0x1e + key - 12; // sigh, half an hour to find  -12 ....
    }
}

static void processKeyEvent9Byte(unsigned int *pressed_keys, unsigned char *buffer)
{
    *pressed_keys = 0;
    g15_log(stderr,G15_LOG_WARN,"Keyboard: %x, %x, %x, %x, %x, %x, %x, %x, %x\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8]);
  
    if (buffer[0] == 0x02)
    {
        if (buffer[1]&0x01)
            *pressed_keys |= G15_KEY_G1;
    
        if (buffer[2]&0x02)
            *pressed_keys |= G15_KEY_G2;

        if (buffer[3]&0x04)
            *pressed_keys |= G15_KEY_G3;
    
        if (buffer[4]&0x08)
            *pressed_keys |= G15_KEY_G4;
    
        if (buffer[5]&0x10)
            *pressed_keys |= G15_KEY_G5;

        if (buffer[6]&0x20)
            *pressed_keys |= G15_KEY_G6;

    
        if (buffer[2]&0x01)
            *pressed_keys |= G15_KEY_G7;
    
        if (buffer[3]&0x02)
            *pressed_keys |= G15_KEY_G8;
    
        if (buffer[4]&0x04)
            *pressed_keys |= G15_KEY_G9;
    
        if (buffer[5]&0x08)
            *pressed_keys |= G15_KEY_G10;
    
        if (buffer[6]&0x10)
            *pressed_keys |= G15_KEY_G11;
    
        if (buffer[7]&0x20)
            *pressed_keys |= G15_KEY_G12;
    
        if (buffer[1]&0x04)
            *pressed_keys |= G15_KEY_G13;
    
        if (buffer[2]&0x08)
            *pressed_keys |= G15_KEY_G14;
    
        if (buffer[3]&0x10)
            *pressed_keys |= G15_KEY_G15;
    
        if (buffer[4]&0x20)
            *pressed_keys |= G15_KEY_G16;
    
        if (buffer[5]&0x40)
            *pressed_keys |= G15_KEY_G17;
    
        if (buffer[8]&0x40)
            *pressed_keys |= G15_KEY_G18;
    
        if (buffer[6]&0x01)
            *pressed_keys |= G15_KEY_M1;
        if (buffer[7]&0x02)
            *pressed_keys |= G15_KEY_M2;
        if (buffer[8]&0x04)
            *pressed_keys |= G15_KEY_M3;
        if (buffer[7]&0x40)
            *pressed_keys |= G15_KEY_MR;

        if (buffer[8]&0x80)
            *pressed_keys |= G15_KEY_L1;
        if (buffer[2]&0x80)
            *pressed_keys |= G15_KEY_L2;
        if (buffer[3]&0x80)
            *pressed_keys |= G15_KEY_L3;
        if (buffer[4]&0x80)
            *pressed_keys |= G15_KEY_L4;
        if (buffer[5]&0x80)
            *pressed_keys |= G15_KEY_L5;

        if (buffer[1]&0x80)
            *pressed_keys |= G15_KEY_LIGHT;

    }
}

static void processKeyEvent5Byte(unsigned int *pressed_keys, unsigned char *buffer)
{
    *pressed_keys = 0;
  
    g15_log(stderr,G15_LOG_WARN,"Keyboard: %x, %x, %x, %x, %x\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
  
    if (buffer[0] == 0x02)
    {
        if (buffer[1]&0x01)
            *pressed_keys |= G15_KEY_G1;
    
        if (buffer[1]&0x02)
            *pressed_keys |= G15_KEY_G2;

        if (buffer[1]&0x04)
            *pressed_keys |= G15_KEY_G3;

        if (buffer[1]&0x08)
            *pressed_keys |= G15_KEY_G4;

        if (buffer[1]&0x10)
            *pressed_keys |= G15_KEY_G5;

        if (buffer[1]&0x20)
            *pressed_keys |= G15_KEY_G6;
            
        if (buffer[1]&0x40)
            *pressed_keys |= G15_KEY_M1;
            
        if (buffer[1]&0x80)
            *pressed_keys |= G15_KEY_M2;
            
        if (buffer[2]&0x20)
            *pressed_keys |= G15_KEY_M3;
            
        if (buffer[2]&0x40)
            *pressed_keys |= G15_KEY_MR;

        if (buffer[2]&0x80)
            *pressed_keys |= G15_KEY_L1;
            
        if (buffer[2]&0x2)
            *pressed_keys |= G15_KEY_L2;
            
        if (buffer[2]&0x4)
            *pressed_keys |= G15_KEY_L3;

        if (buffer[2]&0x8)
            *pressed_keys |= G15_KEY_L4;
            
        if (buffer[2]&0x10)
            *pressed_keys |= G15_KEY_L5;
            
        if (buffer[2]&0x1)
            *pressed_keys |= G15_KEY_LIGHT;
    }
}

int getPressedKeys (unsigned int *pressed_keys, unsigned int timeout)
{
    unsigned char buffer[9];
    int ret = 0;
    
	libusb_lock();
    ret = usb_interrupt_read(keyboard_device, g15_keys_endpoint, (char*)buffer, sizeof(buffer), timeout);
    //ret = usb_bulk_read(keyboard_device, g15_keys_endpoint, (char*)buffer, sizeof(buffer), timeout);
	libusb_unlock();
	
    if (ret>0) {
      if (buffer[0] == 1)
        return G15_ERROR_TRY_AGAIN;    
    }

    if (g15DeviceCapabilities() & G15_DEVICE_5BYTE_RETURN) {
      processKeyEvent5Byte(pressed_keys, buffer);
      return G15_NO_ERROR;  
    }
    
    switch(ret) {
      case 5:
          processKeyEvent5Byte(pressed_keys, buffer);
          return G15_NO_ERROR;
      case 9:
          processKeyEvent9Byte(pressed_keys, buffer);
          return G15_NO_ERROR;
      default:
          return handle_usb_errors("Keyboard Read", ret); /* allow the app to deal with errors */
    }
}

int writeMultiplePixmapToLCD (TG15BUFFER *buffer, int total)

{
	int ret = 0;
    if (!(g15_devices[found_devicetype].caps & G15_LCD))
        return 0;

    unsigned char lcd_buffer[total][G15_BUFFER_LEN];

    /* the keyboard needs this magic byte */
    int i;
    for (i = 0; i < total; i++){
		/* The pixmap conversion function will overwrite everything after G15_LCD_OFFSET, so we only need to blank
		the buffer up to this point.  (Even though the keyboard only cares about bytes 0-23.) */
		dumpPixmapIntoLCDFormat(lcd_buffer[i], (ubyte*)buffer[i].data);
    	lcd_buffer[i][0] = 0x03;
    }
    
  /* in an attempt to reduce peak bus utilisation, we break the transfer into 32 byte chunks and sleep a bit in between.
    It shouldnt make much difference, but then again, the g15 shouldnt be flooding the bus enough to cause ENOSPC, yet 
    apparently does on some machines...
    I'm not sure how successful this will be in combatting ENOSPC, but we'll give it try in the real-world. */
    if (enospc_slowdown != 0){
		libusb_lock();
		const int chunksize = 64;
        for (int transfercount = 0; transfercount <= 15; transfercount++){
            ret = usb_interrupt_write(keyboard_device, g15_lcd_endpoint, (char*)lcd_buffer[0]+(chunksize*transfercount), chunksize, 1000);
            if (ret != chunksize){
				libusb_unlock();
                handle_usb_errors ("LCDPixmap Slow Write",ret);
                return G15_ERROR_WRITING_PIXMAP;
            }
        }
        libusb_unlock();		
    }else{
        /* transfer entire buffer in one hit */
		libusb_lock();
        ret = usb_bulk_write(keyboard_device, g15_lcd_endpoint, (char*)lcd_buffer, G15_BUFFER_LEN*total, G15_BUFFER_LEN*total);
		libusb_unlock();
        if (ret != G15_BUFFER_LEN){
            handle_usb_errors("libmylcd: writeMultiplePixmapToLCD",ret);
            return G15_ERROR_WRITING_PIXMAP;
        }
    }

    return ret;
}

#endif
