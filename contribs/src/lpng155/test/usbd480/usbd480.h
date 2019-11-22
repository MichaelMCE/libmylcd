

#ifndef _USBD480_H_
#define _USBD480_H_


#define USBD480_VID						0x16C0
#define USBD480_PID						0x08A6

#define USBD480_SET_ADDRESS				0xC0 // set SDRAM address
#define USBD480_WRITE_DATA				0xC1 // write one 16-bit word
#define USBD480_SET_FRAME_START_ADDRESS	0xC4 // set frame start address
#define USBD480_SET_STREAM_DECODER		0xC6
#define USBD480_TOUCH_READ				0xE1
#define USBD480_TOUCH_MODE				0xE2
#define USBD480_GET_DEVICE_DETAILS		0x80
#define USBD480_SET_CONFIG_VALUE		0x82
#define USBD480_GET_CONFIG_VALUE		0x83
#define USBD480_SAVE_CONFIG				0x84 // write config to flash
#define USBD480_GET_SAVED_CONFIG_VALUE  0x86
#define USBD480_EEPROM_PREPARE_WRITE	0xBC
#define USBD480_READ_PARAM_STORE		0xBF

#define CFG_TOUCH_MODE					2
#define CFG_TOUCH_DEBOUNCE_VALUE		3	// default 50 - 0 to 254
#define CFG_TOUCH_SKIP_SAMPLES			4	// default 40
#define CFG_TOUCH_PRESSURE_LIMIT_LO		5	// default 30
#define CFG_TOUCH_PRESSURE_LIMIT_HI		6	// default 120
#define CFG_BACKLIGHT_BRIGHTNESS		20	// 0 - 255, default 255
#define CFG_USB_ENUMERATION_MODE		22	// 0 - 1, default is 0. 0:HID device, 1:vender specific device

#define DEVICE_NAMELENGTH				20
#define DEVICE_SERIALLENGTH				10

#define DEBOUNCETIME					80		/* millisecond period*/
#define DRAGDEBOUNCETIME				50



#define USBH_DESCRIPTORTYPE_DEVICE				0x01
#define USBH_DESCRIPTORTYPE_CONFIG				0x02
#define USBH_DESCRIPTORTYPE_STRING				0x03
#define USBH_DESCRIPTORTYPE_INTERFACE			0x04
#define USBH_DESCRIPTORTYPE_ENDPOINT			0x05
#define USBH_DESCRIPTORTYPE_HID					0x21
#define USBH_DESCRIPTORTYPE_REPORT				0x22
#define USBH_DESCRIPTORTYPE_PHYSICAL			0x23
#define USBH_DESCRIPTORTYPE_HUB					0x29



#define USBH_DEVICECLASS_UNSPECIFIED			0x00	// Device class is unspecified, use class information in the Interface Descriptors
#define USBH_DEVICECLASS_AUDIO					0x01	// speaker, microphone, sound card, MIDI
#define USBH_DEVICECLASS_CDC					0x02	// modem, Ethernet adapter, Wi-Fi adapter, RS232 serial adapter. Used together with class 0Ah (below)
#define USBH_DEVICECLASS_HID					0x03	// keyboard, mouse, joystick, touch device, etc..
//#define USBH_DEVICECLASS_						0x04
#define USBH_DEVICECLASS_PID					0x05	// Physical Interface Device, eg; Force feedback joystick
#define USBH_DEVICECLASS_IMAGE					0x06	// PTP/MTP, webcam, scanner
#define USBH_DEVICECLASS_PRINTER				0x07
#define USBH_DEVICECLASS_MASSSTORAGE			0x08	// MSC or UMS. USB flash drive, memory card reader, digital audio player, digital camera, external drive
#define USBH_DEVICECLASS_USBHUB					0x09
#define USBH_DEVICECLASS_CDCDATA				0x0A
#define USBH_DEVICECLASS_VIDEO					0x0E	// webcam
#define USBH_DEVICECLASS_PHDC					0x0F	// Personal Healthcare Device Class. eg; Pulse monitor (watch)
#define USBH_DEVICECLASS_AUDIOVIDIO				0x10
#define USBH_DEVICECLASS_WIRELESSCONTROLLER		0xE0
#define USBH_DEVICECLASS_MISC					0xEF
#define USBH_DEVICECLASS_APPLICATION			0xFE
#define USBH_DEVICECLASS_VENDOR					0xFF


#define USBH_DEVICESUBCLASS_MIDI				0x03

#define USBH_ENDPOINTTYPE_CONTROL				0x00
#define USBH_ENDPOINTTYPE_ISOCHRONOUS			0x01
#define USBH_ENDPOINTTYPE_BULK					0x02
#define USBH_ENDPOINTTYPE_INTERRUPT				0x03



// LFRM_BPP_16 - RGB 565
#define RGB_16_RED		0xF800
#define RGB_16_GREEN	0x07E0
#define RGB_16_BLUE		0x001F	
#define RGB_16_WHITE	(RGB_16_RED|RGB_16_GREEN|RGB_16_BLUE)
#define RGB_16_BLACK	0x0000
#define RGB_16_MAGENTA	(RGB_16_RED|RGB_16_BLUE)
#define RGB_16_YELLOW	(RGB_16_RED|RGB_16_GREEN)
#define RGB_16_CYAN		(RGB_16_GREEN|RGB_16_BLUE)






class USBD480Display : public USBDriver {
public:
	enum { SYSEX_MAX_LEN = 60 };
	
	USBD480Display (USBHost &host)
	{
		init();
	}
	USBD480Display (USBHost *host)
	{
		init();
	}

	
	void setCallbackFunc (int (*func)(uint32_t msg, intptr_t *value1, uint32_t value2))
	{
		callbackFunc = func;
	}

