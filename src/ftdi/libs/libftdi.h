/***************************************************************************
                          ftdi.h  -  description
                             -------------------
    begin                : Fri Apr 4 2003
    copyright            : (C) 2003 by Intra2net AG
    email                : opensource@intra2net.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation;             *
 *                                                                         *
 ***************************************************************************/

#ifndef __libftdi_h__
#define __libftdi_h__

#include <lusb0_usb.h>

#define FTDI_DEFAULT_EEPROM_SIZE 128

/// FTDI chip type
enum ftdi_chip_type { TYPE_AM=0, TYPE_BM=1, TYPE_2232C=2, TYPE_R=3 };
/// Parity mode for ftdi_set_line_property()
enum ftdi_parity_type { NONE=0, ODD=1, EVEN=2, MARK=3, SPACE=4 };
/// Number of stop bits for ftdi_set_line_property()
enum ftdi_stopbits_type { STOP_BIT_1=0, STOP_BIT_15=1, STOP_BIT_2=2 };
/// Number of bits ftdi_set_line_property()
enum ftdi_bits_type { BITS_7=7, BITS_8=8 };

/// MPSSE bitbang modes
enum ftdi_mpsse_mode {
    BITMODE_RESET  = 0x00,
    BITMODE_BITBANG= 0x01,
    BITMODE_MPSSE  = 0x02,
    BITMODE_SYNCBB = 0x04,
    BITMODE_MCU    = 0x08,
    // CPU-style fifo mode gets set via EEPROM
    BITMODE_OPTO   = 0x10,
    BITMODE_CBUS   = 0x20
};

/// Port interface for FT2232C
enum ftdi_interface {
    INTERFACE_ANY = 0,
    INTERFACE_A   = 1,
    INTERFACE_B   = 2
};

/* Shifting commands IN MPSSE Mode*/
#define MPSSE_WRITE_NEG 0x01   /* Write TDI/DO on negative TCK/SK edge*/
#define MPSSE_BITMODE   0x02   /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04   /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08   /* LSB first */
#define MPSSE_DO_WRITE  0x10   /* Write TDI/DO */
#define MPSSE_DO_READ   0x20   /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40   /* Write TMS/CS */

/* FTDI MPSSE commands */
#define SET_BITS_LOW   0x80
/*BYTE DATA*/
/*BYTE Direction*/
#define SET_BITS_HIGH  0x82
/*BYTE DATA*/
/*BYTE Direction*/
#define GET_BITS_LOW   0x81
#define GET_BITS_HIGH  0x83
#define LOOPBACK_START 0x84
#define LOOPBACK_END   0x85
#define TCK_DIVISOR    0x86
/* Value Low */
/* Value HIGH */ /*rate is 12000000/((1+value)*2) */
#define DIV_VALUE(rate) (rate > 6000000)?0:((6000000/rate -1) > 0xffff)? 0xffff: (6000000/rate -1)

/* Commands in MPSSE and Host Emulation Mode */
#define SEND_IMMEDIATE 0x87 
#define WAIT_ON_HIGH   0x88
#define WAIT_ON_LOW    0x89

/* Commands in Host Emulation Mode */
#define READ_SHORT     0x90
/* Address_Low */
#define READ_EXTENDED  0x91
/* Address High */
/* Address Low  */
#define WRITE_SHORT    0x92
/* Address_Low */
#define WRITE_EXTENDED 0x93
/* Address High */
/* Address Low  */

/* Definitions for flow control */
#define SIO_MODEM_CTRL     1 /* Set the modem control register */
#define SIO_SET_FLOW_CTRL  2 /* Set flow control register */

#define SIO_SET_FLOW_CTRL_REQUEST_TYPE 0x40
#define SIO_SET_FLOW_CTRL_REQUEST SIO_SET_FLOW_CTRL

#define SIO_DISABLE_FLOW_CTRL 0x0 
#define SIO_RTS_CTS_HS (0x1 << 8)
#define SIO_DTR_DSR_HS (0x2 << 8)
#define SIO_XON_XOFF_HS (0x4 << 8)

#define SIO_SET_MODEM_CTRL_REQUEST_TYPE 0x40
#define SIO_SET_MODEM_CTRL_REQUEST SIO_MODEM_CTRL

