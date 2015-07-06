#include "stm32f4xx_conf.h"
#include "video.h"
#include "stm32f4xx.h"
#include "Graphics.h"

#define VTOTAL 52 /* Total bytes to send through GPIOB */
__align(1) __IO unsigned char fb[VID_VSIZE][VID_HSIZE+2] = {0};	/* Frame buffer */

static unsigned char counter1 =0;
static unsigned char counter2 =0;

static volatile u16 vline = 0;				/* The current line being drawn */
static volatile u32 vflag = 0;				/* When 1, the SPI DMA request can draw on the screen */
static volatile u32 vdraw = 0;				/* Used to increment vline every 3 drawn lines */ 
volatile unsigned char processready = 0;
volatile unsigned char istopdrawing = 1;
volatile unsigned char isbotdrawing = 0;
volatile unsigned char * CurrentLine = (volatile unsigned char *)fb;

volatile unsigned char SwitchFlag = 0;

/*Function to configure all GPIO needed for the VGA signals
	GPIOB	->	DATA
	PE11	->	Hsync
	PC7		->	Vsync
	(NOT USED YET)
*/


void GPIO_Configuration(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*Configure GPIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	
	/* Configure Pins for HSYNC And VSync Control Signals*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 ,GPIO_AF_TIM1);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7,GPIO_AF_TIM3);
	//---------------------------------
	
	/*Configure GPIOB as Digital Output for 8-bit VGA data*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_All;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//--------------------------------
}


//TIM1_CH2 -> PE11 HSYNC TIM3_CH2 -> PC7
/* Removed GPIO I for Timer 8*/
/* Need to change Tim8 for Tim2 since TIM8 is needed for dma2 request*/
void TIM_Configuration(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	int TimerPeriod, Channel1Pulse, Channel2Pulse, Channel3Pulse;
	
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 ,ENABLE);
	
	/* Pins GPIO A 1 and 8 for Control Signals*/ 
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 ,GPIO_AF_TIM1);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7 ,GPIO_AF_TIM3);
	
	/*
	VGA
	800x600 50hz 
	
	Clock Tick = 1/72Mhz = 0.0138us
	
	Timer Frequency = 35.15625kHz
	TimerPeriod = (1/35.15625kHz)/0.006944us = 4096 (/2)
	
	Channel 1 pulse / HSYNC Pulse Time= 2us
	2us/0.006944us = 288 ticks (/2)
	
	Channel 2 pulse / HSYNC+BackPorch
	2us+3,55us-800ns(dma)/0,00694us = 704 ticks 
	*/
	
	TimerPeriod = 4096;
	Channel1Pulse = 288;
	Channel2Pulse = 800;//785
	
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	TIM_OCStructInit(&TIM_OCInitStructure);
	
	//Channel 1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	
	//Channel 2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Inactive; // Dont actualize pin
	TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	
	/* Select TIM1 as Master */
	TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);
	TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
	
	
	/*
		VSYNC
	
						Lines
		------------------------------
		Visible area	600
		Front porch		1
		Sync pulse		2
		Back porch		22
		Whole frame		625
	
	
	*/
	
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Gated);//! was gated
	TIM_SelectInputTrigger(TIM3, TIM_TS_ITR0);
	
	TimerPeriod = 625;		/* Vertical lines */
	Channel2Pulse = 2;		/* Sync pulse */
	Channel3Pulse = 24;		/* Sync pulse + Back porch */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Inactive;
	TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);

	/*	TIM8 counter enable and output enable */
	TIM_CtrlPWMOutputs(TIM3, ENABLE);

	/* Interrupt TIM8 */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
	
	
	/* Interrupt TIM1 */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM1, TIM_IT_CC3, ENABLE); // Enable only interrupt for CC2
	
	/* Enable Timers */
	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
	
	
	
	//TIMER 5  --- UPDATE

}


