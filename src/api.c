
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
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


#if (defined(__WIN32__))
  #include <windows.h>		// needed for Sleep()
#endif

#include "mylcd.h"
#include "lcd.h"
#include "memory.h"
#include "apilock.h"
#include "device.h"
#include "lstring.h"
#include "pixel.h"
#include "frame.h"
#include "display.h"
#include "misc.h"
#include "copy.h"
#include "utils.h"
#include "bdf.h"
#include "print.h"
#include "cmap.h"
#include "fonts.h"
#include "fileio.h"
#include "chardecode.h"
#include "textbdf.h"
#include "image.h"
#include "scroll.h"
#include "draw.h"
#include "convert.h"
#include "rotate.h"
#include "api.h"


/*****************************/
/*********** lcd.c ***********/
MYLCD_EXPORT THWD *lOpen (const wchar_t *fontPath, const wchar_t *mapPath)
{
	return openLib(fontPath, mapPath);
}

MYLCD_EXPORT void lClose (THWD *hw)
{
	if (!hw) return;
#if (__BUILD_APISYNC_SUPPORT__)
	lock((TTHRDSYNCCTRL*)hw->sync);
#endif
	closeLib(hw);

}


/******************************/
/*********** draw.c ***********/
MYLCD_EXPORT int lDrawImage (TFRAME *src, TFRAME *des, const int x, const int y)
{
	API_LOCK(src->hw->sync)
	int ret = copyAreaEx(src, des, x, y, 0, 0, src->width-1, src->height-1, 1, 1, LCASS_CPY);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lDrawLineDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawLineDotted(frm, x1, y1, x2, y2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawRectangleDottedFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawRectangleDottedFilled(frm, x1, y1, x2, y2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawRectangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawRectangleFilled(frm, x1, y1, x2, y2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawRectangleDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawRectangleDotted(frm, x1, y1, x2, y2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawRectangle (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawRectangle(frm, x1, y1, x2, y2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lInvertFrame (TFRAME *frm)
{
	API_LOCK(frm->hw->sync)
	int ret = invertFrame(frm);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lInvertArea (TFRAME *frm, int x1, int y1, int x2, int y2)
{
	API_LOCK(frm->hw->sync)
	int ret = invertArea(frm, x1, y1, x2, y2);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawCircle (TFRAME *frm, int xc, int yc, int radius, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawCircle(frm, xc, yc, radius, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawMask (TFRAME *src, TFRAME *mask, TFRAME *des, int maskXOffset, int maskYOffset, int mode)
{
	API_LOCK(src->hw->sync)
	int ret = drawMask(src, mask, des, maskXOffset, maskYOffset, mode);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lDrawMaskA (const TFRAME *src, const TFRAME *mask, TFRAME *des, const int Xoffset, const int Yoffset, const float alpha)
{
	API_LOCK(src->hw->sync)
	int ret = drawMaskA(src, mask, des, Xoffset, Yoffset, 0, 0, src->width-1, src->height-1, alpha);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lDrawMaskAEx (const TFRAME *src, const TFRAME *mask, TFRAME *des, const int Xoffset, const int Yoffset, const int srcX1, const int srcY1, const int srcX2, const int srcY2, const float alpha)
{
	API_LOCK(src->hw->sync)
	int ret = drawMaskA(src, mask, des, Xoffset, Yoffset, srcX1, srcY1, srcX2, srcY2, alpha);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lDrawCircleFilled (TFRAME *frm, int xc, int yc, int radius, int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawCircleFilled(frm, xc, yc, radius, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawEnclosedArc (TFRAME *frm, int x, int y, int r1, int r2, double a1, double a2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawEnclosedArc(frm, x, y, r1, r2, a1, a2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawArc (TFRAME *frm, int x, int y, int r1, int r2, double a1, double a2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawArc(frm, x, y, r1, r2, a1, a2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawLine (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawLine(frm, x1, y1, x2, y2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawTriangle (TFRAME *frm, int x1, int y1, int x2, int y2, int x3, int y3, int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawTriangle(frm, x1, y1, x2, y2, x3, y3, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawTriangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, int x3, int y3, int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawTriangleFilled(frm, x1, y1, x2, y2, x3, y3, colour);
	API_UNLOCK_RET(frm->hw->sync)
}


MYLCD_EXPORT int lDrawEllipse (TFRAME *frm, int x, int y, int r1, int r2, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = drawEllipse(frm, x, y, r1, r2, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawPolyLineTo (TFRAME *frm, TPOINTXY *pts, int tPoints, const int colour)
{
	if (tPoints <= 1) return 0;
	API_LOCK(frm->hw->sync)
	int ret = drawPolyLineTo(frm, pts, tPoints, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawPolyLineDottedTo (TFRAME *frm, T2POINT *pt, int tPoints, const int colour)
{
	if (tPoints <= 1) return 0;
	API_LOCK(frm->hw->sync)
	int ret = drawPolyLineDottedTo(frm, pt, tPoints, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawPolyLine (TFRAME *frm, T2POINT *pt, int tPoints, const int colour)
{
	if (tPoints <= 1) return 0;
	API_LOCK(frm->hw->sync)
	int ret = drawPolyLine(frm, pt, tPoints, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lDrawPolyLineEx (TFRAME *frm, TLPOINTEX *pt, int n, const int colour)
{
	if (n <= 1) return 0;
	API_LOCK(frm->hw->sync)
	int ret = drawPolyLineEx(frm, pt, n, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lFloodFill (TFRAME *frame, const int x, const int y, const int colour)
{
	API_LOCK(frame->hw->sync)
	int ret = floodFill(frame, x, y, colour);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT int lEdgeFill (TFRAME *frame, const int x, const int y, const int fillColour, const int edgeColour)
{
	API_LOCK(frame->hw->sync)
	int ret = edgeFill(frame, x, y, fillColour, edgeColour);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT int lBlurArea (TFRAME *frame, const int x1, const int y1, const int x2, const int y2, const int radius_iter)
{
	API_LOCK(frame->hw->sync)
	int ret = blurArea(frame, x1, y1, x2, y2, radius_iter);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT int lBlurImage (TFRAME *frame, const int blurOp, const int radius_iter)
{
	API_LOCK(frame->hw->sync)
	int ret = blurImage(frame, blurOp, radius_iter);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT int lConvolve2DSeparable (TFRAME *const src, TFRAME *des, const float* const kernelX, const int kSizeX, const float* const kernelY, const int kSizeY)
{
	API_LOCK(src->hw->sync)
	int ret = convolve2DSeparable(src, des, kernelX, kSizeX, kernelY, kSizeY);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lConvolve2D (TFRAME *src, TFRAME *des, float *const kernel, const int kSizeX, const int kSizeY)
{
	API_LOCK(src->hw->sync)
	int ret = convolve2D(src, des, kernel, kSizeX, kSizeY);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lBlur (TFRAME *src, TFRAME *workingBuffer, const int applyAlpha, const int radius)
{
	API_LOCK(src->hw->sync)
	int ret = blur(src, workingBuffer, applyAlpha, radius);
	API_UNLOCK_RET(src->hw->sync)
}


/********************************/
/*********** device.c ***********/
MYLCD_EXPORT lDISPLAY lSelectDevice (THWD *hw, const char *displayd, const char *portd, int width, int height, int bpp, int data, TRECT *rect)
{
	if (hw && portd && displayd && rect){
		API_LOCK(hw->sync)
		lDISPLAY ret = selectDevice(hw, displayd, portd, width, height, bpp, data, rect);
		API_UNLOCK_RET(hw->sync)
	}else{
		return 0;
	}
}

MYLCD_EXPORT int lSetCapabilities (THWD *hw, unsigned int flag, int value)
{
	if (flag && (flag < CAPS_TOTAL)){
		API_LOCK(hw->sync)
		int ret = setCapabilities(hw, flag, value);
		API_UNLOCK_RET(hw->sync)
	}else{
		return -1;
	}
}

MYLCD_EXPORT int lGetCapabilities (THWD *hw, unsigned int flag)
{
	if (flag && (flag < CAPS_TOTAL)){
		API_LOCK(hw->sync)
		int ret = getCapabilities(hw, flag);
		API_UNLOCK_RET(hw->sync)
	}else{
		return -1;
	}
}

MYLCD_EXPORT TDRIVER * lDisplayIDToDriver (THWD *hw, lDISPLAY did)
{
	API_LOCK(hw->sync)
	TDRIVER *ret = displayIDToDriver(hw, did);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT TREGDRV * lEnumerateDriversBegin (THWD *hw, int edrv)
{
	API_LOCK(hw->sync)
	TREGDRV *ret = enumerateDriversBegin(hw, edrv);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lEnumerateDriverNext (TREGDRV *d)
{
	API_LOCK(d->hw->sync)
	void *sync = d->hw->sync;
	int ret = enumerateDriverNext(d);
	API_UNLOCK_RET(sync)
}

MYLCD_EXPORT void lEnumerateDriverEnd (TREGDRV *d)
{
	API_LOCK(d->hw->sync)
	void *sync = d->hw->sync;
	enumerateDriverEnd(d);
	API_UNLOCK(sync)
}

MYLCD_EXPORT int lCloseDevice (THWD *hw, const lDISPLAY did)
{
	API_LOCK(hw->sync)
	int ret = closeDeviceId(hw, did);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT lDISPLAY lDriverNameToID (THWD *hw, const char *name, int ldrv_type)
{
	if (!name) return 0;
	API_LOCK(hw->sync)
	lDISPLAY ret = driverNameToID(hw, name, ldrv_type);
	API_UNLOCK_RET(hw->sync)
}


/*********************************/
/*********** display.c ***********/
MYLCD_EXPORT int lClearDisplay (THWD *hw)
{
	API_LOCK(hw->sync)
	int ret = clearDisplay(hw);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lUpdate (const THWD *hw, const void *buffer, const size_t bsize)
{
	if (!bsize) return 0;
	API_LOCK(hw->sync)
	int ret = update(hw, buffer, bsize);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lRefreshArea (TFRAME *frm, const int left, const int top, const int right, const int btm)
{
	API_LOCK(frm->hw->sync)
#ifdef __DEBUG__
	frm->hw->render->framesSubmitted++;
#endif
	int ret = refreshFrameArea(frm, left, top, right, btm);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lRefresh (TFRAME *frm)
{
	API_LOCK(frm->hw->sync)
#ifdef __DEBUG__
	frm->hw->render->framesSubmitted++;
#endif
	int ret = refreshFrame(frm);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lRefreshAsync (TFRAME *frm, const int swap)
{
	API_LOCK(frm->hw->sync)
#ifdef __DEBUG__
	frm->hw->render->framesSubmitted++;
#endif
	int ret = refreshAsync(frm, swap);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lSetDisplayOption (THWD *hw, lDISPLAY did, int option, intptr_t *value)
{
	intptr_t stub;
	if (!value) value = &stub;
	API_LOCK(hw->sync)
	int ret = setDisplayOption(hw, did, option, value);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetDisplayOption (THWD *hw, lDISPLAY did, int option, intptr_t *value)
{
	if (!value) return -1;
	API_LOCK(hw->sync)
	int ret = getDisplayOption(hw, did, option, value);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lPauseDisplay (THWD *hw, lDISPLAY did)
{
	API_LOCK(hw->sync)
	int ret = pauseDisplay(hw, did);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lResumeDisplay (THWD *hw, lDISPLAY did)
{
	API_LOCK(hw->sync)
	int ret = resumeDisplay(hw, did);
	API_UNLOCK_RET(hw->sync)
}


/******************************/
/*********** cmap.c ***********/
MYLCD_EXPORT int lEnumLanguageTables (THWD *hw, char **alias, int *langID, wchar_t **filepath)
{
	API_LOCK(hw->sync)
	int ret = enumLanguageTables(hw, alias, langID, filepath);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lEncodingAliasToID (THWD *hw, const char *name)
{
	API_LOCK(hw->sync)
	int ret = encodingAliasToID(hw, name);
	API_UNLOCK_RET(hw->sync)
}


/************************************/
/*********** chardecode.c ***********/
MYLCD_EXPORT int lSetCharacterEncoding (THWD *hw, int tableId)
{
	API_LOCK(hw->sync)
	int ret = setCharacterEncodingTable(hw, tableId);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetCharacterEncoding (THWD *hw)
{
	API_LOCK(hw->sync)
	int ret = getCharEncoding(hw);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT void lHTMLCharRefEnable (THWD *hw)
{
	API_LOCK(hw->sync)
	htmlCharRefEnable(hw);
	API_UNLOCK(hw->sync)
}

MYLCD_EXPORT void lHTMLCharRefDisable (THWD *hw)
{
	API_LOCK(hw->sync)
	htmlCharRefDisable(hw);
	API_UNLOCK(hw->sync)
}

MYLCD_EXPORT void lCombinedCharEnable (THWD *hw)
{
	API_LOCK(hw->sync)
	combinedCharEnable(hw);
	API_UNLOCK(hw->sync)
}

MYLCD_EXPORT void lCombinedCharDisable (THWD *hw)
{
	API_LOCK(hw->sync)
	combinedCharDisable(hw);
	API_UNLOCK(hw->sync)
}

MYLCD_EXPORT int lCreateCharacterList (THWD *hw, const char *str, UTF32 *glist, int total)
{
	if (!str) return 0;
	API_LOCK(hw->sync)
	int ret = createCharacterList(hw, str, glist, total);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lDecodeCharacterCode (THWD *hw, const char *buffer, UTF32 *ch)
{
	if (!buffer || !ch) return 0;
	API_LOCK(hw->sync)
	int ret = decodeCharacterCode(hw, (ubyte*)buffer, ch);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lDecodeCharacterBuffer (THWD *hw, const char *buffer, UTF32 *glist, int total)
{
	if (!buffer || !glist || !total) return 0;
	API_LOCK(hw->sync)
	int ret = decodeCharacterBuffer(hw, buffer, glist, total);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lCountCharacters (THWD *hw, const char *buffer)
{
	if (!buffer) return 0;
	API_LOCK(hw->sync)
	int ret = countCharacters(hw, buffer);
	API_UNLOCK_RET(hw->sync)
}


/*********************************/
/*********** convert.c ***********/
MYLCD_EXPORT pConverterFn lGetConverter (THWD *hw, int src_bpp, int des_bpp)
{
	API_LOCK(hw->sync)
	pConverterFn ret = getConverter(src_bpp, des_bpp);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT unsigned int lFrame1BPPToRGB (TFRAME *frame, void *des, int des_bpp, const int clrLow, const int clrHigh)
{
	API_LOCK(frame->hw->sync)
	int ret = frame1BPPToRGB(frame, des, des_bpp, clrLow, clrHigh);
	API_UNLOCK_RET(frame->hw->sync)
}


/********************************/
/*********** rotate.c ***********/
MYLCD_EXPORT int lRotateFrameEx (TFRAME *src, TFRAME *des, const float xang, const float yang, const float zang, float flength, const float zoom, const int destx, const int desty)
{
	API_LOCK(src->hw->sync)
	int ret = rotateFrameEx(src, des, xang, yang, zang, flength, zoom, destx, desty);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lRotate (TFRAME *src, TFRAME *des, const int destx, const int desty, double angle)
{
	API_LOCK(src->hw->sync)
	int ret = rotate(src, des, destx, desty, angle);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lRotateFrameL90 (TFRAME *frm)
{
	API_LOCK(frm->hw->sync)
	int ret = rotateFrameL90(frm);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lRotateFrameR90 (TFRAME *frm)
{
	API_LOCK(frm->hw->sync)
	int ret = rotateFrameR90(frm);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT void lRotateX (const float angle, const float y, const float z, float *yr, float *zr)
{
	rotateX(angle, y, z, yr, zr);
}

MYLCD_EXPORT void lRotateY (const float angle, const float x, const float z, float *xr, float *zr)
{
	rotateY(angle, x, z, xr, zr);
}

MYLCD_EXPORT void lRotateZ (const float angle, const float x, const float y, float *xr, float *yr)
{
	rotateZ(angle, x, y, xr, yr);
}

MYLCD_EXPORT void lPoint3DTo2D (const float x, const float y, const float z, const float flength, const float camz, const int x0, const  int y0, int *screenx, int *screeny)
{
	point3DTo2D(x, y, z, flength, camz, x0, y0, screenx, screeny);
}


/******************************/
/*********** copy.c ***********/
MYLCD_EXPORT int lCopyFrame (TFRAME *from, TFRAME *to)
{
	API_LOCK(from->hw->sync)
	int ret = copyFrame(from, to);
	API_UNLOCK_RET(from->hw->sync)
}

MYLCD_EXPORT int lCopyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	API_LOCK(from->hw->sync)
	int ret = copyArea(from, to, dx, dy, x1, y1, x2, y2);
	API_UNLOCK_RET(from->hw->sync)
}

MYLCD_EXPORT int lCopyAreaA (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, float alpha)
{
	API_LOCK(from->hw->sync)
	int ret = copyAreaA(from, to, dx, dy, x1, y1, x2, y2, alpha);
	API_UNLOCK_RET(from->hw->sync)
}

MYLCD_EXPORT int lCopyAreaScaled (TFRAME *from, TFRAME *to, int src_x, int src_y, int src_width, int src_height, int dest_x, int dest_y, int dest_width, int dest_height, int mode)
{
	API_LOCK(from->hw->sync)
	int ret = copyAreaScaled(from, to, src_x, src_y, src_width, src_height, dest_x, dest_y, dest_width, dest_height, mode);
	API_UNLOCK_RET(from->hw->sync)
}

MYLCD_EXPORT int lCopyAreaEx (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int scaleh, int scalev, const int colour)
{
	API_LOCK(from->hw->sync)
	int ret = copyAreaEx(from, to, dx, dy, x1, y1, x2, y2, scaleh, scalev, colour);
	API_UNLOCK_RET(from->hw->sync)
}

MYLCD_EXPORT int lFlipFrame (TFRAME *src, TFRAME *des, int flags)
{
	API_LOCK(src->hw->sync)
	int ret = flipFrame(src, des, flags);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lMoveArea (TFRAME *frm, int x1, int y1, int x2, int y2, int tpixels, const int mode, const int dir)
{
	API_LOCK(frm->hw->sync)
	int ret = moveArea(frm, x1, y1, x2, y2, tpixels, mode, dir);
	API_UNLOCK_RET(frm->hw->sync)
}

/*******************************/
/*********** image.c ***********/
MYLCD_EXPORT int lSaveImage (TFRAME *frame, const wchar_t *filename, const int img_type, int w, int h)
{
	API_LOCK(frame->hw->sync)
	int ret = saveImage(frame, filename, img_type, w, h);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT TFRAME *lNewImage (THWD *hw, const wchar_t *filename, const int frameBPP)
{
	API_LOCK(hw->sync)
	TFRAME *ret = newImage(hw, filename, frameBPP);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lLoadImage (TFRAME *frame, const wchar_t *filename)
{
	API_LOCK(frame->hw->sync)
	int ret = loadImage(frame, filename);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT int lLoadImageEx (TFRAME *frame, const wchar_t *filename, const int flags, const int ox, const int oy)
{
	API_LOCK(frame->hw->sync)
	int ret = loadImageEx(frame, filename, flags, ox, oy);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT int lImageGetMetrics (const wchar_t *filename, int type, int *width, int *height, int *bpp)
{
	return imageGetMetrics(filename, type, width, height, bpp);
}



/********************************/
/*********** scroll.c ***********/
MYLCD_EXPORT TLSCROLLEX * lNewScroll (TFRAME *src, TFRAME *client)
{
	API_LOCK(src->hw->sync)
	TLSCROLLEX *ret = newScroll(src, client);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT int lDrawScroll (TLSCROLLEX *scroll)
{
	API_LOCK(scroll->hw->sync)
	int ret = scrCopyToClient(scroll);
	API_UNLOCK_RET(scroll->hw->sync)
}

MYLCD_EXPORT void lDeleteScroll (TLSCROLLEX *scroll)
{
	API_LOCK(scroll->hw->sync)
	void *sync = scroll->hw->sync;
	deleteScroll(scroll);
	API_UNLOCK(sync)
}

MYLCD_EXPORT int lUpdateScroll (TLSCROLLEX *s)
{
	API_LOCK(s->hw->sync)
	int ret = updateScroll(s);
	API_UNLOCK_RET(s->hw->sync)
}


/*******************************/
/*********** frame.c ***********/
MYLCD_EXPORT int lDeleteFrame (TFRAME *frm)
{
	API_LOCK(frm->hw->sync)
	void *sync = frm->hw->sync;
	deleteFrame(frm);
	API_UNLOCK(sync)
	return 1;//remove me
}

MYLCD_EXPORT void lDeleteFrames (THWD *hw, const int groupId)
{
	API_LOCK(hw->sync)
	deleteFrames(hw, groupId);
	API_UNLOCK(hw->sync)
}

MYLCD_EXPORT int lClearFrame (TFRAME *frm)
{
	API_LOCK(frm->hw->sync)
	int ret = clearFrame(frm);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lClearFrameClr (TFRAME *frm, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = clearFrameClr(frm, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT TFRAME * lCloneFrame (TFRAME *src)
{
	if (src == NULL) return NULL;
	API_LOCK(src->hw->sync)
	TFRAME *ret = cloneFrame(src);
	API_UNLOCK_RET(src->hw->sync)
}

MYLCD_EXPORT TFRAME * lNewFrame (THWD *hw, int width, int height, ubyte bpp)
{
	API_LOCK(hw->sync)
	TFRAME *ret = newFrame(hw, width, height, bpp);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lResizeFrame (TFRAME *frm, int width, int height, int mode)
{
	API_LOCK(frm->hw->sync)
	int ret = resizeFrame(frm, width, height, mode);
	API_UNLOCK_RET(frm->hw->sync)
}


/*******************************/
/*********** pixel.c ***********/
MYLCD_EXPORT void lSetPixelWriteMode (TFRAME *frm, ubyte mode)
{
	API_LOCK(frm->hw->sync)
	frm->style = mode&0x07;
	API_UNLOCK(frm->hw->sync)
}

MYLCD_EXPORT int lGetPixelWriteMode (TFRAME *frm)
{
	API_LOCK(frm->hw->sync)
	int ret = frm->style;
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lGetRGBMask (const TFRAME *frame, const int colourMaskIdx)
{
	API_LOCK(frame->hw->sync)
	int ret = getRGBMask(frame, colourMaskIdx);
	API_UNLOCK_RET(frame->hw->sync)
}

MYLCD_EXPORT int lGetImageBoundingBox (TFRAME *frame, TLPOINTEX *p)
{
	if (!frame || !p) return 0;
	API_LOCK(frame->hw->sync)
	int ret = getImageBoundingBox(frame, p);
	API_UNLOCK_RET(frame->hw->sync)
}


/*********************************/
/*********** textbdf.c ***********/
MYLCD_EXPORT int lSetRenderEffect (THWD *hw, int mode)
{
	if (mode < 0 || mode >= LTR_TOTAL)
		mode = LTR_DEFAULT;
	API_LOCK(hw->sync)
	int ret = setRenderEffect(hw, mode);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT void lRenderEffectReset (THWD *hw, const int fontid, const int mode)
{
	API_LOCK(hw->sync)
	renderEffectReset(hw, fontid, mode);
	API_UNLOCK(hw->sync)
}

MYLCD_EXPORT int lCacheCharacterBuffer (THWD *hw, const UTF32 *glist, int total, int fontid)
{
	if (!glist || !total) return -1;
	API_LOCK(hw->sync)
	int ret = cacheCharacterBuffer(hw, glist, total, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetTextMetricsList (THWD *hw, const UTF32 *glist, int first, int last, int flags, int fontid, TLPRINTR *rect)
{
	if (!glist) return -1;
	API_LOCK(hw->sync)
	int ret = getTextMetricsList(hw, glist, first, last, flags, fontid, rect);
	API_UNLOCK_RET(hw->sync)
}


/*******************************/
/*********** print.c ***********/
MYLCD_EXPORT int lGetCharMetrics (THWD *hw, const char *c, int fontid, int *w, int *h)
{
	API_LOCK(hw->sync)
	int ret = getCharMetrics(hw, c, fontid, w, h);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetFontMetrics (THWD *hw, int fontid, int *w, int *h, int *a, int *d, int *tchars)
{
	API_LOCK(hw->sync)
	int ret = getFontMetrics(hw, fontid, w, h, a, d, tchars);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetTextMetrics (THWD *hw, const char *txt, int flags, int fontid, int *w, int *h)
{
	API_LOCK(hw->sync)
	int ret = getTextMetrics(hw, txt, flags, fontid, w, h);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lPrint (TFRAME *frm, const char *buffer, int x, int y, int font, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = _print(frm, buffer, x, y, font, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lPrintEx (TFRAME *frm, TLPRINTR *rect, int fontid, int flags, int style, const char *string, ...)
{
	API_LOCK(frm->hw->sync)
	int ret;
	VA_OPEN(ap, string);
	ret = _printEx(frm, rect, fontid, flags, style, string, &ap);
	VA_CLOSE(ap);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lPrintf (TFRAME *frm, int x, int y, int font, int style, const char *string, ...)
{
	API_LOCK(frm->hw->sync)
	int ret;
	VA_OPEN(ap, string);
	ret = _printf(frm, x, y, font, style, string, &ap);
	VA_CLOSE(ap);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT int lPrintList (TFRAME *frm, const UTF32 *glist, int first, int total, TLPRINTR *rect, int fontid, int flags, const int colour)
{
	API_LOCK(frm->hw->sync)
	int ret = _printList(frm, glist, first, total, rect, fontid, flags, colour);
	API_UNLOCK_RET(frm->hw->sync)
}

MYLCD_EXPORT TFRAME * lNewStringEx (THWD *hw, const TMETRICS *metrics, const int lfrm_bpp, const int flags, const int font, const char *string, ...)
{
	API_LOCK(hw->sync)
	TFRAME *ret;
	VA_OPEN(ap, string);
	ret = newStringEx(hw, metrics, lfrm_bpp, flags, font, string, &ap);
	VA_CLOSE(ap);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT TFRAME * lNewString (THWD *hw, const int lfrm_bpp, const int flags, const int font, const char *string, ...)
{
	API_LOCK(hw->sync)
	TFRAME *ret;
	VA_OPEN(ap, string);
	ret = newString(hw, lfrm_bpp, flags, font, string, &ap);
	VA_CLOSE(ap);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT TFRAME * lNewStringList (THWD *hw, const int lfrm_bpp, const int flags, const int font, const UTF32 *glist, const int gtotal)
{
	API_LOCK(hw->sync)
	TFRAME *ret = newStringList(hw, lfrm_bpp, flags, font, glist, gtotal);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT TFRAME * lNewStringListEx (THWD *hw, const TMETRICS *metrics, const int lfrm_bpp, const int flags, const int font, const UTF32 *glist, const int gtotal)
{
	API_LOCK(hw->sync)
	TFRAME *ret = newStringListEx(hw, metrics, lfrm_bpp, flags, font, glist, gtotal);
	API_UNLOCK_RET(hw->sync)
}

/*******************************/
/*********** fonts.c ***********/
MYLCD_EXPORT int lCacheCharactersAll (THWD *hw, int fontid)
{
	API_LOCK(hw->sync)
	int ret = cacheCharactersAll(hw, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lCacheCharacters (THWD *hw, const char *str, int fontid)
{
	API_LOCK(hw->sync)
	int ret = cacheCharacters(hw, str, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lCacheCharacterList (THWD *hw, UTF32 *glist, int gtotal, int fontid)
{
	API_LOCK(hw->sync)
	int ret = cacheCharacterList(hw, glist, gtotal, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lCacheCharacterRange (THWD *hw, UTF32 min, UTF32 max, int fontid)
{
	API_LOCK(hw->sync)
	int ret = cacheCharacterRange(hw, min, max, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lStripCharacterList (THWD *hw, UTF32 *glist, int gtotal, TWFONT *font)
{
	API_LOCK(hw->sync)
	int ret = stripCharacterList(hw, glist, gtotal, font);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetFontCharacterSpacing (THWD *hw, int fontid)
{
	API_LOCK(hw->sync)
	int ret = getFontCharacterSpacing(hw, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lSetFontCharacterSpacing (THWD *hw, int fontid, int pixels)
{
	API_LOCK(hw->sync)
	int ret = setFontCharacterSpacing(hw, fontid, pixels);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetFontLineSpacing (THWD *hw, int fontid)
{
	API_LOCK(hw->sync)
	int ret = getFontLineSpacing(hw, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lSetFontLineSpacing (THWD *hw, int fontid, int pixels)
{
	API_LOCK(hw->sync)
	int ret = setFontLineSpacing(hw, fontid, pixels);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lSetFontZeroWidthSpacing (THWD *hw, int fontid, int pixels)
{
	API_LOCK(hw->sync)
	int ret = setFontZeroWidthSpacing(hw, fontid, pixels);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT TWCHAR * lGetGlyph (THWD *hw, const char *c, UTF32 ch, int fontid)
{
	API_LOCK(hw->sync)
	TWCHAR *ret = getGlyph(hw, c, ch, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetCachedGlyphs (THWD *hw, UTF32 *glist, int gtotal, int fontid)
{
	API_LOCK(hw->sync)
	int ret = getCachedGlyphs(hw, glist, gtotal, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lMergeFontW (THWD *hw, int fontid1, int fontid2)
{
	API_LOCK(hw->sync)
	int ret = mergeFontW(hw, fontid1, fontid2);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT void *lFontIDToFont (THWD *hw, int fontid)
{
	API_LOCK(hw->sync)
	void *ret = fontIDToFont(hw, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT void lDeleteFonts (THWD *hw)
{
	API_LOCK(hw->sync)
	deleteFonts(hw);
	API_UNLOCK(hw->sync)
}

MYLCD_EXPORT int lRegisterFontW (THWD *hw, const wchar_t *filename, TWFONT *f)
{
	API_LOCK(hw->sync)
	int ret = registerFontW(hw, filename, f);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lUnregisterFontW (THWD *hw, int fontid)
{
	API_LOCK(hw->sync)
	int ret = unregisterFontW(hw, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lFlushFont (THWD *hw, int fontid)
{
	API_LOCK(hw->sync)
	int ret = flushFont(hw, fontid);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT TENUMFONT *lEnumFontsBeginW (THWD *hw)
{
	API_LOCK(hw->sync)
	TENUMFONT *ret = enumFontsBeginW(hw);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lEnumFontsNextW (TENUMFONT *enf)
{
	API_LOCK(enf->hw->sync)
	int ret = enumFontsNextW(enf);
	API_UNLOCK_RET(enf->hw->sync)
}

MYLCD_EXPORT void lEnumFontsDeleteW (TENUMFONT *enf)
{
	API_LOCK(enf->hw->sync)
	void *sync = enf->hw->sync;
	enumFontsDeleteW(enf);
	API_UNLOCK(sync)
}

MYLCD_EXPORT TENUMFONT *lEnumFontsBeginA (THWD *hw)
{
	API_LOCK(hw->sync)
	TENUMFONT *ret = enumFontsBeginA(hw);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lEnumFontsNextA (TENUMFONT *enf)
{
	API_LOCK(enf->hw->sync)
	int ret = enumFontsNextA(enf);
	API_UNLOCK_RET(enf->hw->sync)
}

MYLCD_EXPORT void lEnumFontsDeleteA (TENUMFONT *enf)
{
	API_LOCK(enf->hw->sync)
	void *sync = enf->hw->sync;
	enumFontsDeleteA(enf);
	API_UNLOCK(sync)
}

MYLCD_EXPORT int lRegisterFontA (THWD *hw, const wchar_t *filename, TFONT *f)
{
	API_LOCK(hw->sync)
	int ret = registerFontA(hw, filename, f);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lUnregisterFontA (THWD *hw, int fontid)
{
	API_LOCK(hw->sync)
	int ret = unregisterFontA(hw, fontid);
	API_UNLOCK_RET(hw->sync)
}


/********************************/
/*********** pixels.c ***********/
#if (__BUILD_PIXELPRIMITIVEEXPORTS__)
MYLCD_EXPORT void lSetPixelf (const TFRAME *frm, const int x, const int y, const float r, const float g, const float b)
{
	return frm->pops->setf(frm, x, y, r, g, b);
}

MYLCD_EXPORT int lGetPixelf (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	return frm->pops->getf(frm, x, y, r, g, b);
}

MYLCD_EXPORT int lGetPixel_NB (const TFRAME *frm, const int x, const int y)
{
	return frm->pops->get_NB(frm, x, y);
}

MYLCD_EXPORT void lSetPixel_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	return frm->pops->set_NB(frm, x, y, value);
}

MYLCD_EXPORT void lSetPixel (const TFRAME *frm, const int x, const int y, const int value)
{
	return frm->pops->set(frm, x, y, value);
}

MYLCD_EXPORT int lGetPixel (const TFRAME *frm, const int x, const int y)
{
	return frm->pops->get(frm, x, y);
}

MYLCD_EXPORT void *lGetPixelAddress (const TFRAME *frm, const int x, const int y)
{
	return frm->pops->getAddr(frm, x, y);
}

MYLCD_EXPORT int lGetPixel_NBr (const TFRAME *frm, const int x, const int row)
{
	return frm->pops->get_NBr(frm, x, row);
}

MYLCD_EXPORT void lSetPixel_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	return frm->pops->set_NBr(frm, x, row, value);
}
#endif


/********************************/
/*********** memory.c ***********/
MYLCD_EXPORT void * my_Malloc (size_t size, const char *func, const int line)
{
#ifdef __DEBUG_MEM__
	return my_malloc(size, func, line);
#else
	return my_malloc(size);
#endif
}

MYLCD_EXPORT void * my_Calloc (size_t nelem, size_t elsize, const char *func, const int line)
{
#ifdef __DEBUG_MEM__
	return my_calloc(nelem, elsize, func, line);
#else
	return my_calloc(nelem, elsize);
#endif
}

MYLCD_EXPORT void * my_Realloc (void *ptr, size_t size, const char *func, const int line)
{
#ifdef __DEBUG_MEM__
	return my_realloc(ptr, size, func, line);
#else
	return my_realloc(ptr, size);
#endif
}

MYLCD_EXPORT void my_Free (void *ptr, const char *func, const int line)
{
#ifdef __DEBUG_MEM__
	my_free(ptr, func, line);
#else
	my_free(ptr);
#endif
}

MYLCD_EXPORT char * my_Strdup (const char *str, const char *func, const int line)
{
	if (str)
#ifdef __DEBUG_MEM__
		return my_strdup(str, func, line);
#else
		return my_strdup(str);
#endif
	else
		return NULL;
}

MYLCD_EXPORT wchar_t * my_Wcsdup (const wchar_t *str, const char *func, const int line)
{
	if (str)
#ifdef __DEBUG_MEM__
		return my_wcsdup(str, func, line);
#else
		return my_wcsdup(str);
#endif
	else
		return NULL;
}

MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n)
{
	l_memcpy(s1, s2, n);
	return s1;
}

MYLCD_EXPORT void my_MemStatsDump (THWD *hw)
{
	dumpMemStats(hw);
}

/****************************/
/*********** misc.c ***********/
MYLCD_EXPORT int lSetForegroundColour (THWD *hw, const int colour)
{
	API_LOCK(hw->sync)
	int ret = setForegroundColour(hw, colour);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lSetBackgroundColour (THWD *hw, const int colour)
{
	API_LOCK(hw->sync)
	int ret = setBackgroundColour(hw, colour);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lSetFilterAttribute (THWD *hw, const int ltr_effect, const int attribute, const int value)
{
	API_LOCK(hw->sync)
	int ret = setFilterAttribute(hw, ltr_effect, attribute, value);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetFilterAttribute (THWD *hw, const int ltr_effect, const int attribute)
{
	API_LOCK(hw->sync)
	int ret = getFilterAttribute(hw, ltr_effect, attribute);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetForegroundColour (THWD *hw)
{
	API_LOCK(hw->sync)
	int ret = getForegroundColour(hw);
	API_UNLOCK_RET(hw->sync)
}

MYLCD_EXPORT int lGetBackgroundColour (THWD *hw)
{
	API_LOCK(hw->sync)
	int ret = getBackgroundColour(hw);
	API_UNLOCK_RET(hw->sync)
}



/****************************/
/*********** misc ***********/
MYLCD_EXPORT void lSleep (int time)
{
	l_sleep(time);
}

MYLCD_EXPORT char* lVersion ()
{
	return libmylcdVERSION;
}


void *ptrstub (const void *stub, ...){return NULL;}
int intstub (const void *stub, ...){return -0x1FFFF;}
void voidstub (const void *stub, ...){return;}
