#include <stdio.h>
#include <sam3xa/include/sam3x8e.h>

#include "config.h"
#include "util/debug.h"
#include "hardware/hardware.h"
#include "kernelMonitor/kernelMonitor.h"
#include "memoryManager/memoryManager.h"
#include "flashManager/flashManager.h"
#include "processManager/processManager.h"
#pragma import(__use_no_heap)

#ifndef __NO_SYSTEM_INIT
void SystemInit()
{
  setupMainClock();

  enableIOBank(PIO_BANK_B);
  enablePin(PIO_BANK_B, PIO_PB27, PIO_OUT);

  //disable watchdog
  REG_WDT_MR &= ~WDT_MR_WDRSTEN;
}
#endif

//dump here if interrupt not handled!
void defaultVector(){
  uint32_t Maddr = 0;
  uint32_t Paddr = 0;
  uint32_t psr = 0;
  asm(
    "mrs %[ma], msp\n"
    "mrs %[pa], psp\n"
    "mrs %[psr], psr\n"
  : [ma] "=r" (Maddr), [pa] "=r" (Paddr), [psr] "=r" (psr)
  :
  :);
  printf("---CRASH DUMP---\n");
  printf("PSR: %x\n", psr);
  printf("---MAIN STACK---\n");
  dumpHex((uint8_t*)Maddr, 100);
  if(Paddr > 0x20070000 && Paddr < 0x20089999){
    printf("---PROC STACK---\n");
    dumpHex((uint8_t*)Paddr, 100);
  }
  printf("###CRASH DUMP###\n");

  dumpHex((uint8_t*)0x20071b09, 200);

  while(1){
    sleep(1000);
    REG_PIOB_SODR |= PIO_PB27;
    sleep(1000);
    REG_PIOB_CODR |= PIO_PB27;
  }
}

int main(void){
  //init core modules
  initUART();
  initRTT();
  //allocate dynamic memory. see config.h for size / location
  allocateKernelMemory((uint8_t*)KERNEL_DYNAMIC_MEMORY_START,KERNEL_DYNAMIC_MEMORY);

  systickConfig(CONTEXT_SWITCH_INTERVAL);
  NVIC_SetPriority(PendSV_IRQn, 255);

  //load monitor handlers
  loadDefaultMonitorHandlers();
  addMemoryDebugKernelMonitor();
  addFlashKernelMonitorFunctions();
  addProgManagerKernelMonitorFunctions();

  printf("===ARMBoy===\n");
  for(;;){
    if(hasPending()){
      servicePendingOperations();
    }

    currentPd = getNextReadyProcess();
    if(currentPd){
      currentPd->proc_state = PROCS_RUNNING;

      runProcess(currentPd);

      if(currentPd->proc_state == PROCS_RUNNING){
        currentPd->proc_state = PROCS_READY;
      }
    }
  }
}
