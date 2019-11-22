

#ifndef _UTILS_H_
#define _UTILS_H_


#include <mylcd.h>


int utilInit ();
void utilCleanup ();
int utilInitConfig (char *configfile);



// defined in utils.c
extern TFRAME *frame;
extern THWD *hw;
extern int DWIDTH;
extern int DHEIGHT;
extern int DBPP;
extern lDISPLAY did;

#endif

