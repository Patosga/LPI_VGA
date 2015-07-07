/*
 * Terminal.h
 *
 * Created: 02-03-2015 15:05:20
 *  Author: Z�
 */ 

#ifndef TERMINAL_H_
#define TERMINAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define INPUT_BUFFER_LENGTH 2048
#define OUTPUT_BUFFER_LENGTH 2048
#define BAUDRATE 2000000
	

extern char input_buffer[INPUT_BUFFER_LENGTH];
extern char aux_buffer[INPUT_BUFFER_LENGTH];
extern volatile unsigned int istart, iend;

extern volatile unsigned char sendactive;
extern volatile unsigned char sendfull;
	
	
extern volatile unsigned char TableReady;

void USART_init(void);
void USART_sendData(void *addrs, int datasize ,int n);
void USART_readData(void *addr, int datasize, int n);

#ifdef __cplusplus
}
#endif
#endif 
