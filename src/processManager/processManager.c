#include "processManager.h"
#include "../flashManager/flashManager.h"
#include "../memoryManager/memoryManager.h"
#include "../kernelMonitor/kernelMonitor.h"
#include "../config.h"
#include "../util/debug.h"
#include "../hardware/hardware.h"
#include <memory.h>

static struct ProcessDescriptor pdList[MAX_PROCESS];

static pid_t __getNewPid(struct ProcessDescriptor *pd){
  for(int i =0; i < MAX_PROCESS; i ++){
    if(pd == (pdList + i)){
      return i + 2;
    }
  }
  return 0;
}

static void __freePid(struct ProcessDescriptor * pd){
  pd->pid = BAD_PID;
  pd->proc_state = PROCS_DEAD;
}

#define FUNCTION_BASE_ADDR 0x80000
static void * relocateData(uint8_t * targetAddr, struct FlashHeader * fh, uint8_t * srcAddr){
  uint8_t * data = srcAddr + fh->data_start;
  uint8_t * bss  = srcAddr + fh->bss_start;
  uint8_t * got  = srcAddr + fh->got_start;
  uint32_t dlen  = (fh->data_end - fh->data_start);
  uint32_t blen  = (fh->bss_end  - fh->bss_start);
  uint32_t glen  = (fh->got_end  - fh->got_start);


  memcpy(targetAddr, got, glen);
  memcpy(targetAddr + glen, data, dlen);
  memcpy(targetAddr + dlen + glen, bss, blen);

  //update got table
  uint32_t * wGot = (uint32_t *)(targetAddr);
  for(uint32_t i = 0; i < glen/4; i ++){
    if(*(wGot + i) < FUNCTION_BASE_ADDR){
      *(wGot + i) = (*(wGot + i) - fh->got_start) + (uint32_t)targetAddr;
    }
  }
  return (void*)wGot;
}

struct ProcessDescriptor * findProcessDescriptor(pid_t pid){
  for(int i =0; i < MAX_PROCESS; i ++){
    if(pdList[i].pid == pid){
      return pdList + i;
    }
  }
  return NULL;
}

struct ProcessDescriptor * findProcessDescriptorCid(uint32_t cid){
  for(int i =0; i < MAX_PROCESS; i ++){
    if(pdList[i].cid == cid){
      return pdList + i;
    }
  }
  return NULL;
}


struct ProcessDescriptor * loadProcess(void * binaryStartPtr,bool flash,uint8_t proc_type){
  struct ProcessDescriptor * freepd = findProcessDescriptor(BAD_PID);
  freepd->pid = __getNewPid(freepd);
  freepd->proc_type = proc_type;
  freepd->block_code = 0;
  freepd->cid = 0;
  freepd->staticBase = 0;
  freepd->binaryAddress = binaryStartPtr;

  if(freepd){
    if(flash){
      int page = addressToFlashPage(binaryStartPtr);
      if(page != -1){
        struct FlashHeader fh;
        struct MemoryHandle * mh;

        parseFlashHeader((uint32_t)page, &fh);
        uint32_t staticSize = (fh.got_end - fh.got_start) + (fh.data_end - fh.data_start) + (fh.bss_end - fh.bss_start);
        freepd->jumpTableStart = fh.jumpTableStart;
        freepd->cid = fh.id;

        mh = requestMemory(fh.reqHeapSize + staticSize, staticSize , freepd->pid);

        freepd->staticBase = relocateData(mh->memptr, &fh, binaryStartPtr);
      }
      else {
        return NULL;
      }
    }
    else {

    }
  }
  else {
    return NULL;
  }

  freepd->proc_state = PROCS_READY;
  return freepd;
}

