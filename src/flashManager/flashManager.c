#include "flashManager.h"

#include <string.h>
#include <stdio.h>

#include "../config.h"
#include "../hardware/hardware.h"
#include "../util/arrayOps.h"
#include "../util/debug.h"
#include "../kernelMonitor/kernelMonitor.h"

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

void clearAllocationPage(){
  uint8_t allocationPage[FLASH_PAGE_SIZE];
  memset(allocationPage, 0, FLASH_PAGE_SIZE);
  writePageAuto(PAGE_ALLOCATION_PAGE,allocationPage);
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
          //write changes to flash
          writePageAuto(PAGE_ALLOCATION_PAGE, allocationPage);
          return pos;
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


bool kmUpload(char * line);
bool kmDelete(char * line);
bool kmlsFlash(char * line);
void addFlashKernelMonitorFunctions(void){
  addMonitorWName(kmUpload, "upload");
  addMonitorWName(kmDelete, "delete");
  addMonitorWName(kmlsFlash, "lsf");
}


bool kmUpload(char * line){
  uint32_t nPages = 0;
  sscanf(line, "%*s %d",&nPages);
  if(nPages){
    uint8_t buffer[FLASH_PAGE_SIZE];
    struct PageAllocationStruct pas;
    memset(&pas,0,sizeof(PageAllocationStruct));
    memset(buffer,0,FLASH_PAGE_SIZE);

    for(uint32_t i =1; i < (nPages*4 + 1); i++){
      printf("next\n");

      //block waiting for uart line rdy
      while(!lineReadyUart()){
        asm("");
      }

      getNextLineUart(buffer + (FLASH_PAGE_SIZE/4)*((i - 1) % 4), FLASH_PAGE_SIZE/4);

      if(i % 4 == 0){
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
  }
  else {
    printf("bad arguments for upload. usage: upload <pageCount>\n");
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
  return false;
}
