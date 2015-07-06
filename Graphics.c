
#include "Graphics.h"
#include "mathsin.h"
#include "math.h"


static GDAu8 BackGroundColor = COLOR_BLACK;

// x[0-200] e y[0-399]

void gdaLine(GDAs32 x1,GDAs32 y1,GDAs32 x2,GDAs32 y2,GDAu8 data) {

GDAs32 		dx, dy, i, e;
GDAs32		incx, incy, inc1, inc2;
GDAs32		x, y;

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
		DrawPoint(x, y, data);
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
			DrawPoint(x, y, data);
		}
	} else {
		DrawPoint(x, y, data);
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
			DrawPoint(x, y, data);
		}
	}
}



void	gdaRectangle(GDAs32 x0, GDAs32 y0, GDAs32 x1,GDAs32 y1, GDAu8 BorderColor, GDABool Fill, GDAu8 FillColor) 
{
	GDAs16 a = 0;
	gdaLine(x0,y0,x1,y0, BorderColor);
	gdaLine(x0,y1,x1,y1, BorderColor);
	gdaLine(x0,y0,x0,y1, BorderColor);
	gdaLine(x1,y0,x1,y1, BorderColor);
	
	if(Fill == GDA_TRUE){
		for(a = x0+1 ; a < x1; a++){
			if(a<0) continue;
			gdaLine(a,y0+1,a,y1-1,FillColor);
		}
	}
	
}


void	gdaCircle(GDAu32 x,GDAu32 y,GDAu32 r,GDAu8 BorderColor,GDABool Fill,GDAu8 FillColor ) {
float		x1, y1;
float		a;

	for (a = 0.0; a < 360.0; a+=0.5) {
		x1 =  r * cos((a * 3.1415F )/180.0F);//mthCos(a);
		y1 =  r * sin((a * 3.1415F )/180.0F); // mthSin(a);
		if(Fill){
			gdaLine(x,y,(int)(x1) + x, (int)(y1) + y,FillColor);
		}
		DrawPoint((int)(x1 ) + x, (int)(y1) + y, BorderColor);
	}
/*		
GDAs32	x1, y1;
GDAu16	a;

	for (a = 0; a < 360; a++) {		
		x1 = r * mthCos(a);
		y1 = r * mthSin(a);
		DrawLine(x,y,(int)(x1 / 10000) + x, (int)(y1 / 10000) + y,0x00);
		DrawPoint((int)(x1 / 10000) + x, (int)(y1 / 10000) + y, data);
	}*/
		
}


void gdaSCircle(primCircle * CircleStruct){
	gdaCircle(CircleStruct->x0,CircleStruct->y0,CircleStruct->Radius,CircleStruct->BorderColor,CircleStruct->Fill,CircleStruct->Color);
}



void gdaSLine(primLine* LineStruct){
	gdaLine(LineStruct->x0,LineStruct->y0,LineStruct->x1,LineStruct->y1,LineStruct->Color);
}



void gdaSRectangle(primRectangle* RectStruct){
	gdaRectangle(RectStruct->x0,RectStruct->y0,RectStruct->x1,RectStruct->y1,RectStruct->BorderColor,RectStruct->Fill,RectStruct->Color);
}



void gdaDrawImage(char image_index,int x,int y,int width, int height)
{
	unsigned char data;
	const unsigned char *image;

	int x_image, y_image;
	image = image_vector[image_index];
	for(x_image=0; x_image<height; x_image++)
	{
		for(y_image=0; y_image<width; y_image++)
		{
			data = image[(x_image*width)+y_image];
			data = ((data << 6) & 0xc0) | ((data >> 5) & 0x7) | ((data << 1) & 0x38);
			if(data==TRANSPARENCE) continue;
			DrawPoint((x+x_image),(y+y_image),data);
		}
	}
}


