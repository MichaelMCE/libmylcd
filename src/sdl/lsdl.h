/*
 * TinyPTC SDL v0.3.2 Main header file
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

#ifndef _LSDL_H_
#define _LSDL_H_


#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>


typedef struct{
	int sdl_pitch;
	int sdl_stored_width;
	int sdl_stored_height;
	SDL_Surface *sdl_video_surface;
	SDL_Surface *sdl_blit_surface;
	SDL_Surface *sdl_buffer_surface;
	SDL_Event sdl_sdl_event;
	unsigned int *buffer;
}TSDLWIN;




#define SDL_FAILURE 0
#define SDL_SUCCESS 1

/* This directive enables windowed output */
#define __SDL_WINDOWED__

/* This directive enables window centering (linux only) */
// #define __SDL_CENTER_WINDOW__

/* This directive enables pixelformat conversions */
// #define __SDL_ENABLE_CONVERSIONS__

/* This directive enables automatic processing of the alpha channel */
// #define __SDL_ENABLE_ALPHA__ 

/* This directive enables the use of an user-supplied callback that will be
 * triggered upon exit */
// #define __SDL_CLEANUP_CALLBACK__

/* Hide cursor over window */
//#define __SDL_HIDECURSOR__


typedef unsigned int int32;
typedef unsigned short int16;
typedef unsigned short short16;


          
TSDLWIN *sdl_open (char *title, int width, int height);
int sdl_update (TSDLWIN *psdlwin, void *buffer);
void sdl_close (TSDLWIN *psdlwin);
int sdl_process_events (TSDLWIN *psdlwin);


#ifdef __SDL_CLEANUP_CALLBACK__
void sdl_cleanup_callback (TSDLWIN *psdlwin);
#endif


#endif