void DMA_Config(void){

  NVIC_InitTypeDef NVIC_InitStructure;
  DMA_InitTypeDef  DMA_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
  __IO uint32_t    Timeout = 50000;
  //int i;
	
  /* Enable DMA clock & TIM3 Clock*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/*Enable GPIOB Pins as digital output*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM8_Init();
	
  /* Reset DMA Stream registers (for debug purpose) */
  DMA_DeInit(DMA2_Stream2);

  /* Configure DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(GPIOB->ODR); //;0x40000010 &(GPIOB->ODR)//dest
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)fb; //source TAVA fb
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)VID_HSIZE+2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC16;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream1, &DMA_InitStructure);
    
  /* Enable DMA Stream Transfer Complete interrupt */
  DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA2_Stream1,ENABLE);
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

	DMA2->LIFCR = 0xC40;
	
  /* Enable the DMA Stream IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	//TIM_Cmd(TIM8,ENABLE);
}

void TIM8_Init(void){
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8 ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6 ,GPIO_AF_TIM8);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource8 ,GPIO_AF_TIM8);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9 ,GPIO_AF_TIM8);
	/* 
	Time base configuration 
	Frequency of 18Mhz
	*/
  TIM_TimeBaseStructure.TIM_Period = 8- 1; // 8 -1 ou 16- 1
  TIM_TimeBaseStructure.TIM_Prescaler = 0; //Not sure if 144Mhz timer
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
	
	//Channel 1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 50;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);
	TIM_OC2Init(TIM8, &TIM_OCInitStructure);
	TIM_OC3Init(TIM8, &TIM_OCInitStructure);
	TIM_OC4Init(TIM8, &TIM_OCInitStructure);
	
  TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE );	/* Enable TIM8_Updates DMA Requests     */
	//SET COUNTER
	
}

void TIM5_Init(void){
	//TIMER 5
	NVIC_InitTypeDef NVIC_InitStructure5;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure5;
	
	NVIC_InitStructure5.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure5.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure5.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure5.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure5);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	
	TIM_TimeBaseStructure5.TIM_Period = (32000000/33) - 1 ;
  TIM_TimeBaseStructure5.TIM_Prescaler = (0);
	TIM_TimeBaseStructure5.TIM_ClockDivision = (0);
	TIM_TimeBaseStructure5.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure5);

	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM5, ENABLE);
	//TIMER5
}


//*****************************************************************************
//	This irq is generated at the end of the horizontal back porch.
//	Test if inside a valid vertical start frame (vflag variable), 
//	and start the DMA to output a single frame buffer line through the SPI device.
//*****************************************************************************
__irq void TIM1_CC_IRQHandler(void)
{
	if (vflag)
	{
		//DMA1_Stream4->CR &= ~0x01; //Enable DMA
		TIM8->CR1 = 0x1; //Enable DMA
	}
	
	//TIM1->SR &= ~0x4; //~TIM_IT_CC2; //limpa interrupçao 
	TIM1->SR = 0x0;
}


//*****************************************************************************
//	This irq is generated at the end of the vertical back porch.
//	Sets the 'vflag' variable to 1 (valid vertical frame).
//*****************************************************************************
void TIM3_IRQHandler(void)
{
	vflag = 1;				// New visible area available
	
	//TIM1->SR |= 0x08; // Turn on interrupt timer 1 ccr 3 (Start line)
	
	TIM3->SR &= ~0x8; // ~TIM_IT_CC3;
}


//*****************************************************************************
//	This interrupt is generated at the end of every line.
//	It will increment the line number and set the corresponding line pointer
//	in the DMA register.
//*****************************************************************************
void DMA2_Stream1_IRQHandler(void)
{	
	TIM8->CR1 &= ~0x1; 								//Disable Timer Stop trigger
	DMA2_Stream1->CR &= ~0x01; 				//Disable DMA NEEDED
  DMA2->LIFCR = 0xC40; 							//Clear IT HT Flag from stream 1 DMA 2
	DMA2_Stream1->NDTR  = VID_HSIZE+2; //Reload Bytes to send !!!!! not sure about -1 !!!!
	TIM8->CNT = 8-1;									//Put TIM8 counter on edge to interrupt
	
	//DMA2_Stream1->M0AR = (u32) &DMA_TEST[0][0]; //DMA DEBUG
	//DMA2_Stream1->M0AR = (uint32_t)CurrentLine;
	vdraw++;
	if (vdraw == 3){
		vdraw = 0;

		vline++;
		
		if (vline == VID_VSIZE - 1)
		{
			
			if(counter1 == 2){
				counter1 = 0;
				processready = 1;
			} else	counter1++;
						
			vdraw = vline = vflag = 0;
			DMA2_Stream1->M0AR = (u32) &fb[0][0]; //Put memory base address
			istopdrawing = 1;
			isbotdrawing = 0;
			processready = 1;
		
		} else if( vline == VID_VSIZE/2 - 1) {		
		
			if(counter2 == 2){
				counter2 = 0;
				processready = 1;
			} else	counter2++;

			
			DMA2_Stream1->M0AR = (u32) &fb[VID_VSIZE/2][0]; //Put memory base address
			istopdrawing = 0;
			isbotdrawing = 1;
			processready = 1;	
		} else {
			DMA2_Stream1->M0AR += VID_HSIZE+2;
		}
		

	}
	DMA2_Stream1->CR |= 0x01; 			//Enable DMA, get him ready for TIM8 START
}


