

#ifndef _UTILS_H_
#define _UTILS_H_


#include <mylcd.h>


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))



int utilInit ();
void utilCleanup ();
int utilInitConfig (char *configfile);


// defined in utils.c

#define DISPLAYMAX	8
extern TRECT displays[DISPLAYMAX];

extern TFRAME *frame;
extern THWD *hw;
extern int DWIDTH;
extern int DHEIGHT;
extern int DBPP;
extern lDISPLAY did;


#endif

