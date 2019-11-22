/* LIBUSB-WIN32, Generic Windows USB Library
 * Copyright (c) 2002-2005 Stephan Meyer <ste_meyer@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdio.h>
#include <windows.h>
#include <errno.h>
#include "../src/lusb0_usb.h"	/* libUSB */


#define _DEBUG				1
#define _BANDWIDTH			0
#define LIBUSB_DLL_NAME		"_libusb0.dll"



typedef usb_dev_handle *(*usb_open_t) (struct usb_device *dev);
typedef int (*usb_close_t) (usb_dev_handle *dev);
typedef int (*usb_get_string_t) (usb_dev_handle *dev, int index, int langid, char *buf, size_t buflen);
typedef int (*usb_get_string_simple_t) (usb_dev_handle *dev, int index, char *buf, size_t buflen);
typedef int (*usb_get_descriptor_by_endpoint_t) (usb_dev_handle *udev, int ep, unsigned char type, unsigned char index, void *buf, int size);
typedef int (*usb_get_descriptor_t) (usb_dev_handle *udev, unsigned char type, unsigned char index, void *buf, int size);
typedef int (*usb_bulk_write_t) (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout);
typedef int (*usb_bulk_read_t) (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout);
typedef int (*usb_interrupt_write_t) (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout);
typedef int (*usb_interrupt_read_t) (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout);
typedef int (*usb_control_msg_t) (usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, int size, int timeout);
typedef int (*usb_set_configuration_t) (usb_dev_handle *dev, int configuration);
typedef int (*usb_claim_interface_t) (usb_dev_handle *dev, int interface);
typedef int (*usb_release_interface_t) (usb_dev_handle *dev, int interface);
typedef int (*usb_set_altinterface_t) (usb_dev_handle *dev, int alternate);
typedef int (*usb_resetep_t) (usb_dev_handle *dev, unsigned int ep);
typedef int (*usb_clear_halt_t) (usb_dev_handle *dev, unsigned int ep);
typedef int (*usb_reset_t) (usb_dev_handle *dev);
typedef int (*usb_reset_ex_t) (usb_dev_handle *dev, unsigned int reset_type);
typedef char * (*usb_strerror_t) (void);
typedef void (*usb_init_t) (void);
typedef void (*usb_set_debug_t) (int level);
typedef int (*usb_find_busses_t) (void);
typedef int (*usb_find_devices_t) (void);
typedef struct usb_device * (*usb_device_t) (usb_dev_handle *dev);
typedef struct usb_bus * (*usb_get_busses_t) (void);
typedef int (*usb_install_service_np_t) (void);
typedef void (*usb_install_service_np_rundll_t) (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show);
typedef int (*usb_uninstall_service_np_t) (void);
typedef void (*usb_uninstall_service_np_rundll_t) (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show);
typedef int (*usb_install_driver_np_t) (const char *inf_file);
typedef void (*usb_install_driver_np_rundll_t) (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show);
typedef int (*usb_touch_inf_file_np_t) (const char *inf_file);
typedef void (*usb_touch_inf_file_np_rundll_t) (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show);
typedef int (*usb_install_needs_restart_np_t) (void);
typedef int (*usb_install_npW_t) (HWND hwnd, HINSTANCE instance, LPCWSTR cmd_line, int starg_arg);
typedef int (*usb_install_npA_t) (HWND hwnd, HINSTANCE instance, LPCSTR cmd_line, int starg_arg);
typedef void (*usb_install_np_rundll_t) (HWND wnd, HINSTANCE instance,  LPSTR cmd_line, int cmd_show);
typedef struct usb_version * (*usb_get_version_t) (void);
typedef int (*usb_isochronous_setup_async_t) (usb_dev_handle *dev, void **context, unsigned char ep, int pktsize);
typedef int (*usb_bulk_setup_async_t) (usb_dev_handle *dev, void **context, unsigned char ep);
typedef int (*usb_interrupt_setup_async_t) (usb_dev_handle *dev, void **context, unsigned char ep);
typedef int (*usb_submit_async_t) (void *context, char *bytes, int size);
typedef int (*usb_reap_async_t) (void *context, int timeout);
typedef int (*usb_reap_async_nocancel_t) (void *context, int timeout);
typedef int (*usb_cancel_async_t) (void *context);
typedef int (*usb_free_async_t) (void **context);



