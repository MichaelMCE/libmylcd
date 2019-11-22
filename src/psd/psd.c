


#include "../../include/mylcd.h"
#if ENABLE_GIFPSD
#include "../memory.h"
#include "../utils.h"
#include "../frame.h"
#include "../fileio.h"
#include "../pixel.h"
#include "../image.h"
#include "../convert.h"
#include "../misc.h"
#include "endianswap.h"

#define SWAP16(X)    ((X)=Endian_SwapLE16(X))
#define SWAP32(X)    ((X)=Endian_SwapLE32(X))

#ifdef _MSC_VER
#define STBI_HAS_LRTOL
#endif

#ifdef STBI_HAS_LRTOL
   #define stbi_lrot(x,y)  _lrotl(x,y)
#else
   #define stbi_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif


typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef int16_t			int16;
typedef uint32_t		uint32;
typedef int32_t			int32;
typedef unsigned int	uint;
typedef ubyte			stbi_uc;



typedef struct{
   uint32 img_x;
   uint32 img_y;
   int img_n;
   int img_out_n;

   FILE *img_file;
   int from_file;
   int buflen;
   uint8 buffer_start[128];
   uint8 *img_buffer;
   uint8 *img_buffer_end;
} stbi;


typedef struct stbi_gif_lzw_struct {
   int16 prefix;
   uint8 first;
   uint8 suffix;
} stbi_gif_lzw;

typedef struct stbi_gif_struct
{
   int w,h;
   stbi_uc *out;                 // output buffer (always 4 components)
   int flags, bgindex, ratio, transparent, eflags;
   uint8  pal[256][4];
   uint8 lpal[256][4];
   stbi_gif_lzw codes[4096];
   uint8 *color_table;
   int parse, step;
   int lflags;
   int start_x, start_y;
   int max_x, max_y;
   int cur_x, cur_y;
   int line_size;
} stbi_gif;


/*


static inline uint64_t freq;
static inline uint64_t tStart;
static double resolution;

static inline void setRes ()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&tStart);
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	resolution = 1.0 / (double)freq;
}


static inline double getTime ()
{
	uint64_t t1 = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((double)((uint64_t)(t1 - tStart) * resolution) * 1000.0);
}
*/
static inline void start_file(stbi *s, FILE *f)
{
   s->img_file = f;
   s->buflen = sizeof(s->buffer_start);
   s->img_buffer_end = s->buffer_start + s->buflen;
   s->img_buffer = s->img_buffer_end;
   s->from_file = 1;
} 

static inline void start_mem (stbi *s, uint8 const *buffer, int len)
{
   s->img_file = NULL;
   s->from_file = 0;

   s->img_buffer = (uint8 *) buffer;
   s->img_buffer_end = (uint8 *) buffer+len;
}

static inline void refill_buffer(stbi *s)
{
   int n = fread(s->buffer_start, 1, s->buflen, s->img_file);
   if (n == 0) {
      s->from_file = 0;
      s->img_buffer = s->img_buffer_end-1;
      *s->img_buffer = 0;
   } else {
      s->img_buffer = s->buffer_start;
      s->img_buffer_end = s->buffer_start + n;
   }
}
/*
static inline int at_eof(stbi *s)
{

   if (s->img_file) {
      if (!feof(s->img_file)) return 0;
      // if feof() is true, check if buffer = end
      // special case: we've only got the special 0 character at the end
      if (s->from_file == 0) return 1;
   }

   return s->img_buffer >= s->img_buffer_end;   
}*/

static inline int get8 (stbi *s)
{
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;

   if (s->from_file) {
      refill_buffer(s);
      return *s->img_buffer++;
   }

   return 0;
}

static inline uint8 get8u (stbi *s)
{
   return (uint8) get8(s);
}

static inline void skip(stbi *s, int n)
{
   if (s->img_file) {
      int blen = s->img_buffer_end - s->img_buffer;
      if (blen < n) {
         s->img_buffer = s->img_buffer_end;
         fseek(s->img_file, n - blen, SEEK_CUR);
         return;
      }
   }
   s->img_buffer += n;
}

