

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


#include <mylcd.h>


int main ()
{
    // Acquire a library handle
    THWD *hw = lOpen(NULL, NULL);
	if (hw == NULL)
		return EXIT_FAILURE;

    int width = 480;	// display width
    int height = 272;	// display height
    int data = 0;		// LPT port, data or device index selection
    int bpp = LFRM_BPP_32A;

    // Think of this as setting where a window is placed within a wall,
    // Adjusting window position will decide what can be seen.
    // This allows us to have frame (surface) larger than the display.
    // Through this a display may pan through a surface in real time at zero cost.
    TRECT rect;	
    rect.left = 0;
    rect.top = 0;
    rect.right = width-1;
    rect.btm = height-1;
    
    // Activate and add a display (device) to our tree.
    // rect assigns an update region within frame which must be maintained by user.
    // This allows one to adjust the position of the display relative to the frame, in realtime.
    lDISPLAY disID = lSelectDevice(hw, "DDRAW", "NULL", width, height, bpp, data, &rect);
    if (disID){
	    // create a frame with 1 bit per pixel surface with 320 columns by 240 rows.
    	// (frame dimension is not tied to display dimension).
    	TFRAME *surface = lNewFrame(hw, width, height, bpp);
    
    	lSetBackgroundColour(hw, lGetRGBMask(surface, LMASK_WHITE));	// paper
		lSetForegroundColour(hw, lGetRGBMask(surface, LMASK_BLACK));	// ink
		lClearFrame(surface);
	        
    	// print something on the top left corner of the frame
    	lPrint(surface, "hello world", 10, 0, LFT_DOTUMCHE24X24, LPRT_CPY);
    
    	// print libmylcd version using formatted print.
    	// print to column 1:row 30, ie; x,y = 1,30
    	lPrintf(surface, 1, 30, LFTW_WENQUANYI12PT, LPRT_CPY, "libmylcd v%s", lVersion());

    	// send frame to display.
    	lRefresh(surface);
    	// or
    	//lUpdate(hw, surface->frame, surface->frameSize);
    	// or
	    //lRefreshAsync(surface, 0);  // (0:copy frame, 1:swap frame buffers)

   
	    // Save result then sleep for a few seconds
	    //lSaveImage(surface, L"hello.bmp", IMG_BMP, 0, 0);
    	lSleep(2000);
    	
    	// delete & free frame resources
    	lDeleteFrame(surface);
    }
    // close library and free resources
    lClose(hw);

    return EXIT_SUCCESS;
}

