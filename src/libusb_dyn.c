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


#include "mylcd.h"

#if ((__BUILD_FTDI__) || (__BUILD_G15LIBUSB__) || (__BUILD_USB13700LIBUSB__) || (__BUILD_USBD480__) || (__BUILD_USBD480LIBUSB__))


#include <errno.h>
#include <lusb0_usb.h>
#include "libusb_dyn.h"

#ifndef ENOFILE
#define ENOFILE ENOENT
#endif

#define LIBUSB_DLL_NAME "libusb0.dll"


DLL_DECLARE(usb_dev_handle *, , usb_open, (struct usb_device *dev));
DLL_DECLARE(int, , usb_close, (usb_dev_handle *dev));
DLL_DECLARE(int, , usb_get_string, (usb_dev_handle *dev, int index, int langid, char *buf, size_t buflen));
DLL_DECLARE(int, , usb_get_string_simple, (usb_dev_handle *dev, int index,  char *buf, size_t buflen));
DLL_DECLARE(int, , usb_get_descriptor_by_endpoint, (usb_dev_handle *udev, int ep, unsigned char type,  unsigned char index, void *buf, int size)); DLL_DECLARE(int, , usb_get_descriptor, (usb_dev_handle *udev, unsigned char type, unsigned char index, void *buf, int size));
DLL_DECLARE(int, , usb_bulk_write, (usb_dev_handle *dev, int ep, char *bytes,  int size, int timeout));
DLL_DECLARE(int, , usb_bulk_read, (usb_dev_handle *dev, int ep, char *bytes,  int size, int timeout));
DLL_DECLARE(int, , usb_interrupt_write, (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout));
DLL_DECLARE(int, , usb_interrupt_read, (usb_dev_handle *dev, int ep, char *bytes, int size, int timeout));
DLL_DECLARE(int, , usb_control_msg, (usb_dev_handle *dev, int requesttype,  int request, int value, int index,  char *bytes, int size, int timeout));
DLL_DECLARE(int, , usb_set_configuration, (usb_dev_handle *dev, int configuration));
DLL_DECLARE(int, , usb_claim_interface, (usb_dev_handle *dev, int interface));
DLL_DECLARE(int, , usb_release_interface, (usb_dev_handle *dev, int interface));
DLL_DECLARE(int, , usb_set_altinterface, (usb_dev_handle *dev, int alternate));
DLL_DECLARE(int, , usb_resetep, (usb_dev_handle *dev, unsigned int ep));
DLL_DECLARE(int, , usb_clear_halt, (usb_dev_handle *dev, unsigned int ep));
DLL_DECLARE(int, , usb_reset, (usb_dev_handle *dev));
DLL_DECLARE(char *, ,usb_strerror, (void));
DLL_DECLARE(void, ,usb_init, (void));
DLL_DECLARE(void, ,usb_set_debug, (int level));
DLL_DECLARE(int, , usb_find_busses, (void));
DLL_DECLARE(int, , usb_find_devices, (void));
DLL_DECLARE(struct usb_device *, , usb_device, (usb_dev_handle *dev));
DLL_DECLARE(struct usb_bus *, , usb_get_busses, (void));
DLL_DECLARE(int, , usb_install_service_np, (void));
DLL_DECLARE(int, , usb_uninstall_service_np, (void));
DLL_DECLARE(int, , usb_install_driver_np, (const char *inf_file));
DLL_DECLARE(const struct usb_version *, , usb_get_version, (void));
DLL_DECLARE(int, , usb_isochronous_setup_async, (usb_dev_handle *dev,  void **context, unsigned char ep, int pktsize));
DLL_DECLARE(int, , usb_bulk_setup_async, (usb_dev_handle *dev, void **context, unsigned char ep));
DLL_DECLARE(int, , usb_interrupt_setup_async, (usb_dev_handle *dev, void **context, unsigned char ep));
DLL_DECLARE(int, , usb_submit_async, (void *context, char *bytes, int size));
DLL_DECLARE(int, , usb_reap_async, (void *context, int timeout));
DLL_DECLARE(int, , usb_free_async, (void **context));


static void dll_load_error (const char *fn, const char *dll, const int err)
{
	if (err == -1)
		printf("libmylcd: '%s' not found\n", dll);
	else if (err == -2)
		printf("libmylcd: symbol '%s' not found in '%s'\n", fn, dll);
}

