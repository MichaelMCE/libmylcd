
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2019  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#include "utils/utils.h"


#define text "The quick brown fox jumps over the lazy dog."




void drawMultilayer1 (TFRAME *frame, const int x, const int y, const int font, const char *str, const int render)
{

	TLPRINTR trect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	memset(&trect, 0, sizeof(trect));
	
	int flags = PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP;
	if (!render) flags |= PF_DONTRENDER;
	const int colour = 0x000000;		// 24bit only
	int blurOp = LTR_BLUR5;


	// not really required in this use case
	lRenderEffectReset(hw, font, LTR_BLUR5);
	lRenderEffectReset(hw, font, LTR_BLUR4);
	lRenderEffectReset(hw, font, LTR_BLUR1);
	

	THWD *hw = frame->hw;
	int oldSpacing = lGetFontCharacterSpacing(hw, font);
	lSetFontCharacterSpacing(hw, font, oldSpacing+2);
	
	// shadow base
	lSetRenderEffect(hw, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 12);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 1000);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);


	// outline
	blurOp = LTR_BLUR4;
	//lRenderEffectReset(hw, font, blurOp);
	lSetRenderEffect(hw, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0x508DC5);
	//lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0x000000);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 4);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 1000);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);


	// top
	blurOp = LTR_BLUR1;
	lSetRenderEffect(hw, blurOp);
	//lRenderEffectReset(hw, font, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0xFFFFFF);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 3);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	//lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 1000);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);
	trect.sx = x; trect.sy = y;
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 800);
	//lPrintEx(frame, &trect, font, flags, LPRT_OR, str);

	// light overlay (green)
	blurOp = LTR_BLUR5;
	lSetRenderEffect(hw, blurOp);
	//lRenderEffectReset(hw, font, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0x00FF1E);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 2);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 300);
	trect.sx = x; trect.sy = y;
	//lPrintEx(frame, &trect, font, flags, LPRT_OR, str);

	lSetFontCharacterSpacing(hw, font, oldSpacing);
}

void renderCB_texFilter (TLPRINTREGION *loc)
{
	TGLYPHPOINTS *gp = loc->glyph->gp;
	TFRAME *to = loc->to;
	TFRAME *tex = (TFRAME*)to->hw->render->udata;
	
	const int w = tex->width-1;
	const int h = tex->height-1;
		
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = loc->dx + gp->points[i].x;
		int y = loc->dy + gp->points[i].y;
		
		int col = lGetPixel(tex, x%w, y%h)&0xFFFFFF;
		int a = ((lGetPixel(to, x, y) >> 24)&0xFF)<<1;
		if (a > 255) a = 255;
		lSetPixel(to, x, y, (a<<24)|col);
	}
}

void drawMultilayer2 (TFRAME *frame, const int x, const int y, const int font, const char *str, const int render)
{

	TLPRINTR trect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	memset(&trect, 0, sizeof(trect));
	
	int flags = PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP;
	if (!render) flags |= PF_DONTRENDER;
	const int colour = 0x000000;		// 24bit only
	int blurOp = LTR_BLUR5;


	// not really required in this use case
	lRenderEffectReset(hw, font, LTR_BLUR5);
	lRenderEffectReset(hw, font, LTR_BLUR4);
	lRenderEffectReset(hw, font, LTR_BLUR1);
	

	THWD *hw = frame->hw;
	
	int oldSpacing = lGetFontCharacterSpacing(hw, font);
	lSetFontCharacterSpacing(hw, font, oldSpacing+2);
	
#if 1	
	// shadow base
	lSetRenderEffect(hw, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 12);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 1000);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);
#endif

#if 1
	// outline
	blurOp = LTR_BLUR4;
	//lRenderEffectReset(hw, font, blurOp);
	lSetRenderEffect(hw, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0x508DC5);
	//lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0x000000);
	//lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0xFFFFFF);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 4);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 1000);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);
#endif
	
	
#if 1
	// top
	blurOp = LTR_BLUR1;
	lSetRenderEffect(hw, blurOp);
	//lRenderEffectReset(hw, font, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0xFFFFFF);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 3);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 1000);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);
	trect.sx = x; trect.sy = y;
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 800);
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);

	// light overlay (green)
	blurOp = LTR_BLUR5;
	lSetRenderEffect(hw, blurOp);
	//lRenderEffectReset(hw, font, blurOp);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_COLOUR, 0x00FF1E);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_RADIUS, 2);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, blurOp, LTRA_BLUR_ALPHA, 300);
	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);
