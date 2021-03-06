#include "processManager.h"
#include "../flashManager/flashManager.h"
#include "../memoryManager/memoryManager.h"
#include "../kernelMonitor/kernelMonitor.h"
#include "../config.h"
#include "../util/misc.h"
#include "../util/debug.h"
#include "../hardware/hardware.h"
#include "systickHandler.h"
#include <memory.h>

static struct ProcessDescriptor pdList[MAX_PROCESS];

volatile struct ProcessDescriptor * currentPd = NULL;

static void parseSRAMHeader(struct FlashHeader * fh, uint8_t * addr);

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
  pd->cid = 0;
  pd->proc_state = PROCS_DEAD;
}

#define FUNCTION_BASE_ADDR 0x80000
static void updateGotTable(uint8_t * textAddr, uint8_t * gotAddress, struct FlashHeader * fh){
  //update got table
  uint32_t * wGot = (uint32_t *)(gotAddress);
  for(uint32_t i = 0; i < (fh->got_end  - fh->got_start)/4; i ++){
    if(*(wGot + i) < FUNCTION_BASE_ADDR){
      if(*(wGot + i) < fh->text_end){
        //module function
        *(wGot + i) = *(wGot + i) + (uint32_t)textAddr;
      }
      else{
        //data
        *(wGot + i) = (*(wGot + i) - fh->got_start) + (uint32_t)gotAddress;
      }
    }
  }
}

static void * relocateData(uint8_t * targetAddr, struct FlashHeader * fh, uint8_t * srcAddr){
  //word alige
  targetAddr = targetAddr + WORD - ((uint32_t)targetAddr % WORD);

  uint8_t * data = srcAddr + fh->data_start;
  uint8_t * bss  = srcAddr + fh->bss_start;
  uint8_t * got  = srcAddr + fh->got_start;
  uint32_t dlen  = (fh->data_end - fh->data_start);
  uint32_t blen  = (fh->bss_end  - fh->bss_start);
  uint32_t glen  = (fh->got_end  - fh->got_start);


  memcpy(targetAddr, got, glen);
  memcpy(targetAddr + glen, data, dlen);
  memset(targetAddr + dlen + glen, 0, blen);

  //update got table
  updateGotTable(srcAddr, targetAddr, fh);
  return (void*)targetAddr;
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
    if(pdList[i].cid == cid && pdList[i].pid != BAD_PID){
      return pdList + i;
    }
  }
  return NULL;
}


struct ProcessDescriptor * loadProcess(void * binaryStartPtr,bool flash,uint8_t proc_type){
  struct ProcessDescriptor * freepd = findProcessDescriptor(BAD_PID);

  if(freepd){
    freepd->pid = __getNewPid(freepd);
    freepd->proc_type = proc_type;
    freepd->block_code = 0;
    freepd->cid = 0;
    freepd->staticBase = 0;
    freepd->binaryAddress = binaryStartPtr;
    freepd->stackPtr = NULL;

    if(flash){
      int page = addressToFlashPage(binaryStartPtr);
      if(page != -1){
        struct FlashHeader fh;
        struct MemoryHandle * mh;

        parseFlashHeader((uint32_t)page, &fh);
        uint32_t staticSize = (fh.got_end - fh.got_start) + (fh.data_end - fh.data_start) + (fh.bss_end - fh.bss_start);
        freepd->jumpTableStart = fh.jumpTableStart;
        freepd->cid = fh.id;

        mh = requestMemory(fh.reqHeapSize + staticSize + WORD, staticSize + WORD, freepd->pid);

        freepd->staticBase = relocateData(mh->memptr, &fh, binaryStartPtr);
      }
      else {
        return NULL;
      }
    }
    else {
      struct FlashHeader fh;
      parseSRAMHeader(&fh, (uint8_t*)binaryStartPtr);
      freepd->jumpTableStart = fh.jumpTableStart;
      updateGotTable((uint8_t*)binaryStartPtr, (uint8_t*)(binaryStartPtr) + fh.got_start, &fh);
    }

    freepd->proc_state = PROCS_READY;
    return freepd;
  }
  else {
    return NULL;
  }
}