int loadLibDLL (const char *lib)
{
	DLL_LOAD(lib, usb_open, 1);
	DLL_LOAD(lib, usb_close, 1);
	DLL_LOAD(lib, usb_get_string, 1);
	DLL_LOAD(lib, usb_get_string_simple, 1);
	DLL_LOAD(lib, usb_get_descriptor_by_endpoint, 1);
	DLL_LOAD(lib, usb_bulk_write, 1);
	DLL_LOAD(lib, usb_bulk_read, 1);
	DLL_LOAD(lib, usb_interrupt_write, 1);
	DLL_LOAD(lib, usb_interrupt_read, 1);
	DLL_LOAD(lib, usb_control_msg, 1);
	DLL_LOAD(lib, usb_set_configuration, 1);
	DLL_LOAD(lib, usb_claim_interface, 1);
	DLL_LOAD(lib, usb_release_interface, 1);
	DLL_LOAD(lib, usb_set_altinterface, 1);
	DLL_LOAD(lib, usb_resetep, 1);
	DLL_LOAD(lib, usb_clear_halt, 1);
	DLL_LOAD(lib, usb_reset, 1);
	DLL_LOAD(lib, usb_strerror, 1);
	DLL_LOAD(lib, usb_init, 1);
	DLL_LOAD(lib, usb_set_debug, 1);
	DLL_LOAD(lib, usb_find_busses, 1);
	DLL_LOAD(lib, usb_find_devices, 1);
	DLL_LOAD(lib, usb_device, 1);
	DLL_LOAD(lib, usb_get_busses, 1);
	DLL_LOAD(lib, usb_install_service_np, 1);
	DLL_LOAD(lib, usb_uninstall_service_np, 1);
	DLL_LOAD(lib, usb_install_driver_np, 1);
	DLL_LOAD(lib, usb_get_version, 1);
	DLL_LOAD(lib, usb_isochronous_setup_async, 1);
	DLL_LOAD(lib, usb_bulk_setup_async, 1);
	DLL_LOAD(lib, usb_interrupt_setup_async, 1);
	DLL_LOAD(lib, usb_submit_async, 1);
	DLL_LOAD(lib, usb_reap_async, 1);
	DLL_LOAD(lib, usb_free_async, 1);
	
	return 1;
}

void usb_init (void)
{
	if (loadLibDLL(LIBUSB_DLL_NAME) == 1){
		if (_usb_init){
			_usb_init();
			return;
		}
	}
	return;
}

usb_dev_handle *usb_open (struct usb_device *dev)
{
  if (_usb_open)
    return _usb_open(dev);
  else
    return NULL;
}

int usb_close (usb_dev_handle *dev)
{
  if (_usb_close)
    return _usb_close(dev);
  else
    return -ENOFILE;
}

int usb_get_string (usb_dev_handle *dev, int index, int langid, char *buf,
                   size_t buflen)
{
  if (_usb_get_string)
    return _usb_get_string(dev, index, langid, buf, buflen);
  else
    return -ENOFILE;
}

int usb_get_string_simple(usb_dev_handle *dev, int index, char *buf,
                          size_t buflen)
{
  if (_usb_get_string_simple)
    return _usb_get_string_simple(dev, index, buf, buflen);
  else
    return -ENOFILE;
}

int usb_get_descriptor_by_endpoint(usb_dev_handle *udev, int ep,
                                   unsigned char type, unsigned char index,
                                   void *buf, int size)
{
  if (_usb_get_descriptor_by_endpoint)
    return _usb_get_descriptor_by_endpoint(udev, ep, type, index, buf, size);
  else
    return -ENOFILE;
}

int usb_get_descriptor(usb_dev_handle *udev, unsigned char type,
                       unsigned char index, void *buf, int size)
{
  if (_usb_get_descriptor)
    return _usb_get_descriptor(udev, type, index, buf, size);
  else
    return -ENOFILE;
}

int usb_bulk_write(usb_dev_handle *dev, int ep, char *bytes, int size,
                   int timeout)
{
  if (_usb_bulk_write)
    return _usb_bulk_write(dev, ep, bytes, size, timeout);
  else
    return -ENOFILE;
}

int usb_bulk_read(usb_dev_handle *dev, int ep, char *bytes, int size,
                  int timeout)
{
  if (_usb_bulk_read)
    return _usb_bulk_read(dev, ep, bytes, size, timeout);
  else
    return -ENOFILE;
}

int usb_interrupt_write(usb_dev_handle *dev, int ep, char *bytes, int size,
                        int timeout)
{
  if (_usb_interrupt_write)
    return _usb_interrupt_write(dev, ep, bytes, size, timeout);
  else
    return -ENOFILE;
}

