


#ifndef _USBH_COMMON_H_
#define _USBH_COMMON_H_


#define DWIDTH			480
#define DHEIGHT			272


#define USBH_ENUMSTATE_END		15




#define USBD_MSG_DISPLAYREADY			1
#define USBD_MSG_BUFFEREXHUSTED			2
#define USBD_MSG_WRITEREADY				3
#define USBD_MSG_WRITEEND				4


typedef struct {
	void *buffer;
	uint32_t len;		// length in  bytes
	int32_t rows;		// how many rows does this buffer describe
	
	int32_t row;		// next write points to data for this row
						// next pixel read request askas for data for this row
						
	int32_t frame;
}tx_descript_t;

typedef struct {
	int32_t x1;
	int32_t y1;
	int32_t x2;
	int32_t y2;
}area_t;



#endif
