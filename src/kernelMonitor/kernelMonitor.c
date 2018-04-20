#include "kernelMonitor.h"
#include "../config.h"
#include "../hardware/hardware.h"
#include "../util/arrayOps.h"
#include "../util/debug.h"
#include <stdio.h>
#include <string.h>

static struct kmHandle {
  kMonitorHandler mon;
  char * name;
} kmHandle;

static struct kmHandle monitorList[KERNEL_MONITOR_HANDLER_MAX];
static uint32_t monitorListLen = 0;

bool hasPending(){
  if(lineReadyUart()){
    return true;
  }
  else {
    return false;
  }
}

static void processBackspace(char * lineBuffer);
static bool __NameCheck(char * name, char * line);
void servicePendingOperations(){
  char lineBuffer[KERNEL_MONITOR_LINE_BUFFER];
  memset(lineBuffer,0,KERNEL_MONITOR_LINE_BUFFER);

  getNextLineUart(lineBuffer, KERNEL_MONITOR_LINE_BUFFER);
  processBackspace(lineBuffer);

  for(uint32_t i = 0; i < monitorListLen; i ++){
    if(monitorList[i].name == NULL || __NameCheck(monitorList[i].name, lineBuffer)){
      if((monitorList[i].mon)(lineBuffer)){
        //monitor just handled line
        return;
      }
    }
  }

  printf("\"%s\" Command Not Found\n",lineBuffer);
}

static bool __NameCheck(char * name, char * line){
  char nbuff[128];
  memset(nbuff,0,128);
  sscanf(line, "%128s", nbuff);
  if(strcmp(nbuff,name) == 0){
    return true;
  }
  else {
    return false;
  }
}

/**
  add kernel monitor handler
  @param handler to add
*/
void addMonitor(kMonitorHandler mon){
  monitorList[monitorListLen].mon = mon;
  monitorList[monitorListLen].name = NULL;
  monitorListLen ++;
}

void addMonitorWName(kMonitorHandler mon, char * name){
  monitorList[monitorListLen].mon = mon;
  monitorList[monitorListLen].name = name;
  monitorListLen ++;
}

/**
  remove kernel monitor handler
  @param address of handler to remove
*/
void removeMonitor(kMonitorHandler mon){
  for(uint32_t i =0; i < monitorListLen; i ++){
    if(monitorList[i].mon == mon){
      //remove
      deletFromArray((uint8_t *)monitorList, i, 1, monitorListLen, sizeof(kmHandle));
      monitorListLen --;
      i --;
    }
  }
}

bool memPeak(char * line);
bool memPoke(char * line);
void loadDefaultMonitorHandlers(void){
  addMonitor(memPeak);
  addMonitor(memPoke);
}

// dump memory locations to terminal
bool memPeak(char * line){
  char callName[128];

  //check for invocation
  sscanf(line,"%128s",callName);
  if(strcmp(callName,"peek") == 0){
    uint32_t start, len = 0;
    sscanf(line,"%*s %x %x", &start, &len);
    if(start != len && len != 0){
      dumpHex((uint8_t*)start,len);
    }
    else {
      printf("bad arguments for peek. format is peek <start> <len>\n");
    }
    return true;
  }
  else{
    return false;
  }
}

//change the value of a memory word.
bool memPoke(char * line){
  char callName[128];

  //check for invocation
  sscanf(line,"%128s",callName);
  if(strcmp(callName,"poke") == 0){
    uint32_t addr = 0;
    uint32_t value = 0;
    sscanf(line,"%*s %x %x", &addr, &value);
    if(addr != 0){
      *((uint32_t *)addr) = value;
    }
    else {
      printf("bad arguments for poke. format is poke <address> <word value>\n");
    }
    return true;
  }
  else {
    return false;
  }
}

static void processBackspace(char * lineBuffer){
  for(int i = 0; i < KERNEL_MONITOR_LINE_BUFFER; i++){
    if(lineBuffer[i] == 0x0){
      break;
    }
    else if (lineBuffer[i] == 0x8){
      //backspace character
      deletFromArray((uint8_t*)lineBuffer,i - 1, 2, KERNEL_MONITOR_LINE_BUFFER, 1);
      i -= 2;
    }
  }
}