//TIM5 interrupt

void TIM5_IRQHandler(void)
{
	unsigned char num_entities;
	struct ImageEntry entity;
	unsigned char i;
	unsigned char begin = 0x55;

	
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		
		if(TableReady){
			
			TableReady = 0;
			num_entities = getchar();
			
			for(i = 0; i < num_entities; i++){   
				USART_readData(&entity, sizeof(struct ImageEntry), 1);
				processEntity(&entity);
			}
			
			SwitchFlag = !SwitchFlag;
			
			putchar(begin);
			
		}
	}
}


void vidClearScreen(void)
{
	u16 x, y;
	int i, k;

	for (y = 0; y < VID_VSIZE; y++)
	{
		//if(!SwitchFlag)
			fb[y][0] = 0x00;
		//else 
		//	fb2[y][0] = 0x00;
		for (x = 1; x < VID_HSIZE+1; x++)
		{
		//	if(!SwitchFlag)
				fb[y][x] = 0xFF;	
		//	else
		//		fb2[y][x] = 0xFF;
		}
	//	if(!SwitchFlag)
			fb[y][VID_HSIZE+1] = 0x00;
	//	else	
	//		fb2[y][VID_HSIZE+1] = 0x00;
	}
	
	//gdaLine(-20,200,20,200,0x00);
	//gdaRectangle(-20,20,40,40,0x00,1,0x00);
}

void vidClearHalfScreen(void){
	
	u16 x, y;
	
	if(isbotdrawing){
		
	for (y = 0; y < (VID_VSIZE/2); y++)
	{
		for (x = 1; x < VID_HSIZE+1; x++)
		{
			fb[y][x] = 0xFF;	
		}
	}
	}
	
	if(istopdrawing){
		
	for (y = (VID_VSIZE/2); y < VID_VSIZE; y++)
	{
		for (x = 1; x < VID_HSIZE+1; x++)
		{
			fb[y][x] = 0xFF;	
		}
	}
		
	}

}

void eraseSquare(int x, int y, int height, int width){
	
	int i, k;
	
	
	if(x > VID_VSIZE || y > VID_HSIZE || x + height < 0 || y + width < 0)
		return;
	
	if(x < 0){
		height += x;
		x = 0;		
	} 

	if( x + height > VID_VSIZE){
		height -= x + height - VID_VSIZE ;
	}
	
	if(y < i){
		width += y;
		y = 0;
	}
	
	if( y + width > VID_HSIZE){
		width -= y + width - VID_HSIZE;
	}
	
	if(isbotdrawing){
		
		if(x  >= VID_VSIZE/2) return;
		
		for(i = 0; (i < height) && (x + i < VID_VSIZE / 2); i++){
			for(k = 1; k < width + 1; k++){
				fb[x+i][y+k] = 0xff;
			}	
		}
	}
	
	if(istopdrawing){
		
		if(x + height  < VID_VSIZE/2) return;
		
		for(i = x < VID_VSIZE/2 ? VID_VSIZE/2 : 0; i < height ; i++){
			for(k = 1; k < width + 1 ; k++){
				fb[x+i][y+k] = 0xff;
			}	
		}
	}
	
	
}

void vidInit(void)
{
	vidClearScreen();
	TIM8_Init();
	TIM_Configuration();
	DMA_Config();
	//TIM5_Init();
}

int DrawPoint(signed int x_point,signed int y_point,uint8_t data)
{
	
	if(x_point > VID_VSIZE - 1  || y_point > VID_HSIZE-1 || x_point < 0 || y_point < 0) return 0; //erro
	
	if((x_point >= (VID_VSIZE/2)  && istopdrawing) || (x_point <= (VID_VSIZE/2) - 1 && isbotdrawing))
	{
			fb[x_point][y_point+1] = data;

	}
	return 1;

}