#endif




	TFRAME *tex = lNewImage(frame->hw, L"images/fire.jpg", frame->bpp);
	
	frame->hw->render->copy = (void*)renderCB_texFilter;
	frame->hw->render->udata = (intptr_t)tex;

	trect.sx = x; trect.sy = y;
	lPrintEx(frame, &trect, font, flags, LPRT_OR, str);
	lDeleteFrame(tex);

	lSetFontCharacterSpacing(hw, font, oldSpacing);
}



static inline void doPrint (TFRAME *frame, const int render)
{

#if 1
	TLPRINTR trect = {0,1,frame->width-1,frame->height-1,0,0,0,0};
	
	int flags = PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP;
	if (render)
		flags |= 0;
	else
		flags |= PF_DONTRENDER;
	
#endif
	
#if 1
	
#if 1
	lSetPixelWriteMode(frame, LSP_XOR);
	lSetRenderEffect(hw, LTR_OUTLINE2);
	lSetFilterAttribute(hw, LTR_OUTLINE2, 0, lGetRGBMask(frame, LMASK_RED));	// set outline colour
	lPrintEx(frame, &trect, LFTW_B14, flags, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_OUTLINE3);
	lSetFilterAttribute(hw, LTR_OUTLINE3, 0, 0xFF000000);	// set outline colour
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);
	
	lSetPixelWriteMode(frame, LSP_SET);

	lSetRenderEffect(hw, LTR_SMOOTH2);
	lSetFilterAttribute(hw, LTR_SMOOTH2, 0, (90 << 24) | 0x00FF00);	// set smoothing colour
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);
		
	lSetRenderEffect(hw, LTR_SKEWEDFW);		// forward skew
	lSetFilterAttribute(hw, LTR_SKEWEDFW, 0, 100);	// set slope factor (1-255)
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDFWSM1);	// with a little smoothing
	lSetFilterAttribute(hw, LTR_SKEWEDFWSM1, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDFWSM2);	// with a little more smoothing
	lSetFilterAttribute(hw, LTR_SKEWEDFWSM2, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);
	
	lSetRenderEffect(hw, LTR_SKEWEDBK);		// backward skew
	lSetFilterAttribute(hw, LTR_SKEWEDBK, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDBKSM1);
	lSetFilterAttribute(hw, LTR_SKEWEDBKSM1, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDBKSM2);
	lSetFilterAttribute(hw, LTR_SKEWEDBKSM2, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);
	
	lSetRenderEffect(hw, LTR_DEFAULT);
	lPrintEx(frame, &trect, LFTW_UNICODE, flags|PF_NEWLINE, LPRT_CPY, text);

	lRenderEffectReset(hw, LFTW_UNICODE, LTR_SHADOW);
	lSetRenderEffect(hw, LTR_SHADOW);
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_N|LTRA_SHADOW_S|LTRA_SHADOW_E|LTRA_SHADOW_W | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(0) | LTRA_SHADOW_TR(40));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0xFFFFFF);	// set shadow colour (default is 0x00000)
	lPrintEx(frame, &trect, LFTW_UNICODE, flags|PF_NEWLINE, LPRT_CPY, text);	
	
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetRenderEffect(hw, LTR_DEFAULT);
	lPrintEx(frame, &trect, LFTW_UNICODE, flags|PF_NEWLINE, LPRT_CPY, text);
	
	lRenderEffectReset(hw, LFTW_UNICODE, LTR_SHADOW);
	lSetRenderEffect(hw, LTR_SHADOW);
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_N|LTRA_SHADOW_S|LTRA_SHADOW_E|LTRA_SHADOW_W | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(0) | LTRA_SHADOW_TR(40));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0x000000);
	lPrintEx(frame, &trect, LFTW_UNICODE, flags|PF_NEWLINE, LPRT_CPY, text);

	lRenderEffectReset(hw, LFTW_UNICODE, LTR_SHADOW);
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_N|LTRA_SHADOW_S|LTRA_SHADOW_E|LTRA_SHADOW_W | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(1) | LTRA_SHADOW_TR(30));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0xFFFFFF);
	lPrintEx(frame, &trect, LFTW_UNICODE, flags|PF_NEWLINE, LPRT_CPY, text);
	
	lRenderEffectReset(hw, LFTW_UNICODE, LTR_SHADOW);
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	// set direction to South-East, shadow thickness to 3, offset by 2 pixels and transparency to 31% (79 = 0.31*255)
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_S|LTRA_SHADOW_E | LTRA_SHADOW_S3 | LTRA_SHADOW_OS(2) | LTRA_SHADOW_TR(79));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0x000000);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	lRenderEffectReset(hw, LFTW_UNICODE, LTR_SHADOW);
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_S|LTRA_SHADOW_E | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(5) | LTRA_SHADOW_TR(60));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0xF0F000);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	trect.ey += 10;

	lRenderEffectReset(hw, LFTW_UNICODE, LTR_SHADOW);
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_N|LTRA_SHADOW_S|LTRA_SHADOW_E|LTRA_SHADOW_W | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(5) | LTRA_SHADOW_TR(40));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0xFFFFFF);	// set shadow colour (default is 0x00000)
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	trect.ey += 8;
	
	lRenderEffectReset(hw, LFTW_UNICODE, LTR_SHADOW);
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_S|LTRA_SHADOW_W | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(5) | LTRA_SHADOW_TR(40));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0xFFFFFF);	// set shadow colour (default is 0x00000)
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_CPY, text);

	trect.ey += 8;
	
	lSetRenderEffect(hw, LTR_FLIPH);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

	lSetRenderEffect(hw, LTR_FLIPV);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);
		
	lSetRenderEffect(hw, LTR_180);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);