#define SIO_SET_DTR_MASK 0x1
#define SIO_SET_DTR_HIGH ( 1 | ( SIO_SET_DTR_MASK  << 8))
#define SIO_SET_DTR_LOW  ( 0 | ( SIO_SET_DTR_MASK  << 8))
#define SIO_SET_RTS_MASK 0x2
#define SIO_SET_RTS_HIGH ( 2 | ( SIO_SET_RTS_MASK << 8 ))
#define SIO_SET_RTS_LOW ( 0 | ( SIO_SET_RTS_MASK << 8 ))

#define SIO_RTS_CTS_HS (0x1 << 8)

/* marker for unused usb urb structures
   (taken from libusb) */
#define FTDI_URB_USERCONTEXT_COOKIE ((void *)0x1)

/**
    \brief Main context structure for all libftdi functions.

    Do not access directly if possible.
*/
struct ftdi_context {
    // USB specific
    /// libusb's usb_dev_handle
    struct usb_dev_handle *usb_dev;
    /// usb read timeout
    int usb_read_timeout;
    /// usb write timeout
    int usb_write_timeout;

    // FTDI specific
    /// FTDI chip type
    enum ftdi_chip_type type;
    /// baudrate
    int baudrate;
    /// bitbang mode state
    unsigned char bitbang_enabled;
    /// pointer to read buffer for ftdi_read_data
    unsigned char *readbuffer;
    /// read buffer offset
    unsigned int readbuffer_offset;
    /// number of remaining data in internal read buffer
    unsigned int readbuffer_remaining;
    /// read buffer chunk size
    unsigned int readbuffer_chunksize;
    /// write buffer chunk size
    unsigned int writebuffer_chunksize;

    // FTDI FT2232C requirecments
    /// FT2232C interface number: 0 or 1
    int interface;   // 0 or 1
    /// FT2232C index number: 1 or 2
    int index;       // 1 or 2
    // Endpoints
    /// FT2232C end points: 1 or 2
    int in_ep;
    int out_ep;      // 1 or 2

    /// Bitbang mode. 1: (default) Normal bitbang mode, 2: FT2232C SPI bitbang mode
    unsigned char bitbang_mode;

    /// EEPROM size. Default is 128 bytes for 232BM and 245BM chips
    int eeprom_size;

    /// String representation of last error
    char *error_str;

    /// Buffer needed for async communication
    char *async_usb_buffer;
    /// Number of URB-structures we can buffer
    unsigned int async_usb_buffer_size;
};

/**
    \brief list of usb devices created by ftdi_usb_find_all()
*/
struct ftdi_device_list {
    /// pointer to next entry
    struct ftdi_device_list *next;
    /// pointer to libusb's usb_device
    struct usb_device *dev;
};

/**
    \brief FTDI eeprom structure
*/
struct ftdi_eeprom {
    /// vendor id
    int vendor_id;
    /// product id
    int product_id;

    /// self powered
    int self_powered;
    /// remote wakepu
    int remote_wakeup;
    /// chip type
    int BM_type_chip;

    /// input in isochronous transfer mode
    int in_is_isochronous;
    /// output in isochronous transfer mode
    int out_is_isochronous;
    /// suspend pull downs
    int suspend_pull_downs;

    /// use serial
    int use_serial;
    /// fake usb version
    int change_usb_version;
    /// usb version
    int usb_version;
    /// maximum power
    int max_power;

    /// manufacturer name
    char *manufacturer;
    /// product name
    char *product;
    /// serial number
    char *serial;

  /// eeprom size in bytes. This doesn't get stored in the eeprom
  /// but is the only way to pass it to ftdi_eeprom_build.
  int size;
};

