#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "stm32f4xx.h"

#define VID_HSIZE 400 //400 ou 200 
#define VID_VSIZE 200 //200

#define	VID_PIXELS_X		(VID_HSIZE * 8) 		//Pixels
#define	VID_PIXELS_Y		VID_VSIZE						//Lines
#define	VID_PIXELS_XR		(VID_PIXELS_X + 16) //
#define	VID_HSIZE_R			(VID_HSIZE + 2)

extern __align(1) __IO unsigned char fb[VID_VSIZE][VID_HSIZE+2];	/* Frame buffer */
//extern __align(1) __IO unsigned char fb2[(VID_VSIZE/2)][VID_HSIZE+2] ;	/* Frame buffer */


extern volatile unsigned char SwitchFlag;
extern volatile unsigned char processready;
extern volatile unsigned char istopdrawing;
extern volatile unsigned char isbotdrawing;

int DrawPoint(signed int x_point,signed int y_point,uint8_t  data);
void eraseSquare(int x, int y, int height, int width);
void vidClearHalfScreen(void);
void vidInit(void);
void TIM8_Init(void);
void TIM5_Init(void);


#endif
