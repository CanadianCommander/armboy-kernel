#include "coreFunctions.h"
#include "../config.h"
#include "../hardware/hardware.h"
#include "../processManager/processManager.h"

void doCoreFunction(uint16_t funcId, void * arg){
  switch(funcId){
    case CF_RANDOM:
      getRandom(arg);
      break;
    case CF_SLEEP:
      doSleep(arg);
      break;
    case CF_IS_LOADED:
      *(uint32_t*)arg = (uint32_t)isLoaded(*(uint32_t*)arg);
    default:
      printf("received unknown core function id: %d\n", funcId);
  }
}

// fill memory pointed to by rnd with 32bit random number
void getRandom(void * rnd){
  *(uint32_t*)rnd = random();
}

// sleep for the amount of milliseconds pointed to by ms
void doSleep(void * ms){
  sleep(*(uint32_t*)ms);
}

bool isLoaded(uint32_t cid){
  struct ProcessDescriptor * pd = findProcessDescriptorCid(cid);
  return (bool)pd;
}
