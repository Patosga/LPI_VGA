
/*----------------------------------------------------------------------------
  Notes:
  The length of the receive and transmit buffers must be a power of 2.
*----------------------------------------------------------------------------*/

#define TBUF_SIZE   512	     /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define RBUF_SIZE   512      /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/

void USART2_Init(void);
int GetKey (void);
int SendChar (int);

void USART_sendData(void *addr, int datasize, int n);
void USART_readData(void *addr, int datasize, int n);

