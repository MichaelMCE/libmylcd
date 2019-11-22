
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


#ifndef _COMBMARKS_H_
#define _COMBMARKS_H_



// tests for diacritical marks and other combined characters
static INLINE int isCombinedMark (THWD *hw, UTF32 ch)
{
	if (!hw->flags.charCombine){
		if (ch < 768)
			return 0;
		else if (ch < 880)
			return 1;			// 768 to 879		latin
		else if (ch == 1161)
			return 1;
		else if (ch < 1456)
			return 0;
		else if (ch < 1475)
			return 1;			// 1456 to 1474		hebrew
		else if (ch < 1611)
			return 0;
		else if (ch < 1649)		
			return 1;			// 1611 to 1648		arabic
		else if (ch < 2305)
			return 0;
		else if (ch < 2308)
			return 1;			// 2305 to 2307		devanagari
		else if (ch < 2364)
			return 0;
		else if (ch < 2387)
			return 1;			// 2364 to 2386		devanagari
		else if ((ch == 3632)
			  || (ch == 3633))	// 3632 and 3633	thai
			return 1;
		else if (ch < 3636)
			return 0;
		else if (ch < 3643)
			return 1;			// 3636 to 3642		thai
		else if (ch < 3655) 
			return 0;
		else if (ch < 3663)
			return 1;			// 3655 to 3662		thai
		else if (ch < 7616)
			return 0;
		else if (ch < 7680)
			return 1;			// 7616 to 7679		marks supplement
		else if (ch < 8400)
			return 0;
		else if (ch < 8448)
			return 1;			// 8400 to 8447		marks for symbols
		else if (ch < 65056)
			return 0;
		else if (ch < 65072)
			return 1;			// 65056 to 65071	half marks
		else
			return 0;
	}
	return 0;
}


#endif 