static inline int get16(stbi *s)
{
   int z = get8(s);
   return (z << 8) + get8(s);
}

static inline uint32 get32(stbi *s)
{
   uint32 z = get16(s);
   return (z << 16) + get16(s);
}

static inline int get16le(stbi *s)
{
   int z = get8(s);
   return z + (get8(s) << 8);
}

static inline uint32 get32le(stbi *s)
{
   uint32 z = get16le(s);
   return z + (get16le(s) << 16);
}

#if 0
static inline void _32bpp24bppFrame (TFRAME *frame, ubyte *data, const int width, const int height, int ox, int oy)
{
	int p = 0;
	
	for (int y = oy; y < oy+height; y++){
		for (int x = ox; x < ox+width; x++){
			int r = data[p++];
			int g = data[p++];
			int b = data[p++];
			l_setPixel(frame, x, y, r<<16 | g<<8 | b);
			p++;
		}
	}
}

static inline void _32bpp32bppFrame (TFRAME *frame, ubyte *data, const int width, const int height, int ox, int oy)
{
	int p = 0;
	
	for (int y = oy; y < oy+height; y++){
		for (int x = ox; x < ox+width; x++){
			int r = data[p++];
			int g = data[p++];
			int b = data[p++];
			l_setPixel(frame, x, y, 0xFF<<24 | r<<16 | g<<8 | b);
			p++;
		}
	}
}

static inline void _32bpp32AbppFrame (TFRAME *frame, ubyte *data, const int width, const int height, const int ox, const int oy)
{
	
	for (int y = oy; y < oy+height; y++){
		for (int x = ox; x < ox+width; x++){
			int r = *data++ << 16;
			int g = *data++ << 8;
			int b = *data++;
			int a = *data++ << 24;
			
			l_setPixel(frame, x, y, a + r + g + b);
		}
	}
}
#endif


#if 1
static inline uint8 compute_y (const int r, const int g, const int b)
{
   return (uint8) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static inline unsigned char *convert_format (unsigned char *data, const int img_n, const int req_comp, const uint x, const uint y)
{
   unsigned char *good;

   if (req_comp == img_n) return data;
   //assert(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) l_malloc(req_comp * x * y);
   if (good == NULL) {
      l_free(data);
      return NULL;
   }

   for (int j=0; j < (int) y; ++j) {
      unsigned char *src  = data + j * x * img_n   ;
      unsigned char *dest = good + j * x * req_comp;

      #define COMBO(a,b)  ((a)*8+(b))
      #define CASE(a,b)   case COMBO((a),(b)): {for(int i=x-1; i >= 0; --i, src += a, dest += b)
      // convert source image with img_n components to one with req_comp components;
      // avoid switch per pixel, so use switch per scanline and massive macros
      switch (COMBO(img_n, req_comp)) {
         CASE(1,2) dest[0]=src[0], dest[1]=255;} break;
         CASE(1,3) dest[0]=dest[1]=dest[2]=src[0];} break;
         CASE(1,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=255;} break;
         CASE(2,1) dest[0]=src[0];} break;
         CASE(2,3) dest[0]=dest[1]=dest[2]=src[0];} break;
         CASE(2,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1];} break;
         CASE(3,4) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255;} break;
         CASE(3,1) dest[0]=compute_y(src[0],src[1],src[2]);} break;
         CASE(3,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = 255;} break;
         CASE(4,1) dest[0]=compute_y(src[0],src[1],src[2]);} break;
         CASE(4,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = src[3];} break;
         CASE(4,3) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2];} break;
         default: return NULL;//assert(0);
      }
      #undef CASE
   }

   l_free(data);
   return good;
}
#endif

// ########################################################################################################
// ########################################################################################################
// ######################################        PSD          #############################################
// ########################################################################################################
// ########################################################################################################