#ifdef __cplusplus
extern "C" {
#endif

    int ftdi_init(struct ftdi_context *ftdi);
    struct ftdi_context *ftdi_new();
    int ftdi_set_interface(struct ftdi_context *ftdi, enum ftdi_interface interface);

    void ftdi_deinit(struct ftdi_context *ftdi);
    void ftdi_free(struct ftdi_context *ftdi);
    void ftdi_set_usbdev (struct ftdi_context *ftdi, usb_dev_handle *usbdev);

    int ftdi_usb_find_all(struct ftdi_context *ftdi, struct ftdi_device_list **devlist,
                          int vendor, int product);
    void ftdi_list_free(struct ftdi_device_list **devlist);
    void ftdi_list_free2(struct ftdi_device_list *devlist);
    int ftdi_usb_get_strings(struct ftdi_context *ftdi, struct usb_device *dev,
                             char * manufacturer, int mnf_len,
                             char * description, int desc_len,
                             char * serial, int serial_len);

    int ftdi_usb_open(struct ftdi_context *ftdi, int vendor, int product);
    int ftdi_usb_open_desc(struct ftdi_context *ftdi, int vendor, int product,
                           const char* description, const char* serial);
    int ftdi_usb_open_dev(struct ftdi_context *ftdi, struct usb_device *dev);

    int ftdi_usb_close(struct ftdi_context *ftdi);
    int ftdi_usb_reset(struct ftdi_context *ftdi);
    int ftdi_usb_purge_rx_buffer(struct ftdi_context *ftdi);
    int ftdi_usb_purge_tx_buffer(struct ftdi_context *ftdi);
    int ftdi_usb_purge_buffers(struct ftdi_context *ftdi);

    int ftdi_set_baudrate(struct ftdi_context *ftdi, int baudrate);
    int ftdi_set_line_property(struct ftdi_context *ftdi, enum ftdi_bits_type bits,
                               enum ftdi_stopbits_type sbit, enum ftdi_parity_type parity);

    int ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, unsigned int size);
    int ftdi_read_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize);
    int ftdi_read_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize);

    int ftdi_write_data(struct ftdi_context *ftdi, unsigned char *buf, unsigned int size);
    int ftdi_write_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize);
    int ftdi_write_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize);

    int ftdi_write_data_async(struct ftdi_context *ftdi, unsigned char *buf, unsigned int size);
    void ftdi_async_complete(struct ftdi_context *ftdi, int wait_for_more);

    int ftdi_enable_bitbang(struct ftdi_context *ftdi, unsigned char bitmask);
    int ftdi_disable_bitbang(struct ftdi_context *ftdi);
    int ftdi_set_bitmode(struct ftdi_context *ftdi, unsigned char bitmask, unsigned char mode);
    int ftdi_read_pins(struct ftdi_context *ftdi, unsigned char *pins);

    int ftdi_set_latency_timer(struct ftdi_context *ftdi, unsigned char latency);
    int ftdi_get_latency_timer(struct ftdi_context *ftdi, unsigned char *latency);

    int ftdi_poll_modem_status(struct ftdi_context *ftdi, unsigned short *status);

    int ftdi_set_event_char(struct ftdi_context *ftdi, unsigned char eventch, unsigned char enable);
    int ftdi_set_error_char(struct ftdi_context *ftdi, unsigned char errorch, unsigned char enable);

    // set eeprom size
    void ftdi_eeprom_setsize(struct ftdi_context *ftdi, struct ftdi_eeprom *eeprom, int size);

    // init and build eeprom from ftdi_eeprom structure
    void ftdi_eeprom_initdefaults(struct ftdi_eeprom *eeprom);
    int  ftdi_eeprom_build(struct ftdi_eeprom *eeprom, unsigned char *output);

    // "eeprom" needs to be valid 128 byte eeprom (generated by the eeprom generator)
    // the checksum of the eeprom is valided
    int ftdi_read_eeprom(struct ftdi_context *ftdi, unsigned char *eeprom);
    int ftdi_read_chipid(struct ftdi_context *ftdi, unsigned int *chipid);
    int ftdi_read_eeprom_getsize(struct ftdi_context *ftdi, unsigned char *eeprom, int maxsize);
    int ftdi_write_eeprom(struct ftdi_context *ftdi, unsigned char *eeprom);
    int ftdi_erase_eeprom(struct ftdi_context *ftdi);

    char *ftdi_get_error_string(struct ftdi_context *ftdi);

    // flow control
    int ftdi_setflowctrl(struct ftdi_context *ftdi, int flowctrl);
    int ftdi_setdtr(struct ftdi_context *ftdi, int state);
    int ftdi_setrts(struct ftdi_context *ftdi, int state);

#ifdef __cplusplus
}
#endif

#endif /* __libftdi_h__ */
