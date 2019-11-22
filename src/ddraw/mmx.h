//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//

#ifndef _MMX_H_
#define _MMX_H_

void __cdecl mmx_memcpy(void *d,void *s,int bytes);
void __cdecl mmx_convert_32_to_32_bgr888(void *d,void *s,int pixels);
void __cdecl mmx_convert_32_to_24_rgb888(void *d,void *s,int pixels);
void __cdecl mmx_convert_32_to_24_bgr888(void *d,void *s,int pixels);
void __cdecl mmx_convert_32_to_16_rgb565(void *d,void *s,int pixels);
void __cdecl mmx_convert_32_to_16_bgr565(void *d,void *s,int pixels);
void __cdecl mmx_convert_32_to_16_rgb555(void *d,void *s,int pixels);
void __cdecl mmx_convert_32_to_16_bgr555(void *d,void *s,int pixels);


#endif