static inline int psd_getSize (stbi *s, int *w, int *h)
{
   // Check identifier
   if (get32(s) != 0x38425053)   // "8BPS"
      return 0;

   // Check file type version.
   if (get16(s) != 1)
      return 0;

   // Skip 6 reserved bytes.
   skip(s, 6 );

   // Read the number of channels (R, G, B, A, etc).
   int channelCount = get16(s);
   if (channelCount < 0 || channelCount > 16)
      return 0;

   // Read the rows and columns of the image.
   if (h) *h = get32(s);
   if (w) *w = get32(s);

   // Make sure the depth is 8 bits.
   if (get16(s) != 8)
      return 0;

#if 1
   int colourMode = get16(s);
   //printf("colour mode %i\n", colourMode);
   if (colourMode != 3 && colourMode != 4)
      return 0;
#endif

   return 1;
}


static inline stbi_uc *psd_load (stbi *s, int *x, int *y, int *comp, int req_comp)
{
   int   pixelCount;
   int channelCount, compression;
   int channel, i, count, len;
   int w,h;
   uint8 *out;

   // Check identifier
   if (get32(s) != 0x38425053)   // "8BPS"
      return NULL;

   // Check file type version.
   if (get16(s) != 1)
      return NULL;

   // Skip 6 reserved bytes.
   skip(s, 6 );

   // Read the number of channels (R, G, B, A, etc).
   channelCount = get16(s);
   if (channelCount < 0 || channelCount > 16)
      return NULL;

   // Read the rows and columns of the image.
   h = get32(s);
   w = get32(s);
   
   // Make sure the depth is 8 bits.
   if (get16(s) != 8)
      return NULL;

   // Make sure the color mode is RGB.
   // Valid options are:
   //   0: Bitmap
   //   1: Grayscale
   //   2: Indexed color
   //   3: RGB color
   //   4: CMYK color
   //   7: Multichannel
   //   8: Duotone
   //   9: Lab color
   int colourMode = get16(s);
   //printf("colour mode %i\n", colourMode);
   if (colourMode != 3 && colourMode != 4)
      return NULL;

   // Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
   skip(s,get32(s) );

   // Skip the image resources.  (resolution, pen tool paths, etc)
   skip(s, get32(s) );

   // Skip the reserved data.
   skip(s, get32(s) );

   // Find out if the data is compressed.
   // Known values:
   //   0: no compression
   //   1: RLE compressed
   compression = get16(s);
   if (compression > 1)
      return NULL;

   // Create the destination image.
   out = (stbi_uc *) l_malloc(4 * w*h);
   if (!out) return NULL;
   pixelCount = w*h;

   // Initialize the data to zero.
   //l_memset( out, 0, pixelCount * 4 );
   
   // Finally, the image data.
   if (compression) {
      // RLE as used by .PSD and .TIFF
      // Loop until you get the number of unpacked bytes you are expecting:
      //     Read the next source byte into n.
      //     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
      //     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
      //     Else if n is 128, noop.
      // Endloop

      // The RLE-compressed data is preceeded by a 2-byte data count for each row in the data,
      // which we're going to just skip.
      skip(s, h * channelCount * 2 );

      // Read the RLE data by channel.
      for (channel = 0; channel < 4; channel++) {
         uint8 *p;
         
         p = out+channel;
         if (channel >= channelCount) {
            // Fill this channel with default data.
            for (i = 0; i < pixelCount; i++) *p = (channel == 3 ? 255 : 0), p += 4;
         } else {
            // Read the RLE data.
            count = 0;
            while (count < pixelCount) {
               len = get8(s);
               if (len == 128) {
                  // No-op.
               } else if (len < 128) {
                  // Copy next len+1 bytes literally.
                  len++;
                  count += len;
                  while (len) {
                     *p = get8u(s);
                     p += 4;
                     len--;
                  }
               } else if (len > 128) {
                  uint8   val;
                  // Next -len+1 bytes in the dest are replicated from next source byte.
                  // (Interpret len as a negative 8-bit int.)
                  len ^= 0x0FF;
                  len += 2;
                  val = get8u(s);
                  count += len;
                  while (len) {
                     *p = val;
                     p += 4;
                     len--;
                  }
               }
            }
         }
      }
      
   } else {
      // We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
      // where each channel consists of an 8-bit value for each pixel in the image.
      
      // Read the data by channel.
      for (channel = 0; channel < 4; channel++) {
         uint8 *p;
         
         p = out + channel;
         if (channel > channelCount) {
            // Fill this channel with default data.
            for (i = 0; i < pixelCount; i++) *p = channel == 3 ? 255 : 0, p += 4;
         } else {
            // Read the data.
            for (i = 0; i < pixelCount; i++)
               *p = get8u(s), p += 4;
         }
      }
   }
#if 0
   if (req_comp && req_comp != 4) {
      out = convert_format(out, 4, req_comp, w, h);
      if (out == NULL) return out; // convert_format frees input on failure
   }
#endif
   if (comp) *comp = channelCount;
   *y = h;
   *x = w;
   
   return out;
}

