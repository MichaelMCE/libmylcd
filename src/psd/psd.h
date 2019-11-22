


#ifndef _PSD_H_
#define _PSD_H_


void *stbi_gif_load (wchar_t const *filename, int *x, int *y, int *comp, int req_comp);

int loadPsd (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy);
int savePsd (TFRAME *frame, const wchar_t *filename, int w, int h, const int flags);
int loadJpg2 (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy);



int psdGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp);
int gifGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp);



#endif