int usb_interrupt_read(usb_dev_handle *dev, int ep, char *bytes, int size,
                       int timeout)
{
  if (_usb_interrupt_read)
    return _usb_interrupt_read(dev, ep, bytes, size, timeout);
  else
    return -ENOFILE;
}

int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
                    int value, int index, char *bytes, int size, 
                    int timeout)
{
  if (_usb_control_msg)
    return _usb_control_msg(dev, requesttype, request, value, index, bytes, 
                            size, timeout);
  else
    return -ENOFILE;
}

int usb_set_configuration(usb_dev_handle *dev, int configuration)
{
  if (_usb_set_configuration)
    return _usb_set_configuration(dev, configuration);
  else
    return -ENOFILE;
}

int usb_claim_interface(usb_dev_handle *dev, int interface)
{
  if (_usb_claim_interface)
    return _usb_claim_interface(dev, interface);
  else
    return -ENOFILE;
}

int usb_release_interface(usb_dev_handle *dev, int interface)
{
  if (_usb_release_interface)
    return _usb_release_interface(dev, interface);
  else
    return -ENOFILE;
}

int usb_set_altinterface(usb_dev_handle *dev, int alternate)
{
  if (_usb_set_altinterface)
    return _usb_set_altinterface(dev, alternate);
  else
    return -ENOFILE;
}

int usb_resetep(usb_dev_handle *dev, unsigned int ep)
{
  if (_usb_resetep)
    return _usb_resetep(dev, ep);
  else
    return -ENOFILE;
}

int usb_clear_halt(usb_dev_handle *dev, unsigned int ep)
{
  if (_usb_clear_halt)
    return _usb_clear_halt(dev, ep);
  else
    return -ENOFILE;
}

int usb_reset(usb_dev_handle *dev)
{
  if (_usb_reset)
    return _usb_reset(dev);
  else
    return -ENOFILE;
}

char *usb_strerror(void)
{
  if (_usb_strerror)
    return _usb_strerror();
  else
    return NULL;
}

void usb_set_debug(int level)
{
  if (_usb_set_debug)
    return _usb_set_debug(level);
}

int usb_find_busses(void)
{
  if (_usb_find_busses)
    return _usb_find_busses();
  else
    return -ENOFILE;
}

int usb_find_devices(void)
{
  if (_usb_find_devices)
    return _usb_find_devices();
  else
    return -ENOFILE;
}

struct usb_device *usb_device(usb_dev_handle *dev)
{
  if (_usb_device)
    return _usb_device(dev);
  else
    return NULL;
}

struct usb_bus *usb_get_busses(void)
{
  if (_usb_get_busses)
    return _usb_get_busses();
  else
    return NULL;
}

int usb_install_service_np(void)
{
  if (_usb_install_service_np)
    return _usb_install_service_np();
  else
    return -ENOFILE;
}

int usb_uninstall_service_np(void)
{
  if (_usb_uninstall_service_np)
    return _usb_uninstall_service_np();
  else
    return -ENOFILE;
}

int usb_install_driver_np(const char *inf_file)
{
  if (_usb_install_driver_np)
    return _usb_install_driver_np(inf_file);
  else
    return -ENOFILE;
}

const struct usb_version *usb_get_version(void)
{
  if (_usb_get_version)
    return _usb_get_version();
  else
    return NULL;
}

int usb_isochronous_setup_async(usb_dev_handle *dev, void **context,
                                unsigned char ep, int pktsize)
{
  if (_usb_isochronous_setup_async)
    return _usb_isochronous_setup_async(dev, context, ep, pktsize);
  else
    return -ENOFILE;
}

int usb_bulk_setup_async(usb_dev_handle *dev, void **context,
                         unsigned char ep)
{
  if (_usb_bulk_setup_async)
    return _usb_bulk_setup_async(dev, context, ep);
  else
    return -ENOFILE;
}

int usb_interrupt_setup_async(usb_dev_handle *dev, void **context,
                              unsigned char ep)
{
  if (_usb_interrupt_setup_async)
    return _usb_interrupt_setup_async(dev, context, ep);
  else
    return -ENOFILE;
}

int usb_submit_async(void *context, char *bytes, int size)
{
  if (_usb_submit_async)
    return _usb_submit_async(context, bytes, size);
  else
    return -ENOFILE;
}

int usb_reap_async(void *context, int timeout)
{
  if (_usb_reap_async)
    return _usb_reap_async(context, timeout);
  else
    return -ENOFILE;
}

int usb_free_async(void **context)
{
  if (_usb_free_async)
    return _usb_free_async(context);
  else
    return -ENOFILE;
}

#endif
