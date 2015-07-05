/*
 * Terminal.h
 *
 * Created: 02-03-2015 15:05:20
 *  Author: Zé
 */ 

#ifndef TERMINAL_H_
#define TERMINAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define INPUT_BUFFER_LENGTH 16
#define OUTPUT_BUFFER_LENGTH 256
#define BAUDRATE 500000
	

extern char input_buffer[INPUT_BUFFER_LENGTH];
extern char aux_buffer[INPUT_BUFFER_LENGTH];
extern volatile unsigned int istart, iend;
extern char output_buffer[OUTPUT_BUFFER_LENGTH];
extern volatile unsigned int ostart, oend;

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
