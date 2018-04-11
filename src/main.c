#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <sam3xa/include/sam3x8e.h>
#include "config.h"
#include "util/debug.h"
#include "hardware/hardware.h"

#ifndef __NO_SYSTEM_INIT
void SystemInit()
{
  setupMainClock();

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
  printSysInfo();

  printf("=== ARM Boy ===\n");
  char input[300];
  while(1){
    memset(input,0,300);
    sleep(1000);
    printf("Entre page data: \n");
    scanf("%s",input);
    if(input[0] == 'r'){
      uint8_t foo[256];
      memset(foo,0,256);
      printf("Reading page...\n");
      if(!readPage(1,0,foo)){
        printf("ERROR\n");
      }
      dumpHex((uint8_t *)foo,256);
    }
    else {
      printf("\nWriting page...\n");
      if(!writePage(1,0,input))
      {
        printf("ERROR\n");
      }
    }
  }
}
