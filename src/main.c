#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <sam3xa/include/sam3x8e.h>
#include "UART/uart.h"
#include "Timers/timers.h"

// set up processor clock. ??? Mhz -> 84 Mhz
void sysClockInit(void){
  // ???? Mhz
  EFC0->EEFC_FMR = EEFC_FMR_FWS(4);
  EFC1->EEFC_FMR = EEFC_FMR_FWS(4);

  PMC->CKGR_MOR = CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCXTST(0x8u);

  //wait for clock to start
  while(!(PMC->PMC_SR & PMC_SR_MOSCXTS));

  //select clock
  PMC->CKGR_MOR = CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCSEL | CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCXTST(0x8u);

  //wait for select
  while(!(PMC->PMC_SR & PMC_SR_MOSCSELS));

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
  initRTT();
}
#endif

//dump here if interrupt not handled!
void defaultVector(){
  while(1){
    sleep(1000);
    REG_PIOB_SODR |= PIO_PB27;
    sleep(1000);
    REG_PIOB_CODR |= PIO_PB27;
  }
}


int main(void){

  printf("=== ARM Boy ===\n");

  char input[25];
  while(1){
    memset(input,0,25);
    sleep(1000);
    scanf("%s",input);
    printf("GOT: %s \n", input);
  }
}
