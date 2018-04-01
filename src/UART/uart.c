#include "uart.h"
#include "../Timers/timers.h"
#include <sam3xa/include/sam3x8e.h>
#include  <stdio.h>

/*enable uart chip & channel 0 (for debug communcation).*/
void initUART(void){
  //set PIO Lines to UART


  REG_PMC_PCER0 |= PMC_PCER0_PID8; // enable UART peripheral clock
  REG_PMC_PCER0 |= PMC_PCER0_PID11; // enable PIO controller A

  //disable PIO lines (so UART can use them)
  REG_PIOA_PDR |= PIO_PA8 | PIO_PA9;
  //disable PIO interrupts on serial lines
  REG_PIOA_IDR |= PIO_PA8 | PIO_PA9;
  //enable pull up on UART pins
  REG_PIOA_PUER |= PIO_PA8 | PIO_PA9;
  //set to UART mode
  REG_PIOA_ABSR &= ~PIO_PA8 & ~PIO_PA9;

  //disable DMA
  REG_UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

  //restet UART
  REG_UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS | UART_CR_RSTSTA;

  //set baud rate to about 9600
  REG_UART_BRGR = UART_BRGR_CD(546);

  //disable parity
  REG_UART_MR = 0x0;
  REG_UART_MR = UART_MR_PAR_NO | UART_MR_CHMODE_NORMAL;

  //disable all interrupts
  REG_UART_IDR = 0xFFFFFFFF;
  //REG_UART_IER = UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME;

  //uart control reg  (enable receive and transmit)
  REG_UART_CR |= UART_CR_RXEN | UART_CR_TXEN;
}

/*
  override C std lib _write and _read methods
*/
int _write(int file, char *ptr, int len){
  for(uint32_t i=0; i < len;){
    if(UART->UART_SR & UART_SR_TXRDY){
      //transmit one character
      REG_UART_THR = *(ptr + i);
      i++;
    }
  }
  return len;
}

int _read (int fd, char *ptr, int len){
  uint32_t startTime = getTime();
  int i=0;
  for(i; i < len;){
    if(UART->UART_SR & UART_SR_RXRDY){
      //read one character
      *(ptr + i) = (char)UART->UART_RHR;
      i++;
    }
    if((startTime - getTime()) < -5 && i > 0){
      //wait 5ms MAX
      break;
    }
  }
  return i;
}
