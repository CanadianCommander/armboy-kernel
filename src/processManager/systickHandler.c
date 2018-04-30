#include "systickHandler.h"
#include "../util/debug.h"
#include "../processManager/processManager.h"
#include "../config.h"
#include "../hardware/hardware.h"

volatile bool toUser = false;
void SYS_TICK_IRQ(void){
  toUser = false;
  triggerPendSV();
}

__attribute__ ((naked)) void PENDSV_IRQ(void) {
  asm("push {r3}\n");
  if(toUser){
    asm("pop {r3} \n");
    SAVE_STATE();
    RESTORE_STATE_PSP();
    IRQ_RET_PSP();
  }
  else{
    asm("pop {r3} \n");
    SAVE_STATE_PSP();
    READ_PSP((uint32_t)currentPd->stackPtr);
    RESTORE_STATE();
    IRQ_RET_MSP();
  }
}

__attribute__ ((naked)) void __launchProcess(uint32_t * sp){
  asm("push {lr}\n");
  MSP_TO_PSP((uint32_t)sp);
  triggerPendSV();
  asm("pop {pc}\n");
}

void launchProcess(uint32_t * sp){
  toUser = true;
  startSysTick();
  __launchProcess(sp);
  //return here in CONTEXT_SWITCH_INTERVAL ms
  stopSysTick();
}
