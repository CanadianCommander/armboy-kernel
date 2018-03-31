#include <stdlib.h>
#include <stdio.h>

#include <sam3xa/include/sam3x8e.h>
#include "UART/uart.h"


// Addresses of several registers used to control the real-time timer.
static volatile int * const timer_mode_register  = (int *)0x400E1A30;
static volatile int * const timer_value_register = (int *)0x400E1A38;


// As the name suggests, this function sleeps for a given number of
// milliseconds. Our replacement for Arduino's delay function.
void sleep_ms(int milliseconds) {
	int sleep_until = *timer_value_register + milliseconds;
	while (*timer_value_register < sleep_until) {}
}

void sysClockInit(void){
  EFC0->EEFC_FMR = EEFC_FMR_FWS(4);
  EFC1->EEFC_FMR = EEFC_FMR_FWS(4);

  PMC->CKGR_MOR = CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCXTST(0x8u);

  //wait for clock to start
  while(!(PMC->PMC_SR & PMC_SR_MOSCXTS));

  //select clock
  PMC->CKGR_MOR = CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCSEL | CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCXTST(0x8u);

  //wait for select
  while(!(PMC->PMC_SR & PMC_SR_MOSCSELS));

  //disable PMC clock
  //PMC->CKGR_PLLAR = CKGR_PLLAR_ONE;
  //PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | CKGR_PLLAR_MULA(0xdUL) | CKGR_PLLAR_DIVA(0x1UL) | CKGR_PLLAR_PLLACOUNT(0x3fU);

  //wait for PLL lock
  //while(!(PMC->PMC_SR & PMC_SR_LOCKA));

  PMC->PMC_MCKR = (PMC->PMC_MCKR & ~(uint32_t)PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;

  while(!(PMC->PMC_SR & PMC_SR_MCKRDY));


  PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | CKGR_PLLAR_MULA(0xdUL) | CKGR_PLLAR_DIVA(0x1UL) | CKGR_PLLAR_PLLACOUNT(0x3fU);

  //wait for lock
  while(!(PMC->PMC_SR & PMC_SR_LOCKA));


  PMC->PMC_MCKR = PMC_MCKR_PRES_CLK_2 | PMC_MCKR_CSS_MAIN_CLK;

  while(!(PMC->PMC_SR & PMC_SR_MCKRDY));

  PMC->PMC_MCKR = PMC_MCKR_PRES_CLK_2 | PMC_MCKR_CSS_PLLA_CLK;

  while(!(PMC->PMC_SR & PMC_SR_MCKRDY));

  //we now should be at 84 Mhz
}

#ifndef __NO_SYSTEM_INIT
void SystemInit()
{
  sysClockInit();

  REG_PMC_PCER0 |= PMC_PCER0_PID12; // enable PIO controller B
  REG_PIOB_PER |= PIO_PB27;
  REG_PIOB_OER |= PIO_PB27;
  REG_PIOB_OWER |= PIO_PB27;

  //disable watchdog
  REG_WDT_MR &= ~WDT_MR_WDRSTEN;

  //init core modules
  initUART();
}
#endif

void defaultVector(){
  while(1){
    sleep_ms(1000);
    REG_PIOB_SODR |= PIO_PB27;
    sleep_ms(1000);
    REG_PIOB_CODR |= PIO_PB27;
  }
}



int main(void){
  //void * hTest = malloc(10);
  char * input = "HELLO WORLD\n";
  *timer_mode_register = 0x00000020;
  int offset = 0;
  while(1){
    sleep_ms(200);
    _write(0,input + offset,12);
    offset ++;
    if(offset > 12){
      offset = 0;
    }
    sleep_ms(200);
  }
  //printf("malloc at address %x", (unsigned long)hTest);
}
