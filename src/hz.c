
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


#include "mylcd.h"

#if (__BUILD_INTERNAL_HZGB2312__)

#include "gb2313.h"




int hzgb2312ToUTF32 (const ubyte *buffer, UTF32 *ch)
{
	// must remove these
	static int s0 = 0;
	static int s1 = 0;
	
    /*
     * When reading, our state variables are:
     * 
     *  - s0 is 0 in ASCII mode, 1 in GB2312 mode.
     * 
     * 	- s1 stores a character we have just seen but not fully
     * 	  processed. So in ASCII mode, this can only ever be zero
     * 	  (no character) or 0x7E (~); in GB2312 mode it can be
     * 	  anything from 0x21-0x7E.
     */

	if (s0 == 0) {
		/*
	 	* ASCII mode.
	 	*/
		if (s1) {
	    	s1 = 0;
	    	/* Process the character after a tilde. */
	    	switch (*buffer) {
	      	case '~':
		  		*ch = *buffer;
				return 1;
	      	case '\n':
	      		*ch = 0;
				return 1;		       /* ~\n is ignored */
			case '{':
				s0 = 1;	       /* switch to GB2312 mode */
				*ch = 0;
				return -1;
			}
			*ch = 0;
			return -1;

		}else if (*buffer == '~'){
	    	s1 = '~';
	    	*ch = 0;
	    	return -1;
		}else{
	    	/* In ASCII mode, any non-tildes go straight  */
	    	*ch = *buffer;
	    	return 1;
		}

    }else{
		/*
	 	* GB2312 mode. As I understand it, we expect never to see
	 	* anything in this mode that isn't 0x21-0x7E. So if we do,
	 	* we'll simply throw an error and return to ASCII mode.
	 	*/
		if (*buffer < 0x21 || *buffer > 0x7E) {
		    s0 = s1 = 0;
		    *ch = 0;
	    	return -1;
		}

		/*
	 	* So if we don't have a character stored already, store
	 	* this one...
	 	*/
		if (!s1) {
	    	s1 = *buffer;
	    	return -11;
		}

		/*
	 	* ... otherwise, combine the stored char with this one.
	 	* This will give either `~}', the escape sequence to
	 	* return to ASCII mode, or something which we translate
	 	* through GB2312.
	 	*/
		if ((s1 == '~') && (*buffer == '}')){
	    	s1 = s0 = 0;
	    	*ch = 0;
	    	return -1;
		}

		*ch = to_gb2321(s1 - 0x21, *buffer - 0x21);
		s1 = 0;
		return 1;
    }
}

#else

int hzgb2312ToUTF32 (const ubyte *buffer, UTF32 *ch){return 0;}

#endif

