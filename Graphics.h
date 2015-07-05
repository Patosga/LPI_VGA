#ifndef __GRAPHICS__
#define __GRAPHICS__

#include "stm32f4xx.h"
#include "video.h"
#include "images.h"
#include "USART.h"

#define COLOR_BLACK 0x00
#define GDA_FALSE 0
#define GDA_TRUE 1
	

//extern __align(1) __IO unsigned char fb[VID_VSIZE][VID_HSIZE+2] ;	/* Frame buffer */
//extern __align(1) __IO unsigned char fb2[VID_VSIZE][VID_HSIZE+2] ;	/* Frame buffer */


typedef uint8_t GDAu8;
typedef uint16_t GDAu16;
typedef int16_t GDAs16;
typedef uint32_t GDAu32;
typedef int32_t GDAs32;
typedef uint8_t GDABool;

typedef struct primRectangle {
	GDAu16 x0;
	GDAu16 y0;
	GDAu16 x1;
	GDAu16 y1;
	GDABool Fill;
	GDAu8 Color;
	GDAu8 BorderColor;
	GDAu8 DeepLevel;
} primRectangle;

typedef struct primLine {
	GDAu16 x0;
	GDAu16 y0;
	GDAu16 x1;
	GDAu16 y1;
	GDAu8 Color;
	GDAu8 DeepLevel;
} primLine;

typedef struct primCircle {
	GDAu16 x0;
	GDAu16 y0;
	GDAu16 Radius;
	GDABool Fill;
	GDAu8 Color;
	GDAu8 BorderColor;
	GDAu8 DeepLevel;
} primCircle;

typedef struct primImage{
	int index;
	GDAu16 x0;
	GDAu16 y0;
	GDAu16 height;
	GDAu16 width;
} primImage;

struct ImageEntry {
	unsigned char type;
	unsigned char layer;
	short int x;
	short int y;
	short int height;
	short int width;
	unsigned char color;
	unsigned int index;
};




int DrawPoint(uint32_t x_point,uint32_t y_point,uint8_t  data);

void gdaSCircle(primCircle * CircleStruct);
void gdaSLine(primLine* LineStruct);
void gdaSRectangle(primRectangle* LineStruct);

void gdaLine(GDAu32 x1, GDAu32 y1, GDAu32 x2, GDAu32 y2, GDAu8 data);
void gdaRectangle(GDAu32 x0, GDAu32 y0, GDAu32 x1, GDAu32 y1, GDAu8 BorderColor, GDABool Fill, GDAu8 FillColor);
void	gdaCircle(GDAu32 x, GDAu32 y, GDAu32 r, GDAu8 BorderColor,GDABool Fill,GDAu8 FillColor); /*GDACIRCLE BORDERCOLOR NOT IMPLEMENTED*/
void 	gdaClearColor(GDAu16 R, GDAu16 G, GDAu16 B);

void gdaDrawImage(char image_index,int x,int y,int width, int height);
void processEntity(struct ImageEntry *Entity);


void process();
#endif