static inline stbi_uc *stbi_psd_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_file(&s, f);
   return psd_load(&s, x,y,comp,req_comp);
}

static inline stbi_uc *stbi_psd_load(wchar_t const *filename, int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = l_wfopen(filename, L"rb");
   if (!f) return NULL;
   data = stbi_psd_load_from_file(f, x,y,comp,req_comp);
   l_fclose(f);
   return data;
}

static inline stbi_uc *stbi_psd_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_mem(&s, buffer, len);
   return psd_load(&s, x,y,comp,req_comp);
}

int loadPsd (TFRAME *frame, const int flags, const wchar_t *filename, const int ox, const int oy)
{
	int width=0, height=0, comp;

	uint32_t *pixels = (uint32_t*)stbi_psd_load(filename, &width, &height, &comp, 0);
	if (pixels){
		if (flags&LOAD_RESIZE){
			if (!_resizeFrame(frame, width, height, 0)){
				l_free(pixels);
				return 0;
			}	
		}
	

#if 1	// x2 faster (than below #else) but less acurate
		uint8_t *des = (uint8_t*)frame->pixels;
		uint8_t *src = (uint8_t*)pixels;

		int tpixels = MIN((width*height), (frame->width*frame->height));
		while(tpixels--){
			*des++ = src[2];
			*des++ = src[1];
			*des++ = src[0];
			*des++ = src[3];
			src += 4;
		}
#else
		if (frame->bpp == LFRM_BPP_32A)
			_32bpp32AbppFrame(frame, (ubyte*)pixels, width, height, ox, oy);
		else if (frame->bpp == LFRM_BPP_32)
			_32bpp32bppFrame(frame, (ubyte*)pixels, width, height, ox, oy);
		else if (frame->bpp == LFRM_BPP_24)
			_32bpp24bppFrame(frame, (ubyte*)pixels, width, height, ox, oy);
#endif
		l_free(pixels);
		return 1;
	}

	return 0;
}

int savePsd (TFRAME *frame, const wchar_t *filename, int w, int h, const int flags)
{
	mylog("PSD write not implemented\n");
	return 0;
}

int psdGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp)
{
	if (bpp) *bpp = LFRM_BPP_32;

	FILE *f = l_wfopen(filename, L"rb");
	if (!f) return 0;
   
	stbi s;
	start_file(&s, f);
	int ret = psd_getSize(&s, width, height);
	fclose(f);
	return ret;

}



// ########################################################################################################
// ########################################################################################################
// ######################################        GIF          #############################################
// ########################################################################################################
// ########################################################################################################


static inline void stbi_gif_parse_colortable (stbi *s, uint8 pal[256][4], int num_entries, int transp)
{
   int i;
   for (i=0; i < num_entries; ++i) {
      pal[i][2] = get8u(s);
      pal[i][1] = get8u(s);
      pal[i][0] = get8u(s);
      pal[i][3] = transp ? 0 : 255;
   }   
}

static inline int stbi_gif_header(stbi *s, stbi_gif *g, int *comp, int is_info)
{
   uint8 version;
   if (get8(s) != 'G' || get8(s) != 'I' || get8(s) != 'F' || get8(s) != '8')
      return 0;

   version = get8u(s);
   if (version != '7' && version != '9')    return 0;
   if (get8(s) != 'a')                      return 0;
 
   g->w = get16le(s);
   g->h = get16le(s);
   g->flags = get8(s);
   g->bgindex = get8(s);
   g->ratio = get8(s);
   g->transparent = -1;

   if (comp != 0) *comp = 4;  // can't actually tell whether it's 3 or 4 until we parse the comments

   if (is_info) return 1;

   if (g->flags & 0x80)
      stbi_gif_parse_colortable(s,g->pal, 2 << (g->flags & 7), -1);

   return 1;
}


