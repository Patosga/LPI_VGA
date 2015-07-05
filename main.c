#include "stm32f4xx_conf.h"                  // Device header
#include "USART.h"
#include "stm32f4xx.h"
#include "Graphics.h"

int main(){
	unsigned int i=0;
	unsigned char x;
	
	vidInit();
	USART_init();
	
	
	
	while (1){
		if(processready == 1){
			processready = 0;
			process();
		}
		//x = fgetc(0);
		//fputc(x,0);
	}

}

