/*
 * TinyPTC SDL v0.3.2 Main SDL interface file
 * Copyright (C) 2000-2003 Alessandro Gatti <a.gatti@tiscali.it>
 *
 * http://sourceforge.net/projects/tinyptc/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 */


// compiled and tested against SDL 'SDL-1.2.9-2ea'

#include "mylcd.h"

#if (__BUILD_SDL__)


#include <string.h>
#include <signal.h>

#include "mylcd.h"
#include "../memory.h"
#include "../convert.h"
#include "lsdl.h"



#ifdef __SDL_ENABLE_ALPHA__
  #define SDL_ALPHA_MASK 0xFF000000
#else
  #define SDL_ALPHA_MASK 0x00000000
#endif
#define SDL_RED_MASK   0x00FF0000
#define SDL_GREEN_MASK 0x0000FF00
#define SDL_BLUE_MASK  0x000000FF
#ifndef __SDL_WINDOWED__
  #undef __SDL_CENTER_WINDOW__
#endif
 


TSDLWIN *sdl_open (char *title, int width, int height)
{
	if (SDL_Init(SDL_INIT_VIDEO/*|SDL_INIT_EVERYTHING*/) < 0)
		return NULL;
	
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
  
	TSDLWIN *psdlwin = (TSDLWIN *)l_calloc(1, sizeof(TSDLWIN));
	if (psdlwin == NULL)
		return NULL;
	
	psdlwin->buffer = (unsigned int *)l_malloc(width * height * sizeof(unsigned int));
	if (psdlwin->buffer == NULL){
		l_free(psdlwin);
		return NULL;
	}

#ifdef __SDL_WINDOWED__
	SDL_WM_SetCaption(title, NULL);
#endif

	psdlwin->sdl_video_surface = SDL_SetVideoMode(width, height,
#ifdef __SDL_ENABLE_CONVERSIONS__				  
	32, 
#else
	0,
#endif /* __SDL_ENABLE_CONVERSIONS__ */
	SDL_HWSURFACE | SDL_DOUBLEBUF
#ifndef __SDL_WINDOWED__
	| SDL_FULLSCREEN
#endif /* !__SDL_WINDOWED__ */
	);
	
	if (psdlwin->sdl_video_surface == NULL)
		return NULL;

#ifdef __SDL_CENTER_WINDOW__
	SDL_SysWMinfo sdl_wm_info;
	
    SDL_VERSION(&sdl_wm_info.version);
    if (SDL_GetWMInfo(&sdl_wm_info) > 0) {
      if (sdl_wm_info.subsystem == SDL_SYSWM_X11) {
        sdl_wm_info.info.x11.lock_func();
        int sdl_wm_width = DisplayWidth(sdl_wm_info.info.x11.display, DefaultScreen(sdl_wm_info.info.x11.display));
        int sdl_wm_height = DisplayHeight(sdl_wm_info.info.x11.display, DefaultScreen(sdl_wm_info.info.x11.display));
       	int sdl_wm_x = (sdl_wm_width - psdlwin->sdl_video_surface->w) >> 1;
        int sdl_wm_y = (sdl_wm_height - psdlwin->sdl_video_surface->h) >> 1;
        XMoveWindow(sdl_wm_info.info.x11.display, sdl_wm_info.info.x11.wmwindow, sdl_wm_x, sdl_wm_y);
        sdl_wm_info.info.x11.unlock_func();
      }
    }
#endif /* __SDL_CENTER_WINDOW__ */

#ifdef __SDL_HIDECURSOR__ 
	SDL_ShowCursor(SDL_FALSE);
#endif

	psdlwin->sdl_stored_width = width;
	psdlwin->sdl_stored_height = height;
	
#ifndef __SDL_WINDOWED__
	psdlwin->sdl_pitch = psdlwin->sdl_video_surface->pitch;
#endif /* !__SDL_WINDOWED__ */


	psdlwin->sdl_buffer_surface = SDL_CreateRGBSurface(SDL_HWSURFACE, psdlwin->sdl_stored_width, psdlwin->sdl_stored_height, 
	  32, SDL_RED_MASK, SDL_GREEN_MASK, SDL_BLUE_MASK, SDL_ALPHA_MASK);
	//psdlwin->sdl_buffer_surface = SDL_CreateRGBSurfaceFrom(psdlwin->buffer, psdlwin->sdl_stored_width, psdlwin->sdl_stored_height, 
	  //32, psdlwin->sdl_stored_width*sizeof(int), SDL_RED_MASK, SDL_GREEN_MASK, SDL_BLUE_MASK, SDL_ALPHA_MASK);
	  
#ifndef __SDL_ENABLE_CONVERSIONS__
	psdlwin->sdl_blit_surface = psdlwin->sdl_buffer_surface;
#endif /* !__SDL_ENABLE_CONVERSIONS__ */

	return psdlwin;
}