void processEntity(struct ImageEntry *Entity){
	
	struct primRectangle rect;
	struct primCircle circle;
	struct primImage image;
	
	switch(Entity->type){
		case 0 : break;   //null
		case 1 :   //square
				
			rect.x0 = Entity->x;
			rect.y0 = Entity->y;
			rect.x1 = Entity->x + Entity->width;
			rect.y1 = Entity->y + Entity->height;
			rect.Color = Entity->color;
			rect.DeepLevel = Entity->layer;
			rect.BorderColor = Entity->color;
			rect.Fill = 1;
			
			gdaRectangle(rect.x0, rect.y0,rect.x1, rect.y1, rect.BorderColor, rect.Fill, rect.Color);
			break;
		
		case 2 :   //circle

			circle.x0 = Entity->x + (Entity->width / 2);
			circle.y0 = Entity->y + (Entity->height / 2);
			circle.Radius = Entity->height / 2;
			circle.Fill = 1;
			circle.Color = Entity->color;
			circle.BorderColor = Entity->color;
			circle.DeepLevel = Entity->layer;
		
			gdaCircle(circle.x0, circle.y0, circle.Radius, circle.BorderColor, circle.Fill, circle.Color );
			break;
		
		case 3 :   //pixmap
			image.index = Entity->index;
			image.x0 = Entity->x;
			image.y0 = Entity->y;
			image.height = Entity->height;
			image.width = Entity->width;
	
			//gdaDrawImage(image.index, image.x0, image.y0, image.height, image.width);
		
			break;
		
		case 4 :    //text
			break;
	}
}

void process(){

	static unsigned char num_entities = 0;
	static unsigned char flag = 0, flag2 = 1;
	unsigned char i = 0;
	static unsigned int inc = 5;
	unsigned char begin = 0x55;
	static struct ImageEntry entities[50];
	static unsigned char counter = 0;
	


	vidClearHalfScreen();
		
	for (i = 0; i < num_entities; i++){ 
//		eraseSquare(entities[i].y, entities[i].x, entities[i].height, entities[i].width);
		processEntity(&entities[i]);
	}


	if(flag == 0){

		num_entities = 5;

		entities[0].type = 1;
		entities[0].height = 30;
		entities[0].width = 30;
		entities[0].y = 100;
		entities[0].x = 20;
		entities[0].color = 7;

		entities[1].type = 1;
		entities[1].height = 30;
		entities[1].width = 200;
		entities[1].y = 50;
		entities[1].x = 0;
		entities[1].color = 0;

		entities[2].type = 1;
		entities[2].height = 30;
		entities[2].width = 200;
		entities[2].y = 150;
		entities[2].x = 0;
		entities[2].color = 56;
		
		entities[3].type = 1;
		entities[3].height = 30;
		entities[3].width = 200;
		entities[3].y = 200;
		entities[3].x = 0;
		entities[3].color = 200;
		
		entities[4].type = 1;
		entities[4].height = 30;
		entities[4].width = 200;
		entities[4].y = 250;
		entities[4].x = 0;
		entities[4].color = 7 + (3 << 6);
		
		entities[5].type = 1;
		entities[5].height = 30;
		entities[5].width = 100;
		entities[5].y = 300;
		entities[5].x = 0;
		entities[5].color = 0;
		
		entities[6].type = 1;
		entities[6].height = 10;
		entities[6].width = 10;
		entities[6].y = 60;
		entities[6].x = 110;
		entities[6].color = 0;
		
		entities[7].type = 1;
		entities[7].height = 10;
		entities[7].width = 10;
		entities[7].y = 60;
		entities[7].x = 160;
		entities[7].color = 0;
		
		entities[8].type = 1;
		entities[8].height = 10;
		entities[8].width = 10;
		entities[8].y = 60;
		entities[8].x = 160;
		entities[8].color = 0;
		
		entities[9].type = 1;
		entities[9].height = 10;
		entities[9].width = 10;
		entities[9].y = 60;
		entities[9].x = 160;
		entities[9].color = 0;
		
		flag = 1;

	}
	else if(flag == 1) {
		
		entities[0].x += inc;
		if(entities[0].x <= 0)
			inc = 2;
		if(entities[0].x + 30 >= VID_VSIZE)
			inc = -2;

	}
		
}