static usb_open_t		                    _usb_open = NULL;
static usb_close_t                          _usb_close = NULL;
static usb_get_string_t                     _usb_get_string = NULL;
static usb_get_string_simple_t              _usb_get_string_simple = NULL;
static usb_get_descriptor_by_endpoint_t     _usb_get_descriptor_by_endpoint = NULL;
static usb_get_descriptor_t                 _usb_get_descriptor = NULL;
static usb_bulk_write_t                     _usb_bulk_write = NULL;
static usb_bulk_read_t                      _usb_bulk_read = NULL;
static usb_interrupt_write_t                _usb_interrupt_write = NULL;
static usb_interrupt_read_t                 _usb_interrupt_read = NULL;
static usb_control_msg_t                    _usb_control_msg = NULL;
static usb_set_configuration_t              _usb_set_configuration = NULL;
static usb_claim_interface_t                _usb_claim_interface = NULL;
static usb_release_interface_t              _usb_release_interface = NULL;
static usb_set_altinterface_t               _usb_set_altinterface = NULL;
static usb_resetep_t                        _usb_resetep = NULL;
static usb_clear_halt_t                     _usb_clear_halt = NULL;
static usb_reset_t                          _usb_reset = NULL;
static usb_reset_ex_t                       _usb_reset_ex = NULL;
static usb_strerror_t                       _usb_strerror = NULL;
static usb_init_t                           _usb_init = NULL;
static usb_set_debug_t                      _usb_set_debug = NULL;
static usb_find_busses_t                    _usb_find_busses = NULL;
static usb_find_devices_t                   _usb_find_devices = NULL;
static usb_device_t                         _usb_device = NULL;
static usb_get_busses_t                     _usb_get_busses = NULL;
static usb_install_service_np_t             _usb_install_service_np = NULL;
static usb_install_service_np_rundll_t      _usb_install_service_np_rundll = NULL;
static usb_uninstall_service_np_t           _usb_uninstall_service_np = NULL;
static usb_uninstall_service_np_rundll_t    _usb_uninstall_service_np_rundll = NULL;
static usb_install_driver_np_t              _usb_install_driver_np = NULL;
static usb_install_driver_np_rundll_t       _usb_install_driver_np_rundll = NULL;
static usb_touch_inf_file_np_t              _usb_touch_inf_file_np = NULL;
static usb_touch_inf_file_np_rundll_t       _usb_touch_inf_file_np_rundll = NULL;
static usb_install_needs_restart_np_t       _usb_install_needs_restart_np = NULL;
static usb_install_npW_t                    _usb_install_npW = NULL;
static usb_install_npA_t                    _usb_install_npA = NULL;
static usb_install_np_rundll_t              _usb_install_np_rundll = NULL;
static usb_get_version_t                    _usb_get_version = NULL;
static usb_isochronous_setup_async_t        _usb_isochronous_setup_async = NULL;
static usb_bulk_setup_async_t               _usb_bulk_setup_async = NULL;
static usb_interrupt_setup_async_t          _usb_interrupt_setup_async = NULL;
static usb_submit_async_t                   _usb_submit_async = NULL;
static usb_reap_async_t                     _usb_reap_async = NULL;
static usb_reap_async_nocancel_t            _usb_reap_async_nocancel = NULL;
static usb_cancel_async_t                   _usb_cancel_async = NULL;
static usb_free_async_t                     _usb_free_async = NULL;



static HINSTANCE hLibusb_dll = NULL;