	struct {
		int width;
		int height;
		int pitch;
	}display;
		
	int setBaseAddress (const uint32_t addr);
	int setFrameAddress (const uint32_t addr);
	int setWriteAddress (const uint32_t addr);
	int setConfigValue (const uint32_t cfg, const uint8_t value);
	int getConfigValue (const uint32_t cfg, uint8_t *value);
	int setBrightness (const uint8_t level);
	int setLineLength (uint32_t length);
	int disableStreamDecoder ();
	int enableStreamDecoder ();
	int setTouchMode (const uint32_t mode);
	int setConfigDefaults ();
	int getDeviceDetails (char *name, uint32_t *width, uint32_t *height, uint32_t *version, char *serial);
	int writeData (const void *data, const size_t size);
	int drawScreenArea (uint8_t *__restrict__ buffer, const int32_t __restrict__ bwidth, const int32_t __restrict__ bheight, const int32_t __restrict__ dx, const int32_t __restrict__ dy, uint32_t frameAddrOffset = 0, int swap = 0);
	int fillScreen (const uint16_t colour);
	
	//int drawScreenArea2 (uint8_t *buffer, const int32_t bwidth, const int32_t bheight);
	//int ustate = 0;
	
	int drawScreenSetup (const int32_t bwidth, const int32_t bheight, const int32_t dx, const int32_t dy, uint32_t frameAddrOffset);
	
	tx_descript_t tx_writeCtx;
	
protected:

	virtual void driverReady ();

	virtual void Task ();
	virtual bool claim (Device_t *device, int type, const uint8_t *descriptors, uint32_t len);
	virtual void control (const Transfer_t *transfer);
	virtual void disconnect ();
	
	//static void tx_callback (const Transfer_t *transfer);	
	//static void rx_callback (const Transfer_t *transfer);
	
	//void rx_data(const Transfer_t *transfer);
	//void tx_data(const Transfer_t *transfer);
	void init();

private:



	Device_t *device;
	Pipe_t *rxpipe;
	Pipe_t *txpipe;
	
	uint32_t start = 0;
	uint32_t lineLength = 0;
	
	uint8_t pixbuffer[(480*2)+8];
	
/*	
	enum {
		MAX_TX_PACKET_SIZE = 512, 
		MAX_RX_PACKET_SIZE = 16
	};
	
	enum { RX_QUEUE_SIZE = 40 }; // must be more than MAX_PACKET_SIZE/4
*/
	//uint32_t tx_buffer[MAX_TX_PACKET_SIZE/sizeof(uint32_t)];
	uint16_t tx_size;
	uint16_t rx_size;
	uint8_t tx_ep;
	uint8_t rx_ep;			
	//uint32_t rx_buffer[MAX_RX_PACKET_SIZE/sizeof(uint32_t)];
	//uint32_t rx_queue[RX_QUEUE_SIZE];

	//uint16_t rx_head;
	//uint16_t rx_tail;
	//bool rx_packet_queued;

	Pipe_t mypipes[4] __attribute__ ((aligned(32)));
	Transfer_t mytransfers[136] __attribute__ ((aligned(32)));
	int (*callbackFunc) (uint32_t msg, intptr_t *value1, uint32_t value2);
};




typedef struct {
	uint8_t bLength;				// Size of Descriptor in Bytes (9 Bytes)
	uint8_t bDescriptorType;		// this Descriptotion type (eg; 0x04 == interface)
	
	uint8_t bInterfaceNumber;		// Number of Interface
	uint8_t bAlternateSetting;		// Value used to select alternative setting
	uint8_t bNumEndpoints;			// Number of Endpoints used for this interface
	uint8_t bInterfaceClass;		// Class Code (Assigned by USB Org)
	uint8_t bInterfaceSubClass;		// Subclass Code (Assigned by USB Org)
	uint8_t bInterfaceProtocol;		// Protocol Code (Assigned by USB Org)
	uint8_t iInterface;				// Index of String Descriptor Describing this interface
}__attribute__((packed)) descriptor_interface_t;


typedef struct {
	uint8_t bLength;				// Size of Descriptor in Bytes (9 Bytes)
	uint8_t bDescriptorType;		// this Descriptotion type (eg; 0x04 == interface)
	
	uint8_t bEndpointAddress;		// Endpoint Address
									// Bits 0..3b Endpoint Number.
									// Bits 4..6b Reserved. Set to Zero
									// Bits 7 Direction 0 = Out, 1 = In (Ignored for Control Endpoints)
									
	uint8_t bmAttributes;			// Bits 0..1 Transfer Type
									//   00 = Control, 01 = Isochronous, 10 = Bulk, 11 = Interrupt
									// Bits 2..7 are reserved. If Isochronous endpoint, 
									// Bits 3..2 = Synchronisation Type (Iso Mode)
									//   00 = No Synchonisation, 01 = Asynchronous, 10 = Adaptive, 11 = Synchronous
									// Bits 5..4 = Usage Type (Iso Mode) 
									//   00 = Data Endpoint, 01 = Feedback Endpoint, 10 = Explicit Feedback Data Endpoint, 11 = Reserved
									
	uint16_t wMaxPacketSize;		// Maximum Packet Size this endpoint is capable of sending or receiving
	uint8_t bInterval;				// Interval for polling endpoint data transfers. Value in frame counts.
									// Ignored for Bulk & Control Endpoints. Isochronous must equal 1 and
									// field may range from 1 to 255 for interrupt endpoints.
}__attribute__((packed)) descriptor_endpoint_t;




#endif

