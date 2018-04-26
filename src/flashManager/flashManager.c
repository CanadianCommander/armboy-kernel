#include "flashManager.h"

#include <string.h>
#include <stdio.h>

#include "../config.h"
#include "../hardware/hardware.h"
#include "../util/arrayOps.h"
#include "../util/debug.h"
#include "../kernelMonitor/kernelMonitor.h"
#include "../memoryManager/memoryManager.h"
#include "../processManager/processManager.h"

// find a free flash location of the specified size. allocate and return its page number
static uint32_t __allocateFlash(uint32_t nPages);

void writeBinaryToFlash( struct PageAllocationStruct * pas, uint8_t * headerPage,uint32_t countPages){
  uint32_t startPage = __allocateFlash(countPages);
  if(startPage){
    pas->startPage = startPage;
    pas->nxtPage = startPage + 1;
    pas->pageCount = countPages;

    writePageAuto(startPage, headerPage);
  }
  else {
    pas->startPage = 0;
    pas->nxtPage = 0;
    pas->pageCount = 0;
  }
}

void writeNextPage( struct PageAllocationStruct * pas, uint8_t * pageData){
  if( (pas->nxtPage - pas->startPage) < pas->pageCount){
    writePageAuto(pas->nxtPage, pageData);
    pas->nxtPage++;
  }
}

void parseFlashHeader(uint32_t page, struct FlashHeader * header){
  uint8_t headerBuffer[FLASH_PAGE_SIZE];
  uint32_t pageAddr = (uint32_t)getFlashStartAddress(0) + page*FLASH_PAGE_SIZE;
  if(readPageAuto(page,headerBuffer)){
    memcpy(header->modName,headerBuffer,60);

    uint32_t * hptr = (uint32_t *)(headerBuffer + 60);
    header->id = *hptr++;
    header->text_start = *hptr++;
    header->text_end = *hptr++;
    header->got_start = *hptr++;
    header->got_end = *hptr++;
    header->data_start = *hptr++;
    header->data_end = *hptr++;
    header->bss_start = *hptr++;
    header->bss_end = *hptr++;
    header->reqHeapSize = *hptr++;
    header->jumpTableStart = (void*)(pageAddr + ((uint32_t)hptr - (uint32_t)headerBuffer));
  }
}

void clearAllocationPage(){
  uint8_t allocationPage[FLASH_PAGE_SIZE];
  memset(allocationPage, 0, FLASH_PAGE_SIZE);
  writePageAuto(PAGE_ALLOCATION_PAGE,allocationPage);
}

uint32_t locateModule(uint32_t id){
  uint8_t allocationPage[FLASH_PAGE_SIZE];
  if(readPageAuto(PAGE_ALLOCATION_PAGE, allocationPage)){
    for(uint32_t i = 0; i <= FLASH_LEN/2; i++){
      if(getBit(i,allocationPage)){
        struct FlashHeader fh;
        parseFlashHeader(i + FLASH_LEN/2, &fh);
        if(fh.id == id){
          return i + FLASH_LEN/2;
        }
      }
    }
  }
  return 0;
}

static uint32_t __allocateFlash(uint32_t nPages){
  uint8_t allocationPage[FLASH_PAGE_SIZE];
  if(readPageAuto(PAGE_ALLOCATION_PAGE, allocationPage)){
    uint32_t freeCount = 0;
    uint32_t pos = PAGE_ALLOCATION_PAGE + 1;
    for(pos; pos < FLASH_LEN; pos++){
      if(!getBit(pos,allocationPage)){
        //this page is free
        freeCount ++;
        if(freeCount >= nPages){
          //we have found a section of sufficent length!
          //allocate pages
          for(uint32_t i=(pos - (freeCount-1)); i <= pos; i++){
            setBit(i,1,allocationPage);
          }
          //set start of block bit
          setBit((pos - (freeCount - 1)) - FLASH_LEN/2, 1, allocationPage);
          //write changes to flash
          writePageAuto(PAGE_ALLOCATION_PAGE, allocationPage);
          return pos - (freeCount - 1);
        }
      }
      else {
        freeCount = 0;
      }
    }
    return (uint32_t)NULL;
  }
  else{
    return (uint32_t)NULL;
  }

}

bool kmUploadUser(char * line);
bool kmUpload(char * line);
bool kmDelete(char * line);
bool kmlsFlash(char * line);
void addFlashKernelMonitorFunctions(void){
  addMonitorWName(kmUploadUser, "u_upload");
  addMonitorWName(kmUpload, "upload");
  addMonitorWName(kmDelete, "delete");
  addMonitorWName(kmlsFlash, "lsf");
}

