#include "memoryManager.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../kernelMonitor/kernelMonitor.h"

//holds all memory handles
static struct MemoryHandle memoryHandles[MEMORY_CHUNK_MAX];
static uint16_t iMemoryHandles = 0;

struct MemoryHandle * requestMemory(uint32_t size, pid_t owner){
  for(int i = iMemoryHandles; i < MEMORY_CHUNK_MAX*2; i++){
    if(memoryHandles[i].owner == 0){
      //free handle, allocate
      iMemoryHandles = i + 1;
      memoryHandles[i].owner = owner;
      memoryHandles[i].len = size;
      //memoryHandles[i].memptr = malloc(size);
      if(memoryHandles[i].memptr != NULL){
        return &memoryHandles[i];
      }
      else {
        return NULL;
      }
    }
  }

  return NULL;
}


void releaseMemory(struct MemoryHandle * memH){
  memH->owner = 0;
  //free(memH->memptr);
}


static void listRegionsUart(){
  printf("memory regions ---------\n");
  for(int i =0; i < MEMORY_CHUNK_MAX;i ++){
    if(memoryHandles[i].owner != 0){
      printf("pid: %d has address: %x with len %d \n",memoryHandles[i].owner, memoryHandles[i].memptr,memoryHandles[i].len);
    }
  }
  printf("end memory regions -----\n");
}

static bool memDebugKernelMonitorHandler(char * line){
  char myName[128];

  sscanf("%s",myName);
  if(strcmp(myName,"memdump") == 0){
    listRegionsUart();
    return true;
  }
  else {
    return false;
  }
}

void addMemoryDebugKernelMonitor(){
  addMonitor(memDebugKernelMonitorHandler);
}
