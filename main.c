#include "stm32f4xx_conf.h"                  // Device header
#include "USART.h"
#include "stm32f4xx.h"
#include "Graphics.h"



int main(){
	unsigned int i=0;
	unsigned char x;
	
	
	vidInit();
	USART_init();
	image_vector_init();
	
	USART_SendData(USART2, 0x55);
	
	while (1){
		if(processready == 1){
			processready = 0;
			process();
		}
		
	
		
		//x = fgetc(0);
		//fputc(x,0);
	}

}