#endif



	int colour = 0x000000;		// 24bit only
	int alpha = 0.80 * 1000.0;	// alpha range of 0 to 1000
	int radius = 3;				// 0 - 255

#if 1

//	000
//	0#0			per pixel blur addition
//	000
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR1);
	lSetFilterAttribute(hw, LTR_BLUR1, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR1, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR1, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR1, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR1, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR1, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

//	#00
//	000
//	#00
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR2);
	lSetFilterAttribute(hw, LTR_BLUR2, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR2, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR2, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR2, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR2, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR2, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

//	00#
//	000
//	00#
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR3);
	lSetFilterAttribute(hw, LTR_BLUR3, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR3, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR3, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR3, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR3, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR3, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

//	#0#
//	000
//	000
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR6);
	lSetFilterAttribute(hw, LTR_BLUR6, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR6, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR6, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR6, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR6, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR6, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);
	
//	000
//	000
//	#0#
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR7);
	lSetFilterAttribute(hw, LTR_BLUR7, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR7, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR7, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR7, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR7, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR7, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

		
//	0#0
//	#0#
//	0#0
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR4);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);
	
//	#0#
//	000
//	#0#
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR5);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

//	#00
//	000
//	00#
	trect.ey += 8;
	lSetRenderEffect(hw, LTR_BLUR8);
	lSetFilterAttribute(hw, LTR_BLUR8, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR8, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR8, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR8, LTRA_BLUR_X, 2);
	lSetFilterAttribute(hw, LTR_BLUR8, LTRA_BLUR_Y, 2);
	lSetFilterAttribute(hw, LTR_BLUR8, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

#if 1
	trect.ey += 8;
	colour = 0xFF10CF;
	lRenderEffectReset(hw, LFTW_ROUGHT18, LTR_BLUR4);	// reset previous blur4 state
	lSetRenderEffect(hw, LTR_BLUR4);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);
	lSetRenderEffect(hw, LTR_DEFAULT);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags, LPRT_OR, text);

	trect.ey += 8;
	colour = 0xFFFFFF;
	radius = 2;
	alpha = 1000;
	lRenderEffectReset(hw, LFTW_ROUGHT18, LTR_BLUR4);
	lSetRenderEffect(hw, LTR_BLUR4);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);

	trect.ey += 8;
	colour = 0x000000;
	radius = 1;
	alpha = 900;
	lSetRenderEffect(hw, LTR_BLUR4);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_ALPHA, alpha);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);
	lSetRenderEffect(hw, LTR_DEFAULT);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags, LPRT_OR, text);
#endif

	lSetForegroundColour(hw, 255<<24 | lGetRGBMask(frame, LMASK_BLACK));
	
	// black on white
	trect.ey += 12;
	alpha = 0.80 * 1000.0;	// alpha range of 0 to 1000
	radius = 3;				// 0 - 255
	lRenderEffectReset(hw, LFTW_ROUGHT18, LTR_BLUR5);
	lSetRenderEffect(hw, LTR_BLUR5);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_COLOUR, lGetRGBMask(frame, LMASK_WHITE));
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_SETTOP, 0);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_ALPHA, alpha*0.75);
	// white base/shadow
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags|PF_NEWLINE, LPRT_OR, text);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR5, LTRA_BLUR_ALPHA, 0);
	//lSetFilterAttribute(hw, LTR_BLUR8, LTRA_BLUR_COLOUR, 0x00FF00);
	// black top
	lPrintEx(frame, &trect, LFTW_ROUGHT18, flags, LPRT_OR, text);
	
#endif

