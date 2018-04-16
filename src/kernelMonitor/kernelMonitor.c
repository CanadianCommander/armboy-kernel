#include "kernelMonitor.h"
#include "../config.h"
#include "../hardware/hardware.h"
#include "../util/arrayOps.h"
#include <stdio.h>
#include <string.h>

static kMonitorHandler monitorList[KERNEL_MONITOR_HANDLER_MAX];
static uint32_t monitorListLen = 0;

bool hasPending(){
  if(lineReadyUart()){
    return true;
  }
  else {
    return false;
  }
}

void servicePendingOperations(){
  char lineBuffer[KERNEL_MONITOR_LINE_BUFFER];
  memset(lineBuffer,0,KERNEL_MONITOR_LINE_BUFFER);

  getNextLineUart(lineBuffer, KERNEL_MONITOR_LINE_BUFFER);
  for(uint32_t i = 0; i < monitorListLen; i ++){
    if((monitorList[i])(lineBuffer)){
      //monitor just handled line
      return;
    }
  }

  printf("\"%s\" Command Not Found\n",lineBuffer);
}

/**
  add kernel monitor handler
  @param handler to add
*/
void addMonitor(kMonitorHandler mon){
  monitorList[monitorListLen] = mon;
  monitorListLen ++;
}

/**
  remove kernel monitor handler
  @param address of handler to remove
*/
void removeMonitor(kMonitorHandler mon){
  for(uint32_t i =0; i < monitorListLen; i ++){
    if(monitorList[i] == mon){
      //remove
      deletFromArray((uint8_t *)monitorList, i, 1, monitorListLen, sizeof(kMonitorHandler));
      monitorListLen --;
      i --;
    }
  }
}

bool memPeak(char * line);
void loadDefaultMonitorHandlers(void){
  addMonitor(memPeak);
}

// dump memory locations to terminal
bool memPeak(char * line){
  char callName[128];

  //check for invocation
  sscanf(line,"%128s",callName);
  if(strcmp(callName,"peek") == 0){
    printf("PEEK INVOKED!\n");
    return true;
  }
  else{
    return false;
  }
}