struct ProcessDescriptor * loadKernelModule(uint16_t mid){
  uint32_t page = locateModule(mid);
  if(page){
    struct ProcessDescriptor * pd = loadProcess(getFlashStartAddress(0) + page*FLASH_PAGE_SIZE, true, PROC_TYPE_KMOD);
    if(pd){
      return pd;
    }
    else {
      return NULL;
    }
  }
  else {
    return NULL;
  }
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

static volatile uint32_t lastRunProcess = 0;
struct ProcessDescriptor * getNextReadyProcess(){
  for(int i =0; i < 2; i ++){
    for(int z = (lastRunProcess+1)*(1 - i); z < MAX_PROCESS*(1 - i) + (lastRunProcess+1)*(i);z++){
      if(pdList[z].pid != BAD_PID && pdList[z].proc_type == PROC_TYPE_USER && pdList[z].proc_state == PROCS_READY){
        lastRunProcess = z;
        return pdList + z;
      }
    }
  }
  return NULL;
}

static uint32_t* initializeStack(uint32_t * sPtr, void * bin, void * jmp);
void runProcess(volatile struct ProcessDescriptor * pd){
  if(pd->stackPtr){
    //resume
    launchProcess(pd->stackPtr);
  }
  else {
    //start for first time.
    //allocate stack
    uint32_t iter = 0;
    struct MemoryHandle * mh = getAllocatedMemory(pd->pid, &iter);
    if(mh){
      pd->stackPtr = alignPtr((void*)(mh->memptr + mh->len - 4), WORD);
      pd->stackPtr = initializeStack(pd->stackPtr,pd->binaryAddress,pd->jumpTableStart);
      runProcess(pd);
    }
  }
}

#define INITIAL_STATUS_REG 0x01000000 // all default / zero. W/ inturrupt bit set to SysTick inturrupt.
static uint32_t* initializeStack(uint32_t * sPtr, void * binAddr, void * jumpAddr){
  *sPtr = INITIAL_STATUS_REG;
  sPtr--;
  *sPtr = (uint32_t)binAddr + *(uint32_t*)jumpAddr;//pc
  sPtr--;
  *sPtr = 0x0;//lr

  //r0 - r3 + r12 all zero
  memset(sPtr - 5,0,5*4);
  sPtr = sPtr - 5;

  //r4 - r11 all zero
  memset(sPtr - 8,0,8*4);
  sPtr = sPtr - 8;

  /****** Stack ********//*
  xPSR
  PC
  LR
  R12
  R3
  R2
  R1
  R0
  --- custom below. HW stack above
  R11
  R10
  R9
  R8
  R7
  R6
  R5
  R4
  *//********************/

  return sPtr;
}

static void parseSRAMHeader(struct FlashHeader * fh, uint8_t * addr){
  memset(fh, 0 , sizeof(struct FlashHeader));

  uint32_t * ptr = (uint32_t*)addr;
  fh->text_start    = *ptr++;
  fh->text_end      = *ptr++;
  fh->got_start     = *ptr++;
  fh->got_end       = *ptr++;
  fh->data_start    = *ptr++;
  fh->data_end      = *ptr++;
  fh->bss_start     = *ptr++;
  fh->bss_end       = *ptr++;
  fh->reqHeapSize   = *ptr++;
  fh->reqStackSize  = *ptr++;
  fh->jumpTableStart = (void*)ptr;
}


static bool insmod(char * line);
static bool rmmod(char * line);
static bool lsmod(char * line);
static bool call(char * line);
static bool jump(char * line);
void addProgManagerKernelMonitorFunctions(void){
  addMonitorWName(insmod,"insmod");
  addMonitorWName(rmmod, "rmmod");
  addMonitorWName(lsmod, "lsmod");
  addMonitorWName(call, "call");
  addMonitorWName(jump, "jump");
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
    if(pd){
      uint32_t jump = (*((uint32_t*)pd->jumpTableStart + jv) + (uint32_t)pd->binaryAddress);
      printf("jumping to %x with SB: %x \n", jump, (uint32_t)pd->staticBase);
      asm(
        "mov r9, %[staticB] \n"
        "blx %[jumpAddr] "
        :
        : [jumpAddr] "r" (jump),
          [staticB] "r" ((uint32_t)pd->staticBase)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "pc", "sp", "lr", "memory"
      );
    }
    else{
      printf("Error, could not find process with module id %x did you load it?\n", cid);
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

static bool jump(char * line){
  uint32_t jumpAddr = 0;
  sscanf(line, "%*s %x", &jumpAddr);
  if(jumpAddr){
    printf("jumping to %x\n", jumpAddr);
    asm(
      "blx %[jumpAddr] "
      :
      : [jumpAddr] "r" (jumpAddr)
      : "r0", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "r11", "r12", "pc", "sp", "lr", "memory"
    );
  }
  else{
    printf("ERROR, usage: jump <address>\n");
  }
  return true;
}
