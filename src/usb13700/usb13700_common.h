
// libmylcd
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


/*
//! PORT0
//! - bit 0 - expansion connector pin 6
//! - bit 1 - expansion connector pin 5
//! - bit 2 - expansion connector pin 13 (open drain)
//! - bit 3 - expansion connector pin 14 (open drain)
//! - bit 4 - expansion connector pin 16 (open drain)
//! - bit 5 - expansion connector pin 17 (open drain)
*/


// usb13700 expansion port0
#define PIN6		0x01
#define PIN5		0x02
#define PIN13		0x04
#define PIN14		0x08
#define PIN16		0x10
#define PIN17		0x20


#define BIT0		PIN6
#define BIT1		PIN5
#define BIT2		PIN13
#define BIT3		PIN14
#define BIT4		PIN16
#define BIT5		PIN17


#define USB13700_PORT0MASK	0x03		// port 0 write mask
#define USB13700_SPISENDBUFFERSIZE 32768