void my_lock ();
void my_unlock ();
int my_printf (const char *str, ...)
	__attribute__((format(printf, 1, 2)));
int my_wprintf (const wchar_t *wstr, ...);
	/*__attribute__((format(__wprintf__, 1, 2)));*/

	
int initDebugPrint ();
void closeDebugPrint ();


#if (_DEBUG)
# define dbprintf my_printf
# define dbwprintf my_wprintf
#else
# define dbprintf(X,...) 
# define dbwprintf(X,...) 
#endif


#if (_BANDWIDTH)
static size_t total = 0;
static unsigned int t0 = 0;
#endif




void usb_init (void)
{
	
	initDebugPrint();
	dbprintf("usb_init()\n");

	if (hLibusb_dll == NULL){
		hLibusb_dll = LoadLibraryA(LIBUSB_DLL_NAME);
		if (hLibusb_dll == NULL){
			dbprintf("error loading '%s' (%i)\n", LIBUSB_DLL_NAME, (int)GetLastError());
			return;
		}else{
			dbprintf("'%s' loaded\n", LIBUSB_DLL_NAME);
		}
	}

  _usb_open 						= (usb_open_t)GetProcAddress(hLibusb_dll, "usb_open");
  _usb_close 						= (usb_close_t)GetProcAddress(hLibusb_dll, "usb_close");
  _usb_get_string 					= (usb_get_string_t)GetProcAddress(hLibusb_dll, "usb_get_string");
  _usb_get_string_simple 			= (usb_get_string_simple_t)GetProcAddress(hLibusb_dll, "usb_get_string_simple");
  _usb_get_descriptor_by_endpoint 	= (usb_get_descriptor_by_endpoint_t)GetProcAddress(hLibusb_dll, "usb_get_descriptor_by_endpoint");
  _usb_get_descriptor 				= (usb_get_descriptor_t)GetProcAddress(hLibusb_dll, "usb_get_descriptor");
  _usb_bulk_write 					= (usb_bulk_write_t)GetProcAddress(hLibusb_dll, "usb_bulk_write");
  _usb_bulk_read 					= (usb_bulk_read_t)GetProcAddress(hLibusb_dll, "usb_bulk_read");
  _usb_interrupt_write 				= (usb_interrupt_write_t)GetProcAddress(hLibusb_dll, "usb_interrupt_write");
  _usb_interrupt_read 				= (usb_interrupt_read_t)GetProcAddress(hLibusb_dll, "usb_interrupt_read");
  _usb_control_msg 					= (usb_control_msg_t)GetProcAddress(hLibusb_dll, "usb_control_msg");
  _usb_set_configuration 			= (usb_set_configuration_t)GetProcAddress(hLibusb_dll, "usb_set_configuration");
  _usb_claim_interface 				= (usb_claim_interface_t)GetProcAddress(hLibusb_dll, "usb_claim_interface");
  _usb_release_interface 			= (usb_release_interface_t)GetProcAddress(hLibusb_dll, "usb_release_interface");
  _usb_set_altinterface 			= (usb_set_altinterface_t)GetProcAddress(hLibusb_dll, "usb_set_altinterface");
  _usb_resetep 						= (usb_resetep_t)GetProcAddress(hLibusb_dll, "usb_resetep");
  _usb_clear_halt 					= (usb_clear_halt_t)GetProcAddress(hLibusb_dll, "usb_clear_halt");
  _usb_reset 						= (usb_reset_t)GetProcAddress(hLibusb_dll, "usb_reset");
  _usb_reset_ex 					= (usb_reset_ex_t)GetProcAddress(hLibusb_dll, "usb_reset_ex");
  _usb_strerror 					= (usb_strerror_t)GetProcAddress(hLibusb_dll, "usb_strerror");
  _usb_init 						= (usb_init_t)GetProcAddress(hLibusb_dll, "usb_init");
  _usb_set_debug 					= (usb_set_debug_t)GetProcAddress(hLibusb_dll, "usb_set_debug");
  _usb_find_busses 					= (usb_find_busses_t)GetProcAddress(hLibusb_dll, "usb_find_busses");
  _usb_find_devices 				= (usb_find_devices_t)GetProcAddress(hLibusb_dll, "usb_find_devices");
  _usb_device 						= (usb_device_t)GetProcAddress(hLibusb_dll, "usb_device");
  _usb_get_busses 					= (usb_get_busses_t)GetProcAddress(hLibusb_dll, "usb_get_busses");
  _usb_install_service_np 			= (usb_install_service_np_t)GetProcAddress(hLibusb_dll, "usb_install_service_np");
  _usb_uninstall_service_np 		= (usb_uninstall_service_np_t)GetProcAddress(hLibusb_dll, "usb_uninstall_service_np");
  _usb_install_driver_np			= (usb_install_driver_np_t)GetProcAddress(hLibusb_dll, "usb_install_driver_np");
  _usb_get_version					= (usb_get_version_t)GetProcAddress(hLibusb_dll, "usb_get_version");
  _usb_isochronous_setup_async		= (usb_isochronous_setup_async_t)GetProcAddress(hLibusb_dll, "usb_isochronous_setup_async");
  _usb_bulk_setup_async 			= (usb_bulk_setup_async_t)GetProcAddress(hLibusb_dll, "usb_bulk_setup_async");
  _usb_interrupt_setup_async		= (usb_interrupt_setup_async_t)GetProcAddress(hLibusb_dll, "usb_interrupt_setup_async");
  _usb_submit_async 				= (usb_submit_async_t)GetProcAddress(hLibusb_dll, "usb_submit_async");
  _usb_reap_async			 		= (usb_reap_async_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_free_async 					= (usb_free_async_t)GetProcAddress(hLibusb_dll, "usb_free_async");
  
  _usb_install_service_np_rundll	= (usb_install_service_np_rundll_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_uninstall_service_np_rundll  = (usb_uninstall_service_np_rundll_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_install_driver_np_rundll     = (usb_install_driver_np_rundll_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_touch_inf_file_np_rundll     = (usb_touch_inf_file_np_rundll_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_touch_inf_file_np            = (usb_touch_inf_file_np_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_install_needs_restart_np		= (usb_install_needs_restart_np_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_install_npW				    = (usb_install_npW_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_install_npA                  = (usb_install_npA_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_install_np_rundll            = (usb_install_np_rundll_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_reap_async_nocancel          = (usb_reap_async_nocancel_t)GetProcAddress(hLibusb_dll, "usb_reap_async");
  _usb_cancel_async                 = (usb_cancel_async_t)GetProcAddress(hLibusb_dll, "usb_reap_async");

	if (_usb_init) _usb_init();
}

usb_dev_handle *usb_open (struct usb_device *dev)
{
	dbprintf("usb_open() VID:0x%.4X PID:0x%.4X\n",  dev->descriptor.idVendor, dev->descriptor.idProduct);
	
	if (_usb_open){
		usb_dev_handle *ret = _usb_open(dev);
		if (ret == NULL)
			dbprintf(" - usb_open() FAILED for VID:0x%X PID:0x%X\n",  dev->descriptor.idVendor, dev->descriptor.idProduct);
		return ret;
	}
	return NULL;
}

int usb_close (usb_dev_handle *dev)
{
	dbprintf("usb_close()\n");
	closeDebugPrint();
	
	if (_usb_close)
		return _usb_close(dev);
	else
		return -ENOFILE;
}

int usb_get_string (usb_dev_handle *dev, int index, int langid, char *buf, size_t buflen)
{
	dbprintf("usb_get_string() index:%i, langId:%i\n", index, langid);
	
	if (_usb_get_string){
		int len = _usb_get_string(dev, index, langid, buf, buflen);
		if (len > 0 && buflen && (*(wchar_t*)buf))
			dbwprintf(L" - usb_get_string() returned len:%i \"%s\"\n", len, (wchar_t*)buf);
		return len;
	}
	return -ENOFILE;
}

int usb_get_string_simple (usb_dev_handle *dev, int index, char *buf, size_t buflen)
{
	dbprintf("usb_get_string_simple() index:%i {", index);
	
	if (_usb_get_string_simple){
		int len = _usb_get_string_simple(dev, index, buf, buflen);
		dbprintf("len:%i \"",len);
		
		if (len > 0 && buflen /*&& *buf*/){
			for (int i = 0; i < 100 && i < buflen && i < len; i++){
				if ((int)(buf[i]&0xFF) > 31)
		  			dbprintf("%c", (int)(buf[i]&0xFF));
		  	}
		}
		dbprintf("\"}\n");
		return len;
	}
	return -ENOFILE;
}

int usb_get_descriptor_by_endpoint(usb_dev_handle *udev, int ep, unsigned char type, unsigned char index, void *buf, int size)
{
	dbprintf("usb_get_descriptor_by_endpoint() ep:0x%X type:%i index:%i size:%i\n", ep, type, index, size);
	
	if (_usb_get_descriptor_by_endpoint)
		return _usb_get_descriptor_by_endpoint(udev, ep, type, index, buf, size);
	else
		return -ENOFILE;
}

int usb_get_descriptor(usb_dev_handle *udev, unsigned char type, unsigned char index, void *buf, int size)
{
	dbprintf("usb_get_descriptor() type:%i index:%i size:%i\n", type, index, size);
	
	if (_usb_get_descriptor)
		return _usb_get_descriptor(udev, type, index, buf, size);
	else
		return -ENOFILE;
}

int usb_bulk_write (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout)
{
#if (_BANDWIDTH)
	total += size;
	
	unsigned int t1 = GetTickCount();
	double dt = t1 - t0;
	if (dt >= 1000.0){
		dt /= 1000.0;
		printf("usb_bulk_write(): %.0fk, dt:%.3fs\n", (total/dt)/1024.0, dt);
		total = 0;
		t0 = t1;
	}
#endif
	
	my_lock();
	dbprintf("usb_bulk_write() ep:0x%X size:%i timeout:%i {", ep, size, timeout);
	
#if (_DEBUG)
	  for (int i = 0; i < 100 && i < size; i++)
		  dbprintf("%X ", (int)(bytes[i]&0xFF));
#endif
	dbprintf("}\n");
	my_unlock();
	
	if (_usb_bulk_write)
		return _usb_bulk_write(dev, ep, bytes, size, timeout);
	else
		return -ENOFILE;
}

int usb_bulk_read (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout)
{
	my_lock();
	dbprintf("usb_bulk_read() ep:0x%X size:%i timeout:%i {", ep, size, timeout);
	
	if (_usb_bulk_read){
#if (_DEBUG)
		memset(bytes, 0, size);
    	const int ret = _usb_bulk_read(dev, ep, bytes, size, timeout);
    	dbprintf("ret:%i ", ret);
    	
		for (int i = 0; i < 100 && i < size && i < ret; i++)
			dbprintf(" %X", (int)(bytes[i]&0xFF));
		dbprintf("}\n");
		my_unlock();
		return ret;
#else
		dbprintf("}\n");
		my_unlock();
  		return _usb_bulk_read(dev, ep, bytes, size, timeout);
#endif
	}else{
		dbprintf("}\n");
		my_unlock();
		return -ENOFILE;
	}
}

int usb_interrupt_write (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout)
{
	my_lock();
	dbprintf("usb_interrupt_write() ep:0x%X size:%i timeout:%i {", ep, size, timeout);

#if (_DEBUG)
	for (int i = 0; i < 100 && i < size; i++)
		dbprintf("%X ", (int)(bytes[i]&0xFF));
#endif
	dbprintf("}\n");
	my_unlock();
	  	
	if (_usb_interrupt_write)
		return _usb_interrupt_write(dev, ep, bytes, size, timeout);
	else
		return -ENOFILE;
}

int usb_interrupt_read (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout)
{
	my_lock();
	dbprintf("usb_interrupt_read() ep:0x%X size:%i timeout:%i {", ep, size, timeout);
	
	if (_usb_interrupt_read){
#if (_DEBUG)
		memset(bytes, 0, size);
    	const int ret = _usb_interrupt_read(dev, ep, bytes, size, timeout);
    	dbprintf("ret:%i ", ret);
    	
		for (int i = 0; i < 100 && i < size && i < ret; i++)
			dbprintf(" %X", (int)(bytes[i]&0xFF));
		dbprintf("}\n");
		my_unlock();
		return ret;
#else
		dbprintf("}\n");
		my_unlock();
  		return _usb_interrupt_read(dev, ep, bytes, size, timeout);
#endif
	}else{
		dbprintf("}\n");
		my_unlock();
		return -ENOFILE;
	}
}

int usb_control_msg (usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, int size, int timeout)
{
	my_lock();
	if (size && bytes)
		dbprintf("usb_control_msg() 0x%X 0x%X/%i 0x%X/%i 0x%X/%i (size:%i)\n", requesttype, request, request, value, value, index, index, size);
	else
		dbprintf("usb_control_msg() 0x%X 0x%X/%i 0x%X/%i 0x%X/%i\n", requesttype, request, request, value, value, index, index);
	
#if (_DEBUG)
	//if (size && bytes && requesttype == 0xC0)
	//	memset(bytes, 0, size);

	if (size && bytes){
		dbprintf("->:%i (", size);
		for (int i = 0; i < 100 && i < size; i++)
			dbprintf(" %i", (int)(bytes[i]&0xFF));
		dbprintf(")\n");
	}

#endif
	
	if (_usb_control_msg){
		volatile int ret = _usb_control_msg(dev, requesttype, request, value, index, bytes, size, timeout);
    
#if (_DEBUG)
		if (size && bytes){
			dbprintf("ret:%i (", ret);
			for (int i = 0; i < 100 && i < size && i < ret; i++)
				dbprintf(" %i", (int)(bytes[i]&0xFF));
			dbprintf(")\n");
		}
#endif
		my_unlock();
		return ret;
	}else{
		my_unlock();
		return -ENOFILE;
	}
}

int usb_set_configuration(usb_dev_handle *dev, int configuration)
{
	dbprintf("usb_set_configuration() configuration:%i\n", configuration);
	
	if (_usb_set_configuration)
		return _usb_set_configuration(dev, configuration);
	else
		return -ENOFILE;
}

int usb_claim_interface(usb_dev_handle *dev, int interface)
{
	dbprintf("usb_claim_interface() interface:%i\n",interface);
	
	if (_usb_claim_interface)
		return _usb_claim_interface(dev, interface);
	else
		return -ENOFILE;
}

int usb_release_interface(usb_dev_handle *dev, int interface)
{
	dbprintf("usb_release_interface() interface:%i\n", interface);
	
	if (_usb_release_interface)
		return _usb_release_interface(dev, interface);
	else
		return -ENOFILE;
}

int usb_set_altinterface(usb_dev_handle *dev, int alternate)
{
	dbprintf("usb_set_altinterface() alternate:%i\n", alternate);
	
	if (_usb_set_altinterface)
		return _usb_set_altinterface(dev, alternate);
	else
		return -ENOFILE;
}

int usb_resetep(usb_dev_handle *dev, unsigned int ep)
{
	dbprintf("usb_resetep() ep:%i\n", ep);
	
	if (_usb_resetep)
		return _usb_resetep(dev, ep);
	else
		return -ENOFILE;
}

int usb_clear_halt(usb_dev_handle *dev, unsigned int ep)
{
	dbprintf("usb_clear_halt() ep:%i\n", ep);
	
	if (_usb_clear_halt)
		return _usb_clear_halt(dev, ep);
	else
		return -ENOFILE;
}

int usb_reset(usb_dev_handle *dev)
{
	dbprintf("usb_reset()\n");
	
	if (_usb_reset)
		return _usb_reset(dev);
	else
		return -ENOFILE;
}

int usb_reset_ex (usb_dev_handle *dev, unsigned int reset_type)
{
	dbprintf("usb_reset_ex()\n");
	
	if (_usb_reset_ex)
		return _usb_reset_ex(dev, reset_type);
	else
		return -ENOFILE;
}

char *usb_strerror(void)
{
	dbprintf("usb_strerror()\n");
	
	if (_usb_strerror)
		return _usb_strerror();
	else
		return NULL;
}

void usb_set_debug(int level)
{
	dbprintf("usb_set_debug() level:%i\n",level);
	
	if (_usb_set_debug)
		return _usb_set_debug(level);
}

int usb_find_busses(void)
{
	dbprintf("usb_find_busses()\n");
	
	if (_usb_find_busses)
		return _usb_find_busses();
	else
		return -ENOFILE;
}

int usb_find_devices(void)
{
	dbprintf("usb_find_devices()\n");
	
	if (_usb_find_devices)
		return _usb_find_devices();
	else
		return -ENOFILE;
}

struct usb_device *usb_device(usb_dev_handle *dev)
{
	dbprintf("usb_device()\n");
	
	if (_usb_device)
		return _usb_device(dev);
	else
		return NULL;
}

struct usb_bus *usb_get_busses(void)
{
	dbprintf("usb_get_busses()\n");
	
	if (_usb_get_busses)
		return _usb_get_busses();
	else
		return NULL;
}

int usb_install_service_np(void)
{
	dbprintf("usb_install_service_np()\n");
	
	if (_usb_install_service_np)
		return _usb_install_service_np();
	else
		return -ENOFILE;
}

int usb_uninstall_service_np(void)
{
	dbprintf("usb_uninstall_service_np()\n");
	
	if (_usb_uninstall_service_np)
		return _usb_uninstall_service_np();
	else
		return -ENOFILE;
}

int usb_install_driver_np(const char *inf_file)
{
	dbprintf("usb_install_driver_np() \"%s\"\n", inf_file);
	
	if (_usb_install_driver_np)
		return _usb_install_driver_np(inf_file);
	else
		return -ENOFILE;
}

const struct usb_version *usb_get_version(void)
{
	dbprintf("usb_get_version()\n");
	
	if (_usb_get_version)
		return _usb_get_version();
	else
		return NULL;
}

int usb_isochronous_setup_async(usb_dev_handle *dev, void **context, unsigned char ep, int pktsize)
{
	dbprintf("usb_isochronous_setup_async() ep:%i pktsize:%i\n", ep, pktsize);
	
	if (_usb_isochronous_setup_async)
		return _usb_isochronous_setup_async(dev, context, ep, pktsize);
	else
		return -ENOFILE;
}

int usb_bulk_setup_async(usb_dev_handle *dev, void **context, unsigned char ep)
{
	dbprintf("usb_bulk_setup_async() ep:%i \n",ep);
	
	if (_usb_bulk_setup_async)
		return _usb_bulk_setup_async(dev, context, ep);
	else
		return -ENOFILE;
}

int usb_interrupt_setup_async(usb_dev_handle *dev, void **context, unsigned char ep)
{
	dbprintf("usb_interrupt_setup_async() ep:%i\n",ep);
	
	if (_usb_interrupt_setup_async)
		return _usb_interrupt_setup_async(dev, context, ep);
	else
		return -ENOFILE;
}

int usb_submit_async(void *context, char *bytes, int size)
{
	dbprintf("usb_submit_async() size:%i\n", size);
	
	if (_usb_submit_async)
		return _usb_submit_async(context, bytes, size);
	else
		return -ENOFILE;
}

int usb_reap_async(void *context, int timeout)
{
	dbprintf("usb_reap_async() timeout:%i\n",timeout);
	
	if (_usb_reap_async)
		return _usb_reap_async(context, timeout);
	else
		return -ENOFILE;
}

int usb_free_async (void **context)
{
	dbprintf("usb_free_async()\n");
		
	if (_usb_free_async)
		return _usb_free_async(context);
	else
		return -ENOFILE;
}

int usb_cancel_async (void *context)
{
	dbprintf("usb_cancel_async()\n");
	
	if (_usb_cancel_async)
		return _usb_cancel_async(context);
	else
		return -ENOFILE;
}

int usb_reap_async_nocancel (void *context, int timeout)
{
	dbprintf("usb_reap_async_nocancel()\n");
	
	if (_usb_reap_async_nocancel)
		return _usb_reap_async_nocancel(context, timeout);
	else
		return -ENOFILE;
}

void CALLBACK usb_install_np_rundll (HWND wnd, HINSTANCE instance,  LPSTR cmd_line, int cmd_show)
{
	dbprintf("usb_install_np_rundll()\n");
	
	if (_usb_install_np_rundll)
		_usb_install_np_rundll(wnd, instance, cmd_line, cmd_show);
}

int usb_install_npW (HWND wnd, HINSTANCE instance, LPCWSTR cmd_line, int starg_arg)
{
	dbprintf("usb_install_npW()\n");
	
	if (_usb_install_npW)
		return _usb_install_npW(wnd, instance, cmd_line, starg_arg);
	else
		return -ENOFILE;
}

int usb_install_npA (HWND wnd, HINSTANCE instance, LPCSTR cmd_line, int starg_arg)
{
	dbprintf("usb_install_npA()\n");
	
	if (_usb_install_npA)
		return _usb_install_npA(wnd, instance, cmd_line, starg_arg);
	else
		return -ENOFILE;
}

int usb_install_needs_restart_np ()
{
	dbprintf("usb_install_needs_restart_np()\n");
	
	if (_usb_install_needs_restart_np)
		return _usb_install_needs_restart_np();
	else
		return -ENOFILE;
}

int usb_touch_inf_file_np (const char *inf_file)
{
	dbprintf("usb_touch_inf_file_np()\n");
	
	if (_usb_touch_inf_file_np)
		return _usb_touch_inf_file_np(inf_file);
	else
		return -ENOFILE;
}

void CALLBACK usb_touch_inf_file_np_rundll (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show)
{
	dbprintf("usb_touch_inf_file_np_rundll()\n");
	
	if (_usb_touch_inf_file_np_rundll)
		_usb_touch_inf_file_np_rundll(wnd, instance, cmd_line, cmd_show);
}

void CALLBACK usb_install_driver_np_rundll (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show)
{
	dbprintf("usb_install_driver_np_rundll()\n");
	
	if (_usb_install_driver_np_rundll)
		_usb_install_driver_np_rundll(wnd, instance, cmd_line, cmd_show);
}

void CALLBACK usb_uninstall_service_np_rundll (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show)
{
	dbprintf("usb_uninstall_service_np_rundll()\n");
	
	if (_usb_uninstall_service_np_rundll)
		_usb_uninstall_service_np_rundll(wnd, instance, cmd_line, cmd_show);
}

void CALLBACK usb_install_service_np_rundll (HWND wnd, HINSTANCE instance, LPSTR cmd_line, int cmd_show)
{
	dbprintf("usb_install_service_np_rundll()\n");
	
	if (_usb_install_service_np_rundll)
		_usb_install_service_np_rundll(wnd, instance, cmd_line, cmd_show);
}
