// Simple test of USB Host
//
// This example is in the public domain


#include "usbhost/USBHost_t36.h"
#include "usbd480/usbd480.h"
#include "libpng/png.h"
#include "SdFat.h"



static USBHost myusb;
static USBHub hub1(myusb);
static USBD480Display usbd480(myusb);
static SdFatSdioEX sdEx;
static File file;

int doTests = 0;


typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
}__attribute__ ((packed))TRGB;		// 888





inline void setPixel16 (uint8_t *buffer, const int pitch, const int x, const int y, uint16_t colour)
{
	//uint16_t *pixels = (uint16_t*)buffer;	
	
//	if (x >= BWIDTH || x < 0) return;
//	if (y >= BHEIGHT || y < 0) return;
	
	*(uint16_t*)(buffer+(y*pitch) + (x<<1)) = colour;
}

inline void setPixel24 (uint8_t *buffer, const int pitch, const int x, const int y, uint32_t colour)
{
	
//	if (x >= BWIDTH || x < 0) return;
//	if (y >= BHEIGHT || y < 0) return;

	TRGB *addr = (TRGB*)(buffer+((y*pitch)+(x*3)));
	*addr = *(TRGB*)&colour;
}


inline void readPng32To16_565 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<1;
	
	while(passCount--){
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<8;	// 5
			int g = (line[i++]&0xFC)<<3;	// 6
			int b = (line[i++]&0xF8)>>3;	// 5
			setPixel16(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
	}
}

inline void readPng32To16_555 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<1;
	
	while(passCount--){
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<7;	// 5
			int g = (line[i++]&0xF8)<<2;	// 6
			int b = (line[i++]&0xF8)>>3;	// 5
			setPixel16(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
	}
}

static inline void readPng32To24 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width*3;
	
	while(passCount--){
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++, i += 4){
			setPixel24(frame, pitch, x+ox, y+oy, line[i]<<16|line[i+1]<<8|line[i+2]);
		}
	}
	}
}

int readPngToFrame (uint8_t *frame, png_structp *png_ptr, int bpp, int width, int height, int ox, int oy, int passCount)
{
	uint8_t line[(width * 4) + 4];		// enough for a single RGBA line

	int ret = 1;
	switch (bpp){
	  case 15: readPng32To16_555(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case 16: readPng32To16_565(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case 24: readPng32To24(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case 32: // not implemented
	  default: ret = 0;
	};

	return ret;
}

static inline void PNGAPI warningCallback (png_structp png_ptr, png_const_charp warning_msg)
{
	char *err = (char *)png_get_error_ptr(png_ptr);
	if (err){
		Serial.printf("libmylcd: png warning (%s), %s\r\n",err, (intptr_t)warning_msg);
	}else{
		Serial.printf("libmylcd: png warning %s\r\n",(intptr_t)warning_msg);
	}
}


void png_read_data (png_structp png_ptr, png_bytep data, png_size_t length)
{

	//File *file = (File*)png_ptr->io_ptr;

	png_size_t check = (png_size_t)file.read(data, length);

	if (check != length){
		Serial.printf("Read Error\r\n");
	}
}

int32_t loadPng (uint8_t *frame, const int flags, const char *filename, int32_t ox, int32_t oy, uint32_t *width, uint32_t *height, uint32_t *bpp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;

	
	if (!file.open(filename, O_RDONLY)){
		return 0;
	}
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		file.close();
		return 0;
	}

	png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) warningCallback, warningCallback);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		file.close();
		return 0;
	}

#if 0
   if (setjmp(png_jmpbuf(png_ptr))){
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	  file.close();
      /* If we get here, we had a problem reading the file */
      return 0;
   }
#endif

	//png_init_io(png_ptr, fp); 
	png_set_read_fn(png_ptr, (png_voidp)&file, png_read_data);
	 
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);

	int passCount = 1;
	int bit_depth, color_type, interlace_type;
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)width, (png_uint_32*)height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	if (bpp) *bpp = bit_depth;
	
	if (frame != NULL){

		if (interlace_type == PNG_INTERLACE_ADAM7)
			passCount = png_set_interlace_handling(png_ptr);
		//printf("interlace_type %i, ct:%i, %i %i\n", interlace_type, passCount, *width, *height);

	
		if (bit_depth == 16)
			png_set_strip_16(png_ptr);
		png_set_packing(png_ptr);

		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);
		else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand_gray_1_2_4_to_8(png_ptr);

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png_ptr);
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		
		/*if (flags&LOAD_RESIZE){
			if (!_resizeFrame(frame, *width, *height, 0))
				return 0;
		}*/
	}

	int ret = 1;
	if (frame)
		ret = readPngToFrame(frame, &png_ptr, flags, *width, *height, ox, oy, passCount);
	
	//png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
	file.close();
	return ret;
}

int32_t png_read (const char *filename, uint8_t *buffer, const int bufferbpp, int ox, int oy)
{
	uint32_t width;
	uint32_t height;
	
	return loadPng(buffer, bufferbpp, filename, ox, oy, &width, &height, NULL);
}

int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp)
{
	*width = 0;
	*height = 0;
	*filebpp = 0;

	return loadPng(NULL, 0, filename, 0, 0, width, height, filebpp);
}

inline bool sdBusy ()
{
	return sdEx.card()->isBusy();
}

static bool sdInit ()
{
	if (!sdEx.begin()){
      Serial.println("sdEx.begin() failed");
      return false;
     }

	sdEx.chvol();
	return 1;
}


void runTest ()
{
	uint32_t width, height, filebpp;

	if (png_metrics("somuchwin24.png", &width, &height, &filebpp)){
		Serial.printf("width %i, height %i, bpp %i\r\n", width, height, filebpp);
				
		uint8_t buffer[width * height * sizeof(uint16_t)];
		
		int ret = png_read("somuchwin24.png", buffer, 16, 0, 0);
		Serial.printf("png_read ret %i\r\n", ret);
		
		int x = (usbd480.display.width - width)/2;
		int y = (usbd480.display.height - height)/2;
		usbd480.drawScreenArea(buffer, width, height, x, y);
	}
}

int usbd_callback (uint32_t msg, intptr_t *value1, uint32_t value2)
{
	//Serial.printf("usbd_callback %i %i %i", (int)msg, value1, value2);
	//Serial.println();
	
	if (msg == USBD_MSG_DISPLAYREADY){
		doTests = 1;
	}
	
	return 1;
}

void setup ()
{
	while (!Serial) ; // wait for Arduino Serial Monitor
	
	Serial.println("libpng testing");
	
	myusb.begin();
	usbd480.setCallbackFunc(usbd_callback);
	delay(25);
	sdInit();
}

void loop ()
{
	myusb.Task();
	
	if (doTests){
		doTests = 0;
		
		delay(10);
		runTest();
	}
}

