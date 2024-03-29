//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//

#include "mylcd.h"

#if ((__BUILD_DDRAW__) && (__BUILD_WIN32__))

#include "converter.h"
#include "dd.h"
#include "mmx.h"


void convert_32_to_32_rgb888 (void *src, void *dst, int pixels)
{
    #ifdef __MDD_MMX__
        mmx_memcpy(dst,src,pixels<<2);
    #else
        #ifdef __MDD_MEMCPY__
            memcpy(dst,src,pixels<<2);
        #else
            int *p = (int*) src;
            int *q = (int*) dst;
            while (pixels--)
                *(q++) = *(p++);
        #endif
    #endif
}


void convert_32_to_32_bgr888 (void *src, void *dst, int pixels)
{
    int *p = (int*) src;
    int *q = (int*) dst;
    while (pixels--)
    {
        int r = (int)((*p & 0x00FF0000) >> 16);
        int g = (int)((*p & 0x0000FF00) );
        int b = (int)((*p & 0x000000FF) << 16 );
        *(q++) = r | g | b;
    }
}


void convert_32_to_24_rgb888(void *src,void *dst,int pixels)
{
    #ifdef __MDD_MMX__

        mmx_convert_32_to_24_rgb888(dst,src,pixels);

    #else

        char8 *p = (char8*) src;
        char8 *q = (char8*) dst;
        while (pixels--)
        {
            #ifdef __MDD_LITTLE_ENDIAN__
            *q = *p;
            *(q+1) = *(p+1);
            *(q+2) = *(p+2);
            #else
            *(q+2) = *p;
            *(q+1) = *(p+1);
            *q = *(p+2);
            #endif
            p+=4;
            q+=3;
        }

    #endif
}


void convert_32_to_24_bgr888(void *src,void *dst,int pixels)
{
    char8 *p = (char8*) src;
    char8 *q = (char8*) dst;
    while (pixels--)
    {
        #ifdef __MDD_LITTLE_ENDIAN__
        *(q+2) = *p;
        *(q+1) = *(p+1);
        *q = *(p+2);
        #else
        *q = *p;
        *(q+1) = *(p+1);
        *(q+2) = *(p+2);
        #endif
        p+=4;
        q+=3;
    }
}


void convert_32_to_16_rgb565(void *src,void *dst,int pixels)
{
    #ifdef __MDD_MMX__

        mmx_convert_32_to_16_rgb565(dst,src,pixels);

    #else

        int *p = (int*) src;
        short16 *q = (short16*) dst;
        while (pixels--)
        {
            short16 r = (short16) ( (*p & 0x00F80000) >> 8 );
            short16 g = (short16) ( (*p & 0x0000FC00) >> 5 );
            short16 b = (short16) ( (*p & 0x000000F8) >> 3 );
            *q = r | g | b;
            p++;
            q++;
        }

    #endif
}


void convert_32_to_16_bgr565(void *src,void *dst,int pixels)
{
    #ifdef __MDD_MMX__

        mmx_convert_32_to_16_bgr565(dst,src,pixels);

    #else

        int *p = (int*) src;
        short16 *q = (short16*) dst;
        while (pixels--)
        {
            short16 r = (short16) ( (*p & 0x00F80000) >> 19 );
            short16 g = (short16) ( (*p & 0x0000FC00) >> 5  );
            short16 b = (short16) ( (*p & 0x000000F8) << 8  );
            *q = r | g | b;
            p++;
            q++;
        }

    #endif
}


void convert_32_to_16_rgb555(void *src,void *dst, int pixels)
{
    #ifdef __MDD_MMX__

        mmx_convert_32_to_16_rgb555(dst,src,pixels);

    #else

        int *p = (int*) src;
        short16 *q = (short16*) dst;
        while (pixels--){
            short16 r = (short16)((*p & 0x00F80000) >> 9);
            short16 g = (short16)((*p & 0x0000F800) >> 6);
            short16 b = (short16)((*p & 0x000000F8) >> 3);
            *q = r | g | b;
            p++;
            q++;
        }

    #endif
}


void convert_32_to_16_bgr555(void *src,void *dst,int pixels)
{
    #ifdef __MDD_MMX__

        mmx_convert_32_to_16_rgb555(dst,src,pixels);

    #else

        int *p = (int*) src;
        short16 *q = (short16*) dst;
        while (pixels--)
        {
            short16 r = (short16) ( (*p & 0x00F80000) >> 20);
            short16 g = (short16) ( (*p & 0x0000F800) >> 6);
            short16 b = (short16) ( (*p & 0x000000F8) << 8);
            *q = r | g | b;
            p++;
            q++;
        }

    #endif
}


CONVERTER request_converter (int bits, int r, int g, int b)
{
    // 32bit RGB888 -> 32bit RGB888
    #ifdef __MDD_CONVERTER_32_TO_32_RGB888
    if (bits==32 && r==0x00FF0000 && g==0x0000FF00 && b==0x000000FF) return &convert_32_to_32_rgb888;
    #endif

    // 32bit RGB888 -> 32bit BGR888
    #ifdef __MDD_CONVERTER_32_TO_32_BGR888
    if (bits==32 && r==0x000000FF && g==0x0000FF00 && b==0x00FF0000) return &convert_32_to_32_bgr888;
    #endif

    // 32bit RGB888 -> 24bit RGB888
    #ifdef __MDD_CONVERTER_32_TO_24_RGB888
    if (bits==24 && r==0x00FF0000 && g==0x0000FF00 && b==0x000000FF) return &convert_32_to_24_rgb888;
    #endif

    // 32bit RGB888 -> 24bit BGR888
    #ifdef __MDD_CONVERTER_32_TO_24_BGR888
    if (bits==24 && r==0x000000FF && g==0x0000FF00 && b==0x00FF0000) return &convert_32_to_24_bgr888;
    #endif
    
    // 32bit RGB888 -> 16bit RGB565
    #ifdef __MDD_CONVERTER_32_TO_16_RGB565
    if (bits==16 && r==0xF800 && g==0x07E0 && b==0x001F) return &convert_32_to_16_rgb565;
    #endif

    // 32bit RGB888 -> 16bit BGR565
    #ifdef __MDD_CONVERTER_32_TO_16_BGR565
    if (bits==16 && r==0x001F && g==0x07E0 && b==0xF800) return &convert_32_to_16_bgr565;
    #endif

    // 32bit RGB888 -> 16bit RGB555
    #ifdef __MDD_CONVERTER_32_TO_16_RGB555
    if (bits==16 && r==0x7C00 && g==0x03E0 && b==0x001F) return &convert_32_to_16_rgb555;
    #endif

    // 32bit RGB888 -> 16bit BGR555
    #ifdef __MDD_CONVERTER_32_TO_16_BGR555
    if (bits==16 && r==0x001F && g==0x03E0 && b==0x7C00) return &convert_32_to_16_bgr555;
    #endif

    // failure
    return 0;
}
#endif

