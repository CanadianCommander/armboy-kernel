#include "memoryManager.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../kernelMonitor/kernelMonitor.h"
#include "../processManager/processManager.h"

//holds all memory handles
static struct MemoryHandle memoryHandles[MEMORY_CHUNK_MAX];

struct MemoryHandle * requestMemory(uint32_t size, uint32_t heapStart, pid_t owner){
  for(int i = 0; i < MEMORY_CHUNK_MAX; i++){
    if(memoryHandles[i].owner == 0){
      //free handle, allocate
      memoryHandles[i].owner = owner;
      memoryHandles[i].len = size;
      memoryHandles[i].heapStart = heapStart;
      memoryHandles[i].memptr = malloc_pid(size, KERNEL_PID);
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
  free(memH->memptr);
}

void transferMemory(struct MemoryHandle * memH, pid_t newOwner){
  memH->owner = newOwner;
}


void allocateKernelMemory(uint8_t * startAddr, uint32_t size){
  //give memory handle zero to the kernel... told you this was dangerous
  memoryHandles[0].owner = KERNEL_PID;
  memoryHandles[0].len = size;
  memoryHandles[0].heapStart = 1;
  memoryHandles[0].memptr = startAddr;
  //setup heap
  initializeMemoryRegion(startAddr + 1, size -1);
}

struct MemoryHandle * getAllocatedMemory(pid_t owner, uint32_t * iter){
  for(uint32_t i = *iter; i < MEMORY_CHUNK_MAX; i++){
    if(memoryHandles[i].owner == owner){
      *iter = i+1;
      return &(memoryHandles[i]);
    }
  }
  return NULL;
}

void initializeMemoryRegion(uint8_t * memptr, uint32_t len){
  uint32_t * start = (uint32_t *)memptr;
  *start = (uint32_t)(start + 1); // top pointer
  *(start + 1) = BLOCK_HEADER(len - HEAP_HEADER_SIZE,0,0,1,1); // one big free block
  //thats it :)
}

void * __malloc_pid(size_t size, struct MemoryHandle * mhandle);
void * malloc_pid(size_t size, pid_t pid){
  if(size < 4){
    //size must be 4 or more
    size = 4;
  }

  uint32_t iter=0;
  struct MemoryHandle * mhand = getAllocatedMemory(pid,&iter);

  while(mhand != NULL){
    void * res = __malloc_pid(size, mhand);

    if(!res){
      mhand = getAllocatedMemory(pid,&iter);
    }
    else {
      //memory allocated
      return res;
    }
  }
  return NULL;
}

void * __malloc_pid(size_t size, struct MemoryHandle * mhandle){
  if(mhandle->heapStart == 0){
    //zero means no heap! abort
    return NULL;
  }
  uint8_t * heapPtr = mhandle->memptr + mhandle->heapStart;

  //look for free region starting at top ptr.
  heapPtr = (uint8_t *) *(uint32_t*)heapPtr;
  uint8_t * startPoint = heapPtr;

  //look for memory
  while(1){
    if(BLOCK_LEN(*(uint32_t *)heapPtr) > size && !(*(uint32_t*)heapPtr & BLOCK_ALLOCATED) && !(*(uint32_t*)heapPtr & BLOCK_DELETED)){
      //use this block
      uint8_t wasEnd = (*(uint32_t*)heapPtr & HEAP_END) >> 3;
      uint32_t oldSize = BLOCK_LEN(*(uint32_t *)heapPtr);

      //rewrite header
      if(oldSize - size <= 4){
        //use entire chunk
        *(uint32_t*)heapPtr = BLOCK_HEADER(oldSize,1,((*(uint32_t*)heapPtr & BLOCK_LAST_ALLOC) >> 1),((*(uint32_t*)heapPtr & HEAP_BASE) >> 2), ((*(uint32_t*)heapPtr & HEAP_END) >> 3));

        //update top ptr to next block
        if(!(*(uint32_t*)heapPtr & HEAP_END)){
          *(uint32_t *)(mhandle->memptr + mhandle->heapStart) = (uint32_t)(heapPtr + BLOCK_LEN(*(uint32_t *)heapPtr));
        }
        else {
          //roll around
          *(uint32_t *)(mhandle->memptr + mhandle->heapStart) = (uint32_t)(mhandle->memptr + mhandle->heapStart + HEAP_HEADER_SIZE);
        }

        return heapPtr + HEAP_HEADER_SIZE;
      }
      else {
        //split
        *(uint32_t*)heapPtr = BLOCK_HEADER(size,1,((*(uint32_t*)heapPtr & BLOCK_LAST_ALLOC) >> 1),((*(uint32_t*)heapPtr & HEAP_BASE) >> 2), 0);
        void * retAddr = heapPtr + HEAP_HEADER_SIZE;
        // write new header at split point
        heapPtr = heapPtr + size + HEAP_HEADER_SIZE;
        *(uint32_t*)heapPtr = BLOCK_HEADER(oldSize - size - HEAP_HEADER_SIZE, 0, 1, 0, wasEnd);

        //set top pointer to new header
        *(uint32_t *)(mhandle->memptr + mhandle->heapStart) = (uint32_t)heapPtr;

        return retAddr;
      }

    }
    else {
      //continue search
      if(*(uint32_t *)heapPtr & HEAP_END){
        //we are at end of heap! roll around to first block
        heapPtr = mhandle->memptr + mhandle->heapStart + HEAP_HEADER_SIZE;
      }
      else {
        // go to next block
        heapPtr = heapPtr + BLOCK_LEN(*(uint32_t *)heapPtr) + HEAP_HEADER_SIZE;
      }

      if(heapPtr == startPoint){
        //no space available
        return NULL;
      }
    }
  }
}

void * malloc(size_t size){
  pid_t currPid = KERNEL_PID;
  //TODO get curr pid from process manager
  return malloc_pid(size,currPid);
}

void free(void * ptr){
  //de alloc
  ptr = (void *)((uint8_t*)ptr - HEAP_HEADER_SIZE);
  *(uint32_t*)ptr &= ~(BLOCK_ALLOCATED);

  //write block len in last 4 bytes of user data space
  uint32_t * tailPtr = (uint32_t*)((uint8_t *)ptr + BLOCK_LEN(*(uint32_t *)ptr));
  *tailPtr = BLOCK_LEN(*(uint32_t *)ptr);

  if(!(*(uint32_t*)ptr & HEAP_END)){
    uint32_t * nextPtr = (uint32_t*)(((uint8_t*)ptr) + BLOCK_LEN(*(uint32_t*)ptr) + HEAP_HEADER_SIZE);

    //tell next guy we are free :)
    *nextPtr &= ~BLOCK_LAST_ALLOC;

    if(!(*nextPtr & BLOCK_ALLOCATED)){
      //block after not allocated merge.
      uint32_t currLen = BLOCK_LEN(*(uint32_t*)ptr);
      *(uint32_t*)ptr &= 0x000000f7;
      *(uint32_t*)ptr |= SET_BLOCK_LEN(currLen + HEAP_HEADER_SIZE + BLOCK_LEN(*nextPtr));

      if(*nextPtr & HEAP_END){
        *(uint32_t*)ptr |= HEAP_END;
      }

      //mark block that we merged with as deleted
      *nextPtr |= BLOCK_DELETED;

      //update tail
      tailPtr = (uint32_t*)((uint8_t *)ptr + BLOCK_LEN(*(uint32_t *)ptr));
      *tailPtr = BLOCK_LEN(*(uint32_t *)ptr);
    }
  }

  if(!(*(uint32_t*)ptr & BLOCK_LAST_ALLOC) && !(*(uint32_t*)ptr & HEAP_BASE)){
    //block before not allocated. merge
    uint32_t * lastBlockPtr = (uint32_t*)(((uint8_t*)ptr - HEAP_HEADER_SIZE) - *((uint32_t*)ptr - 1));

    //adjust length of block
    uint32_t lastBLen = BLOCK_LEN(*lastBlockPtr);
    *lastBlockPtr &= 0x000000f7;
    *lastBlockPtr |= SET_BLOCK_LEN(lastBLen + HEAP_HEADER_SIZE + BLOCK_LEN(*(uint32_t*)ptr));

    if(*(uint32_t *)ptr & HEAP_END){
      *lastBlockPtr |= HEAP_END;
    }

    //mark self as deleted
    *(uint32_t*)ptr |= BLOCK_DELETED;

    //update tail
    tailPtr = (uint32_t*)((uint8_t *)lastBlockPtr + BLOCK_LEN(*lastBlockPtr));
    *tailPtr = BLOCK_LEN(*lastBlockPtr);
  }
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

// kernel monitor debug functions bellow --------------------
static bool memDebugKernelMonitorHandler(char * line){
  char myName[128];

  sscanf(line,"%128s",myName);
  if(strcmp(myName,"memdump") == 0){
    listRegionsUart();
    return true;
  }
  else {
    return false;
  }
}

static bool memMallocKernelMonitor(char * line){
  char myName[128];

  sscanf(line,"%128s",myName);
  if(strcmp(myName,"malloc") == 0){
    uint32_t size = 0;
    sscanf(line, "%*s %d", &size);
    if(size == 0){
      printf("bad arguments. usage: malloc <size in bytes>\n");
    }
    else {
      void * addr = malloc_pid(size,KERNEL_PID);
      printf("memory allocated @ %p\n", addr);
    }
    return true;
  }
  else {
    return false;
  }
}

static bool memFreeKernelMonitor(char * line){
  char myName[128];

  sscanf(line,"%128s",myName);
  if(strcmp(myName,"free") == 0){
    uint32_t addr = 0;
    sscanf(line, "%*s %x", &addr);
    if(addr == 0){
      printf("bad arguments. usage: free <address>\n");
    }
    else {
      free((uint8_t*)addr);
      printf("memory freed @ %p\n", addr);
    }
    return true;
  }
  else {
    return false;
  }
}

static bool heapWalkKernelMonitorHandler(char * line){
  char myName[128];

  sscanf(line,"%128s",myName);
  if(strcmp(myName,"hwalk") == 0){
    uint32_t addr = 0;
    sscanf(line, "%*s %x", &addr);
    if(addr == 0){
      printf("bad arguments. usage: hwalk <heap start>\n");
    }
    else {
      uint32_t * bhead = (uint32_t *)addr;
      uint32_t count = 0;
      printf("block num | len | end | base | lalloc | alloc\n");
      while(1){
        printf("Block %d @ %p: %d, %d, %d, %d, %d\n", count, bhead, BLOCK_LEN(*bhead), (*bhead & HEAP_END) >> 3, (*bhead & HEAP_BASE) >> 2, (*bhead & BLOCK_LAST_ALLOC) >> 1, (*bhead & BLOCK_ALLOCATED));
        count ++;
        if((*bhead & HEAP_END)){
          printf("--- Done ---\n");
          break;
        }
        bhead = (uint32_t *)(((uint8_t*)bhead) + BLOCK_LEN(*bhead) + HEAP_HEADER_SIZE);
      }
    }
    return true;
  }
  else {
    return false;
  }
}


void addMemoryDebugKernelMonitor(){
  addMonitor(memDebugKernelMonitorHandler);
  addMonitor(memMallocKernelMonitor);
  addMonitor(memFreeKernelMonitor);
  addMonitor(heapWalkKernelMonitorHandler);
}