#define UPLOAD_USER_CHUNK_SIZE 64
bool kmUploadUser(char * line){
  uint32_t binLen = 0;
  uint32_t hLen = 0;
  uint32_t sLen = 0;
  uint32_t checksum = 0;
  sscanf(line, "%*s %d %d %d %u", &binLen, &hLen, &sLen, &checksum);
  if(binLen && binLen % UPLOAD_USER_CHUNK_SIZE == 0 && hLen && sLen && checksum){
    struct MemoryHandle * mh = requestMemory(binLen + hLen + sLen + 1, binLen, KERNEL_PID);
    if(mh){
      uint8_t * mPtr = mh->memptr;
      //half word align
      if((uint32_t)mPtr % 2 != 0)mPtr++;

      uint8_t buffer[UPLOAD_USER_CHUNK_SIZE];
      checksum = ~checksum + 1;

      for(uint32_t i = 0; i < binLen; i += UPLOAD_USER_CHUNK_SIZE){
        printf("next\n");

        //block waiting for 64 bytes on uart
        while(getRxBufferLenUart() < UPLOAD_USER_CHUNK_SIZE){
          asm("");
        }
        uint32_t nBytesRead = readDirectUART(buffer, UPLOAD_USER_CHUNK_SIZE);

        if(nBytesRead != UPLOAD_USER_CHUNK_SIZE){
          printf("ERROR, uart read error!\n");
          return true;
        }

        checksum = evalChecksum((uint32_t*)buffer,UPLOAD_USER_CHUNK_SIZE/4,checksum);
        memcpy(mPtr, buffer, UPLOAD_USER_CHUNK_SIZE);
        mPtr = mPtr + UPLOAD_USER_CHUNK_SIZE;
      }

      if(checksum != 0){
        printf("ERROR, checksum error %x\n", checksum);
      }
      else {
        uint8_t * memaddr = mh->memptr;
        if((uint32_t)memaddr % 2 != 0)memaddr++;

        struct ProcessDescriptor * pd = loadProcess(memaddr,false,PROC_TYPE_USER);
        transferMemory(mh,pd->pid);
        printf("done\n");
      }
    }
    else {
      printf("ERROR, could not allocate the requested %d bytes\n", binLen + hLen + sLen);
    }
  }
  else {
    printf("ERROR, usage: u_upload <binary length (multiple of %d bytes)> <heap size> <stack size> <checksum>\n", UPLOAD_USER_CHUNK_SIZE);
  }

  return true;
}

bool kmUpload(char * line){
  uint32_t nPages = 0;
  uint32_t myChecksum = 0;
  sscanf(line, "%*s %d %u",&nPages, &myChecksum);
  if(nPages && myChecksum){
    myChecksum = ~myChecksum + 1;
    uint8_t buffer[FLASH_PAGE_SIZE];
    struct PageAllocationStruct pas;
    memset(&pas,0,sizeof(PageAllocationStruct));
    memset(buffer,0,FLASH_PAGE_SIZE);

    for(uint32_t i =1; i < (nPages*4 + 1); i++){
      printf("next\n");

      //block waiting for uart line rdy
      while(getRxBufferLenUart() < (FLASH_PAGE_SIZE/4)){
        asm("");
      }

      uint32_t nBytesRead = readDirectUART(buffer + (FLASH_PAGE_SIZE/4)*((i - 1) % 4), FLASH_PAGE_SIZE/4);

      if(nBytesRead != (FLASH_PAGE_SIZE/4)){
        printf("ERROR, binary data read error\n");
        return true;
      }

      if(i % 4 == 0){
        myChecksum = evalChecksum((uint32_t*)buffer,FLASH_PAGE_SIZE/4,myChecksum);

        if(i == 4){
          //header
          writeBinaryToFlash(&pas,buffer,nPages);
          memset(buffer,0,FLASH_PAGE_SIZE);
          if(pas.startPage == 0){
            printf("ERROR, could not allocate flash!\n");
            return true;
          }
        }
        else {
          //data page
          writeNextPage(&pas,buffer);
          memset(buffer,0,FLASH_PAGE_SIZE);
        }
      }
    }

    if(myChecksum == 0){
      printf("done\n");
    }
    else {
      printf("ERROR, checksum error! %u\n", myChecksum);
    }
  }
  else {
    printf("bad arguments for upload. usage: upload <pageCount> <checksum>\n");
  }

  return true;
}

bool kmDelete(char * line){
  char cmd[128];
  memset(cmd,0,128);
  uint32_t arg = 0;
  sscanf(line, "%*s %s",cmd);
  if(strlen(cmd) == 0){
    printf("bad arguments for delete. usage: delete <[page] | [all]> [page number]\n");
  }
  else if(strcmp(cmd,"all") == 0){
    clearAllocationPage();
    printf("all pages cleared\n");
  }

  return true;
}

bool kmlsFlash(char * line){
  printf("---- programs in flash -----\n");
  uint8_t allocationPage[FLASH_PAGE_SIZE];
  if(readPageAuto(PAGE_ALLOCATION_PAGE, allocationPage)){
    for(uint32_t i = 0; i <= FLASH_LEN/2; i++){
      if(getBit(i,allocationPage)){
        struct FlashHeader fh;
        parseFlashHeader(i+FLASH_LEN/2,&fh);
        printf("------------------\n");
        printf("name: %.60s\n", fh.modName);
        printf("id:   %.8x\n", fh.id);
        printf("text start: %.8x\n", fh.text_start);
        printf("text end:   %.8x\n", fh.text_end);
        printf("got start:  %.8x\n", fh.got_start);
        printf("got end:    %.8x\n", fh.got_end);
        printf("data start: %.8x\n", fh.data_start);
        printf("data end:   %.8x\n", fh.data_end);
        printf("bss start:  %.8x\n", fh.bss_start);
        printf("bss end:    %.8x\n", fh.bss_end);
        printf("requested heap size: %.8x\n", fh.reqHeapSize);
        printf("jump table start: %p\n", fh.jumpTableStart);
        printf("------------------\n");
      }
    }
  }
  else {
    printf("ERROR reading allocation page");
  }

  printf("###############################\n");
  return true;
}
