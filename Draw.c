#include "Draw.h"
#include "mathsin.h"

extern unsigned char fb[VID_VSIZE][VID_HSIZE+2];

void DrawPoint(signed short int x, signed short int y, unsigned char color)
{

	//	Test for point outside display area
	if (x >= VID_PIXELS_X || y >= VID_PIXELS_Y) return;
	if (x < 0 || y < 0) return;

	fb[x][y] = color;
	
}


//*****************************************************************************
//	Function gdiCircle(i16 x, i16 y, i16 r, u16 rop)
//
//	Draw circle. This function uses the integer-precision math
//
//	parameters:
//		x			Circle center X position
//		y			Circle center Y position
//		r			Radius
//
//	return			none
//*****************************************************************************
void	DrawCircle(unsigned short int x, unsigned short int y,unsigned short int r, unsigned char color) {

signed int	x1, y1;
unsigned short int	a;

	for (a = 0; a < 360; a++) {		
		x1 = r * mthCos(a);
		y1 = r * mthSin(a);
		DrawPoint((x1 / 10000) + x,(y1 / 10000) + y,color);
	}
}


//*****************************************************************************
//	Function gdiLine(i16 x1, i16 y1, i16 x2, i16 y2, u16 rop)
//
//	Draw line using Bresenham algorithm 
//
//	This function was taken from the book:
//	Interactive Computer Graphics, A top-down approach with OpenGL
//	written by Emeritus Edward Angel
//
//	parameters:
//		prc			Clipping rectangle
//		x1			X start position
//		y1			Y start position
//		x2			X end position
//		y2			Y end position
//		rop			Raster operation. See GDI_ROP_xxx defines
//
//	return			none
//*****************************************************************************
void DrawLine(signed short int x1,	signed short int y1,	signed short int x2,	signed short int y2,	unsigned short int rop) {

signed short int		dx, dy, i, e;
signed short int		incx, incy, inc1, inc2;
signed short int		x, y;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;
	incx = 1;
	if(x2 < x1) incx = -1;
	incy = 1;
	if(y2 < y1) incy = -1;
	x=x1;
	y=y1;

	if (dx > dy) {
		DrawPoint(x, y ,rop);
		e = 2*dy - dx;
		inc1 = 2 * ( dy -dx);
		inc2 = 2 * dy;
		for (i = 0; i < dx; i++) {
			if (e >= 0) {
				y += incy;
				e += inc1;
			}
			else {
				e += inc2;
			}
			x += incx;
			DrawPoint(x, y, rop);
		}
	} else {
		DrawPoint(x, y, rop);
		e = 2 * dx - dy;
		inc1 = 2 * (dx - dy);
		inc2 = 2 * dx;
		for(i = 0; i < dy; i++) {
			if (e >= 0) {
				x += incx;
				e += inc1;
			} else {
				e += inc2;
			}
			y += incy;
			DrawPoint(x, y, rop);
		}
	}
}



//*****************************************************************************
//	Function gdiRectangle(i16 x1, i16 y1, i16 x2, i16 y2, u16 rop)
//
//	Draw rectangle
//
//	parameters:
//		x1			X start position
//		y1			Y start position
//		x2			X end position
//		y2			Y end position
//		rop			Raster operation. See GDI_ROP_xxx defines
//
//	return			none
//*****************************************************************************
void	DrawRectangle(signed short int x0,signed short int y0,signed short int x1,signed short int y1, unsigned short int rop) {

	DrawLine(x0,y0,x1,y0,rop);
	DrawLine(x0,y1,x1,y1,rop);
	DrawLine(x0,y0,x0,y1,rop);
	DrawLine(x1,y0,x1,y1,rop);
}
