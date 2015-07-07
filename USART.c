#include "USART.h"
#include <stm32f4xx.h>

#define DISABLE_DMA_INTERRUPTS NVIC_DisableIRQ(DMA1_Stream6_IRQn);NVIC_DisableIRQ(DMA1_Stream5_IRQn);
#define ENABLE_DMA_INTERRUPTS NVIC_EnableIRQ(DMA1_Stream6_IRQn);NVIC_EnableIRQ(DMA1_Stream5_IRQn);

char input_buffer[INPUT_BUFFER_LENGTH]  __attribute__((at(0x2001C000))) = {0};
volatile unsigned char TableReady = 0;



//RX
__irq void DMA1_Stream5_IRQHandler(void){
		static unsigned char first = 1;
	
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
	
		if(first){
				DMA1_Stream5->CR &= ~0x01; 	
				while ((DMA_GetCmdStatus(DMA1_Stream5)!= DISABLE)){};
				DMA1_Stream5->NDTR  = input_buffer[0];
				DMA1_Stream5->M0AR += 1;
				DMA1_Stream5->CR |= 0x01;
				while ((DMA_GetCmdStatus(DMA1_Stream5)!= ENABLE)){};	
				first = 0;
				USART_SendData(USART2, 0x56);
		}
		else{
				TableReady = 1;
				first = 1;
			  DMA1_Stream5->CR &= ~0x01; 
			  while ((DMA_GetCmdStatus(DMA1_Stream5)!= DISABLE)){};
				DMA1_Stream5->M0AR -= 1;
				DMA1_Stream5->NDTR  = 1;
		}
}
		
void USART_readData(void *addr, int datasize, int n){
	unsigned char *ptr = addr;
	int num_bytes = datasize * n, i;
	char * ptrbuff = input_buffer;
	
	ptrbuff++;
	for(i = 0; i < num_bytes; i++){
		*ptr = *(ptrbuff++);
		ptr++;
	}
}

void USART_init(void){
	
	GPIO_InitTypeDef  gpio_init_struct;
	USART_InitTypeDef usart_init_struct;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	__IO uint32_t    Timeout = 50000;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	gpio_init_struct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_PuPd =  GPIO_PuPd_UP;
	gpio_init_struct.GPIO_OType = GPIO_OType_PP;
	
	GPIO_Init(GPIOA, &gpio_init_struct);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	/* Reset DMA Stream registers (for debug purpose) */
  DMA_DeInit(DMA1_Stream5);

  /* Configure DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR); //;0x40000010 &(GPIOB->ODR)//dest
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)input_buffer; //source TAVA fb
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure); //USART2_RX
	
	
  /* Enable DMA Stream Transfer Complete interrupt */
  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA1_Stream5,ENABLE);
	  /* Enable DAC Channel2 */
  Timeout = 50000;
  while ((DMA_GetCmdStatus(DMA2_Stream1) != ENABLE) && (Timeout-- > 0))
  {
  }
  
  /* Check if a timeout condition occurred */
  if (Timeout == 0)
  {
    /* Manage the error: to simplify the code enter an infinite loop */
    while (1)
    {
    }
  }

	//DMA2->LIFCR = 0xC40;
	
  /* Enable the DMA Stream IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	//USART structinitialization
	usart_init_struct.USART_BaudRate= BAUDRATE; // baudrate
	usart_init_struct.USART_WordLength = USART_WordLength_8b; // frame size 8 bits (standard)
	usart_init_struct.USART_StopBits= USART_StopBits_1; // 1 stop bit (standard)
	usart_init_struct.USART_Parity= USART_Parity_No; // no parity bit (standard)
	usart_init_struct.USART_HardwareFlowControl= USART_HardwareFlowControl_None; // no flow control (standard)
	usart_init_struct.USART_Mode= USART_Mode_Tx| USART_Mode_Rx; // enable transmitter and receiver//
	
	USART_Init(USART2, &usart_init_struct);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);
	USART_Cmd(USART2, ENABLE);
	

}