int sdl_update (TSDLWIN *psdlwin, void *buffer)
{
	int sdl_return_code;
	
//	if (buffer != psdlwin->sdl_buffer_surface->pixels)
	//	l_memcpy(psdlwin->sdl_buffer_surface->pixels, buffer, psdlwin->sdl_stored_width * psdlwin->sdl_stored_height * sizeof(int));
	
#ifdef __SDL_ENABLE_CONVERSIONS__
	psdlwin->sdl_blit_surface = SDL_DisplayFormat(psdlwin->sdl_buffer_surface);
	if (psdlwin->sdl_blit_surface == NULL) {
		return SDL_FAILURE;
	}
#else
	psdlwin->sdl_blit_surface = psdlwin->sdl_buffer_surface;
#endif /* __SDL_ENABLE_CONVERSIONS__ */

	//void *real = psdlwin->sdl_buffer_surface->pixels;
	psdlwin->sdl_buffer_surface->pixels = buffer;
	sdl_return_code = SDL_BlitSurface(psdlwin->sdl_blit_surface, NULL, psdlwin->sdl_video_surface, NULL);
	//psdlwin->sdl_buffer_surface->pixels = real;
	
	if (sdl_return_code != 0) {
		return SDL_FAILURE;
	}
#ifdef __SDL_ENABLE_CONVERSIONS__
	SDL_FreeSurface(psdlwin->sdl_blit_surface);
#endif /* __SDL_ENABLE_CONVERSIONS__ */

	sdl_return_code = SDL_Flip(psdlwin->sdl_video_surface);
	if (sdl_return_code != 0) {
		return SDL_FAILURE;
	}

    if (sdl_process_events(psdlwin) == SDL_FAILURE) {
#ifdef __SDL_CLEANUP_CALLBACK__
      sdl_cleanup_callback(psdlwin);
#endif /* __SDL_CLEANUP_CALLBACK__ */
      sdl_close(psdlwin);
      //exit(0);
      raise(SIGINT);
    }

	return SDL_SUCCESS;
}

int sdl_process_events (TSDLWIN *psdlwin)
{

	if (SDL_PollEvent (&psdlwin->sdl_sdl_event)) {
		unsigned char *sdl_keypressed;

		switch (psdlwin->sdl_sdl_event.type) {
			case SDL_KEYDOWN: {
#if 0
				sdl_keypressed = SDL_GetKeyboardState(NULL);
#else
				sdl_keypressed = SDL_GetKeyState(NULL);
#endif
				if (sdl_keypressed[SDLK_ESCAPE] == SDL_PRESSED){
					sdl_close(psdlwin);
					SDL_Quit();
					raise(SIGINT);
					return SDL_FAILURE;
                }
			}; break;
			case SDL_QUIT: {
                sdl_close(psdlwin);
				SDL_Quit();
              	raise(SIGINT);
				return SDL_FAILURE;
			}; break;
		}
	}
	return SDL_SUCCESS;
}

void sdl_close (TSDLWIN *psdlwin)
{
	if (psdlwin){
		SDL_ShowCursor(SDL_TRUE);

		if (psdlwin->sdl_buffer_surface)
			SDL_FreeSurface(psdlwin->sdl_buffer_surface);
		if (psdlwin->sdl_video_surface)
			SDL_FreeSurface(psdlwin->sdl_video_surface);
		if (psdlwin->buffer)
			l_free(psdlwin->buffer);
			
		psdlwin->buffer = NULL;
		psdlwin->sdl_video_surface = NULL;
		psdlwin->sdl_buffer_surface = NULL;
		l_free(psdlwin);
	}
}

#endif