static inline void stbi_out_gif_code(stbi_gif *g, uint16 code)
{
   uint8 *p, *c;

   // recurse to decode the prefixes, since the linked-list is backwards,
   // and working backwards through an interleaved image would be nasty
   if (g->codes[code].prefix >= 0)
      stbi_out_gif_code(g, g->codes[code].prefix);

   if (g->cur_y >= g->max_y) return;
  
   p = (uint8*)&g->out[g->cur_x + g->cur_y];
   c = (uint8*)&g->color_table[g->codes[code].suffix * 4];

   if (c[3]){
#if 0
      *p = *c;
#else
      p[0] = c[0];
      p[1] = c[1];
      p[2] = c[2];
      p[3] = c[3];
#endif
   }
   g->cur_x += 4;

   if (g->cur_x >= g->max_x) {
      g->cur_x = g->start_x;
      g->cur_y += g->step;

      while (g->cur_y >= g->max_y && g->parse > 0) {
         g->step = (1 << g->parse) * g->line_size;
         g->cur_y = g->start_y + (g->step >> 1);
         --g->parse;
      }
   }
}

static inline uint8 *stbi_process_gif_raster(stbi *s, stbi_gif *g)
{
   uint8 lzw_cs;
   int32 len, code;
   uint32 first;
   int32 codesize, codemask, avail, oldcode, bits, valid_bits, clear;
   stbi_gif_lzw *p;

   lzw_cs = get8u(s);
   clear = 1 << lzw_cs;
   first = 1;
   codesize = lzw_cs + 1;
   codemask = (1 << codesize) - 1;
   bits = 0;
   valid_bits = 0;
   for (code = 0; code < clear; code++) {
      g->codes[code].prefix = -1;
      g->codes[code].first = (uint8) code;
      g->codes[code].suffix = (uint8) code;
   }

   // support no starting clear code
   avail = clear+2;
   oldcode = -1;

   len = 0;
   for(;;) {
      if (valid_bits < codesize) {
         if (len == 0) {
            len = get8(s); // start new block
            if (len == 0) 
               return g->out;
         }
         --len;
         bits |= (int32) get8(s) << valid_bits;
         valid_bits += 8;
      } else {
         int32 code = bits & codemask;
         bits >>= codesize;
         valid_bits -= codesize;
         // @OPTIMIZE: is there some way we can accelerate the non-clear path?
         if (code == clear) {  // clear code
            codesize = lzw_cs + 1;
            codemask = (1 << codesize) - 1;
            avail = clear + 2;
            oldcode = -1;
            first = 0;
         } else if (code == clear + 1) { // end of stream code
            skip(s, len);
            while ((len = get8(s)) > 0)
               skip(s,len);
            return g->out;
         } else if (code <= avail) {
            if (first) return NULL;

            if (oldcode >= 0) {
               p = &g->codes[avail++];
               if (avail > 4096) return NULL;
               p->prefix = (int16) oldcode;
               p->first = g->codes[oldcode].first;
               p->suffix = (code == avail) ? p->first : g->codes[code].first;
            } else if (code == avail)
               return NULL;

            stbi_out_gif_code(g, (uint16) code);

            if ((avail & codemask) == 0 && avail <= 0x0FFF) {
               codesize++;
               codemask = (1 << codesize) - 1;
            }

            oldcode = code;
         } else {
            return NULL;
         }
      } 
   }
}

static inline  void stbi_fill_gif_background(stbi_gif *g)
{
   int i;
   uint8 *c = g->pal[g->bgindex];
   // @OPTIMIZE: write a dword at a time
   for (i = 0; i < g->w * g->h * 4; i += 4) {
      uint8 *p  = &g->out[i];
      p[0] = c[2];
      p[1] = c[1];
      p[2] = c[0];
      p[3] = c[3];
   }
}

