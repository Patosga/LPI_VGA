#ifndef __DRAW__
#define __DRAW__

#include "video.h"

/*
Pixel Map defined as

	(0,0)--------------------(0,VID_PIXELS_X)
		|-----------------------------|
		|-----------------------------|
		|-----------------------------|
(VID_PIXELS_Y,0)---------(VID_PIXELS_Y,VID_PIXELS_X)

VID_PIXELS_Y -> Lines
VID_PIXELS_X -> Pixels(Columns)

*/

void DrawPoint(signed short int x,signed short int y, unsigned char color);





#endif