bool unloadProcess(struct ProcessDescriptor * pd){
  if(pd){
    //free process memory
    uint32_t iter = 0;
    struct MemoryHandle * mh;
    mh = getAllocatedMemory(pd->pid,&iter);
    while(mh != NULL){
      releaseMemory(mh);
      mh = getAllocatedMemory(pd->pid,&iter);
    }

    __freePid(pd);
    return true;
  }
  else {
    return false;
  }
}

bool unloadProcessPid(pid_t pid){
  struct ProcessDescriptor * pd = findProcessDescriptor(pid);
  return unloadProcess(pd);
}

bool unloadProcessCid(uint32_t cid){
  struct ProcessDescriptor * pd = findProcessDescriptorCid(cid);
  return unloadProcess(pd);
}

static bool insmod(char * line);
static bool rmmod(char * line);
static bool lsmod(char * line);
static bool call(char * line);
void addProgManagerKernelMonitorFunctions(void){
  addMonitorWName(insmod,"insmod");
  addMonitorWName(rmmod, "rmmod");
  addMonitorWName(lsmod, "lsmod");
  addMonitorWName(call, "call");
}

static bool insmod(char * line){
  uint32_t mid = 0;
  sscanf(line, "%*s %x", &mid);
  if(mid){
    uint32_t page = locateModule(mid);
    if(page){
      struct ProcessDescriptor * pd = loadProcess(getFlashStartAddress(0) + page*FLASH_PAGE_SIZE, true, PROC_TYPE_KMOD);
      if(pd){
        printf("Module loaded!\n");
      }
      else {
        printf("Error loading module\n");
      }
    }
    else {
      printf("Error, no module with id %x\n", mid);
    }
  }
  else {
    printf("Error, usage: insmod <module id>\n");
  }
  return true;
}

static bool call(char * line){
  uint32_t cid = 0;
  int jv = -1;
  sscanf(line, "%*s %x %d",&cid,&jv);
  if(cid && jv != -1){
    struct ProcessDescriptor * pd = findProcessDescriptorCid(cid);
    if(1){
      uint32_t jump = (*((uint32_t*)pd->jumpTableStart + jv) + (uint32_t)pd->binaryAddress);
      asm(
        "mov r9, %[staticB] \n"
        "blx %[jumpAddr] "
        :
        : [jumpAddr] "r" (jump),
          [staticB] "r" ((uint32_t)pd->staticBase)
        : "r0", "r1", "r2", "r3", "r9", "pc", "sp", "lr", "memory"
      );
    }
    else{
      printf("Error, could not find process with module id %x did you load it?", cid);
    }
  }
  else {
    printf("Error, usage: call <module id> <jump vector>\n");
  }
  return true;
}

static bool rmmod(char * line){
  uint32_t cid = 0;
  sscanf(line, "%*s %x", &cid);
  if(cid){
    if(unloadProcessCid(cid)){
      printf("moudle unloaded!\n");
    }
    else {
      printf("could not unload module!\n");
    }
  }
  else {
    printf("bad arguments, usage: rmmod <module id>\n");
  }
  return true;
}

static bool lsmod(char * line){
  printf("----- kernel modules ------\n");
  for(int i =0; i < MAX_PROCESS; i ++){
    if(pdList[i].pid != BAD_PID && pdList[i].proc_type == PROC_TYPE_KMOD){
      struct FlashHeader fh;
      parseFlashHeader(locateModule(pdList[i].cid), &fh);

      printf("--- module: %.60s\n", fh.modName);
      printf("pid: %x\n", pdList[i].pid);
      printf("module id: %x\n", pdList[i].cid);
      printf("state: ");
      switch(pdList[i].proc_state){
        case PROCS_DEAD:
          printf("DEAD\n");
          break;
        case PROCS_READY:
          printf("READY\n");
          break;
        case PROCS_BLOCK:
          printf("BLOCKED\n");
          break;
        case PROCS_RUNNING:
          printf("RUNNING\n");
          break;
        default:
          printf("UNKNOWN\n");
          break;
      }
    }
  }
  printf("#########################\n");
  return true;
}