// this function is designed to support animated gifs, although stb_image doesn't support it
static inline uint8 *stbi_gif_load_next(stbi *s, stbi_gif *g, int *comp, int req_comp)
{
   int i;
   uint8 *old_out = 0;

   if (g->out == 0) {
      if (!stbi_gif_header(s, g, comp,0))     return 0; // failure_reason set by stbi_gif_header
      g->out = (uint8 *) l_malloc(4 * g->w * g->h);
      if (g->out == 0) return NULL;
      stbi_fill_gif_background(g);
   } else {
      // animated-gif-only path
      if (((g->eflags & 0x1C) >> 2) == 3) {
         old_out = g->out;
         g->out = (uint8 *) l_malloc(4 * g->w * g->h);
         if (g->out == 0) return NULL;
         l_memcpy(g->out, old_out, g->w*g->h*4);
      }
   }
    
   for (;;) {
      switch (get8(s)) {
         case 0x2C: /* Image Descriptor */
         {
            int32 x, y, w, h;
            uint8 *o;

            x = get16le(s);
            y = get16le(s);
            w = get16le(s);
            h = get16le(s);
            if (((x + w) > (g->w)) || ((y + h) > (g->h)))
               return NULL;

            g->line_size = g->w * 4;
            g->start_x = x * 4;
            g->start_y = y * g->line_size;
            g->max_x   = g->start_x + w * 4;
            g->max_y   = g->start_y + h * g->line_size;
            g->cur_x   = g->start_x;
            g->cur_y   = g->start_y;

            g->lflags = get8(s);

            if (g->lflags & 0x40) {
               g->step = 8 * g->line_size; // first interlaced spacing
               g->parse = 3;
            } else {
               g->step = g->line_size;
               g->parse = 0;
            }

            if (g->lflags & 0x80) {
               stbi_gif_parse_colortable(s,g->lpal, 2 << (g->lflags & 7), g->eflags & 0x01 ? g->transparent : -1);
               g->color_table = (uint8 *) g->lpal;       
            } else if (g->flags & 0x80) {
               for (i=0; i < 256; ++i)  // @OPTIMIZE: reset only the previous transparent
                  g->pal[i][3] = 255; 
               if (g->transparent >= 0 && (g->eflags & 0x01))
                  g->pal[g->transparent][3] = 0;
               g->color_table = (uint8 *) g->pal;
            } else
               return NULL;
   
            o = stbi_process_gif_raster(s, g);
            if (o == NULL) return NULL;

            if (req_comp && req_comp != 4)
               o = convert_format(o, 4, req_comp, g->w, g->h);
            return o;
         }

         case 0x21: // Comment Extension.
         {
            int len;
            if (get8(s) == 0xF9) { // Graphic Control Extension.
               len = get8(s);
               if (len == 4) {
                  g->eflags = get8(s);
                  get16le(s); // delay
                  g->transparent = get8(s);
               } else {
                  skip(s, len);
                  break;
               }
            }
            while ((len = get8(s)) != 0)
               skip(s, len);
            break;
         }

         case 0x3B: // gif stream termination code
            return (uint8 *) 1;

         default:
            return NULL;
      }
   }
}

static inline stbi_uc *stbi_gif_load_from_file (FILE *f, int *x, int *y, int *comp, int req_comp)
{
   uint8 *u = 0;
   stbi s;
   stbi_gif g={0};
   start_file(&s, f);

   u = stbi_gif_load_next(&s, &g, comp, req_comp);
   if (u == (void *) 1) u = 0;  // end of animated gif marker
   if (u) {
      *x = g.w;
      *y = g.h;
   }

   return u;
}

static inline void *stbi_gif_load (wchar_t const *filename, int *x, int *y, int *comp, int req_comp)
{
   uint8 *data;
   FILE *f = _wfopen(filename, L"rb");
   if (!f) return NULL;
   data = stbi_gif_load_from_file(f, x,y,comp,req_comp);
   l_fclose(f);
   return data;
}


