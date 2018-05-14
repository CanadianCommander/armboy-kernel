#include "syscallDispatcher.h"
#include "processManager.h"

__attribute__ ((naked)) void SVC_IRQ(void){
  asm(
  "push {lr} \n"
  "mrs r0, PSP \n"
  "bl SVC_IRQ_main \n"
  "pop {lr} \n"
  "bx lr\n"
  :
  :
  :
  );
}

void SVC_IRQ_main(unsigned int * svc_args){
  /*    * Stack contains:    * r0, r1, r2, r3, r12, r14, the return address and xPSR */
  unsigned int svc_number;
  svc_number = ((char *)svc_args[6])[-2];
  switch(svc_number){
    case SYS_CALL_YIELD:
      break;
    case SYS_CALL_ABORT:
      break;
    case SYS_CALL_BLOCK:
      break;
    case SYS_CALL_MOD:
      //r0 contains cid in the high half word and the jVec in the low half word
      //r1 contains arg
      doModuleCall((0xFFFF0000 & (svc_args[0])) >> 16, (0x0000FFFF & (svc_args[0])), svc_args[1]);
      break;
    case SYS_CALL_LOAD_MOD:
      loadKernelModule(svc_args[0]);
      break;
    default:
      break;
  }
}


void doModuleCall(uint32_t cid, uint8_t jVec, uint32_t arg){
  struct ProcessDescriptor * pd = findProcessDescriptorCid(cid);
  if(pd){
    uint32_t jump = (*((uint32_t*)pd->jumpTableStart + jVec) + (uint32_t)pd->binaryAddress);
    asm(
      "mov r9, %[staticB] \n"
      "mov r0, %[arg]\n"
      "blx %[jumpAddr] "
      :
      : [jumpAddr] "r" (jump),
        [staticB] "r" ((uint32_t)pd->staticBase),
        [arg] "r" (arg)
      : "r0", "r1", "r2", "r3", "r9", "pc", "sp", "lr", "memory"
    );
  }
}
//00080a0c
