
	libmylcd
v1.0.0

libmylcd includes 4 base API's in one:
  Full unciode text rendering API.
  Display driver API.
  A base drawing primitive API.
  Surface layer API to bind everything together.



---


libmylcd is a modular multi-platform framebuffer API framework library designed to provide 
low level access to 2D hardware via a video framebuffer.
libmylcd contains an API's for drawing, plotting, printing text, scaling, rotating, copying,
moving, loading and saving of images and frames and scrolling of images/frames & text too.
With two proportional font engines, supporting ASCII and Unicode (wide character) font sets,
enabling the support of non-ASCII character sets such as traditional and simplifed Chinese.


There isn't anything in the way of documentation but viewing mylcd.h and the examples
should give you enough to get started.
In the mean time here follows a quick 'it'll-get-you-started' lesson.



hello.c:
```
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
    
    	lSetBackgroundColour(hw, lGetRGBMask(surface, LMASK_WHITE));	        // paper
		lSetForegroundColour(hw, lGetRGBMask(surface, LMASK_BLACK));    // ink
		lClearFrame(surface);
	        
    	// print something on the top left corner of the frame
    	lPrint(surface, "hello world", 0, 0, LFT_SMALLFONTS7X7, LPRT_CPY);
    
    	// print libmylcd version using formatted print.
    	// print to column 1:row 30, ie; x,y = 1,30
    	lPrintf(surface, 1, 30, LFTW_WENQUANYI12PT, LPRT_CPY, "libmylcd v%s", lVersion());

    	// send frame to display.
    	lRefresh(surface);
    	// or
    	//lUpdate(hw, surface->frame, surface->frameSize);
    	// or
		//lRefreshAsync(surface, 0);   /* 0:copy frame, 1:swap frame pointers */

   
		// Save result then sleep for a few seconds
		lSaveImage(surface, L"h.bmp", IMG_BMP, 0, 0);
    	lSleep(2000);
    	
    	// delete & free frame resources
    	lDeleteFrame(surface);
    }

    // close library and free resources
    lClose(hw);

    return EXIT_SUCCESS;
}
```