#if 1
	// draw a multi layered string
	drawMultilayer1(frame, 0, 272*2+270, LFTW_B34, text, render);
	drawMultilayer2(frame, 0, 272*3+110, LFTW_B34, text, render);
#endif


#endif
/*
	lSetFontCharacterSpacing(hw, LFTW_ROUGHT18, lGetFontCharacterSpacing(hw, LFTW_ROUGHT18)+1);
	lSetForegroundColour(hw, 255<<24 | lGetRGBMask(frame, LMASK_BLACK));

	colour = 0xFFFFFF;
	radius = 2;
	alpha = 900;
	lRenderEffectReset(hw, LFTW_ROUGHT18, LTR_BLUR4);
	lSetRenderEffect(hw, LTR_BLUR4);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_COLOUR, colour);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_RADIUS, radius);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_X, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_Y, 0);
	lSetFilterAttribute(hw, LTR_BLUR4, LTRA_BLUR_ALPHA, alpha);
	TFRAME *img = lNewString(frame->hw, frame->bpp, flags, LFTW_ROUGHT18, text);
	if (img){
		lDrawImage(img, frame, abs(img->width-frame->width)/2, 272*3+215);
		lDeleteFrame(img);
	}
*/
}


void doTest (TFRAME *frame)
{
	lSetForegroundColour(hw, 255<<24 | 0x000000);
	lSetBackgroundColour(hw, 255<<24 | 0xFFFFFF);
	int flags = /*PF_WORDWRAP|*/PF_CLIPWRAP|PF_CLIPDRAW|PF_TEXTBOUNDINGBOX|PF_GLYPHBOUNDINGBOX;
	
	char *str = "TFRAME *img = lNewString(frame->hw, frame->bpp, flags, LFTW_ROUGHT18, text);";	
		
#if 1
	TMETRICS metrics = {0, 0, frame->width, 0};
	TFRAME *frm = lNewStringEx(hw, &metrics, LFRM_BPP_32A, flags, LFTW_B34, str);
	if (!frm) return;
	
	lDrawImage(frm, frame, 0, 4);
	lDeleteFrame(frm);
#else

	TLPRINTR rect = {4, 4};
	lPrintEx(frame, &rect, LFTW_B34, flags, LPRT_CPY, str);

#endif
}


int main (int argc, char* argv[])
{
	if (!utilInitConfig("config.cfg"))
		return 0;

	//lSetCharacterEncoding(hw, CMT_UTF8);	
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, 255<<24 | 0xFFFFFF);
	lResizeFrame(frame, 480, 272*4, 0);


#if 0
	lClearFrameClr(frame, 0xFFFFFFFF);

#else
	lClearFrame(frame);
	
	int r,g,b;
    for (int y = 0; y < frame->height; y++){
    	for (int x = 0; x < frame->width; x++){
    		if (frame->bpp == LFRM_BPP_12){
       			r = x>>3;
				g = y>>3;
				b = 8; //(15-(x>>3))&0x0F;
				lSetPixel(frame, x+1, y+1, (r<<8)|(g<<4)|b);	// 12bit

			}else if (frame->bpp == LFRM_BPP_32A || frame->bpp == LFRM_BPP_32){
       			r = (x>>1)&0xFF;
				g = y&0xFF;
				b = 127;
				lSetPixel(frame, x+1, y+1, 0xFF000000|(r<<16)|(g<<8)|b);	// 32bit
				
			}else if (frame->bpp == LFRM_BPP_24){
       			r = (x>>1)&0xFF;
				g = y&0xFF;
				b = 127;
				lSetPixel(frame, x+1, y+1, (r<<16)|(g<<8)|b);	// 24bit
												
			}else if (frame->bpp == LFRM_BPP_16){
       			r = (x>>4)&0x1F;
				g = (y>>2)&0x3F;
				b = 16&0x1F;
				lSetPixel(frame, x+1, y+1, (r<<11)|(g<<5)|b);	// 16bit
			}else{
				r = (x/16);
				g = (y/16);
				b = 1;
				frame->pops->set(frame, x, y, r<<5|g<<2|b);	// 8bit
			}
	    }
	}
#endif


	//doPrint(frame, 0);	// preload fonts
	//doPrint(frame, 1);
	//doTest(frame);	

	int n = 0;
	for (n = 0; n < 1; n++)
		doPrint(frame, 1);

	//lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, 0xFF000000|lGetRGBMask(frame, LMASK_WHITE));


	lRefresh(frame);
	lSaveImage(frame, L"quickfox.png", IMG_PNG, 0, 0);
	
	lSleep(2000);
	utilCleanup();
	return 1;
}