int loadGif (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
	int width, height;
	int comp;

	uint32_t *pixels = (uint32_t*)stbi_gif_load(filename, &width, &height, &comp, 0);
	if (pixels){
		if (flags&LOAD_RESIZE){
			if (!_resizeFrame(frame, width, height, 0)){
				l_free(pixels);
				return 0;
			}	
		}
		
#if 1
		uint32_t *des = (uint32_t*)frame->pixels;
		uint32_t *src = (uint32_t*)pixels;
		int tpixels = MIN((width*height), (frame->width*frame->height));

#if 1
		l_memcpy(des, src, tpixels*sizeof(int));
#else
		while(tpixels--) *des++ = *src++;
#endif

#else
		if (frame->bpp == LFRM_BPP_32A)
			_32bpp32AbppFrame(frame, (ubyte*)gpixels, width, height, ox, oy);
		else if (frame->bpp == LFRM_BPP_32)
			_32bpp32bppFrame(frame, (ubyte*)gpixels, width, height, ox, oy);
		else if (frame->bpp == LFRM_BPP_24)
			_32bpp24bppFrame(frame, (ubyte*)gpixels, width, height, ox, oy);
		else if (frame->bpp == LFRM_BPP_16)
			_32bpp16bpp565Frame(frame, (ubyte*)gpixels, width, height, ox, oy);
		else if (frame->bpp == LFRM_BPP_15)
			_32bpp16bpp555Frame(frame, (ubyte*)gpixels, width, height, ox, oy);
#endif
		l_free(pixels);
		return 1;
	}

	return 0;
}

int saveGif (TFRAME *frame, const wchar_t *filename, int w, int h, const int flags)
{
	mylog("GIF write not implemented\n");
	return 0;
}


static inline int gifGetSize (const wchar_t * szFileName, int *gwidth, int *gheight, int *gbpp)
{

	// OPEN FILE
	FILE *fd = l_wfopen(szFileName, L"rb");
	if (!fd)
	{
	  return 0;
	}
	
	// *1* READ HEADERBLOCK (6bytes) (SIGNATURE + VERSION)
	char szSignature[6];    // First 6 bytes (GIF87a or GIF89a)
	int iRead = l_fread(szSignature, 1, 6, fd);
	if (iRead != 6)
	{
	  l_fclose(fd);
	  return 0;
	}
  

	if (szSignature[0] != 'G' || szSignature[1] != 'I' || szSignature[2] != 'F'){
		//printf("sig failed\n");
		l_fclose(fd);
    	return 0;
	}

	struct GIFLSDtag {
		unsigned short ScreenWidth;  // Logical Screen Width
		unsigned short ScreenHeight; // Logical Screen Height
		unsigned char PackedFields;  // Packed Fields. Bits detail:
		//  0-2: Size of Global Color Table
		//    3: Sort Flag
		//  4-6: Color Resolution
		//    7: Global Color Table Flag
		unsigned char Background;  // Background Color Index
		unsigned char PixelAspectRatio; // Pixel Aspect Ratio
	}__attribute__((packed))giflsd;
	

	// *2* READ LOGICAL SCREEN DESCRIPTOR
	iRead = l_fread(&giflsd, 1, sizeof(giflsd), fd);
	if (iRead != sizeof(giflsd))
	{
		//printf("read failed\n");
	  l_fclose(fd);
	  return 0;
	}
	
	l_fclose(fd);
	
	// endian swap
	SWAP16(giflsd.ScreenWidth);
	SWAP16(giflsd.ScreenHeight);
	
	if (gbpp) *gbpp = (giflsd.PackedFields & 0x07) + 1;
	if (gwidth) *gwidth = (int)giflsd.ScreenWidth;
	if (gheight) *gheight = (int)giflsd.ScreenHeight;
	
	return *gwidth * *gheight;
  
}

int gifGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp)
{
	if (bpp) *bpp = LFRM_BPP_32;
	return gifGetSize(filename, width, height, NULL);
}

#else

int loadGif (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy){return 0;}
int loadPsd (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy){return 0;}
int saveGif (TFRAME *frame, const wchar_t *filename, int w, int h, const int flags){return 0;}
int savePsd (TFRAME *frame, const wchar_t *filename, int w, int h, const int flags){return 0;}

int psdGetMetrics (wchar_t *filename, int *width, int *height, int *bpp){return 0;}
int gifGetMetrics (wchar_t *filename, int *width, int *height, int *bpp){return 0;}

#endif


