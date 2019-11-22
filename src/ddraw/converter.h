//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//

#ifndef _DDCONVERTER_H_
#define _DDCONVERTER_H_


// converter function type
typedef void (*CONVERTER) (void *src, void *dst, int pixels);

// converter request
CONVERTER request_converter (int bits, int r, int g, int b);


#endif
