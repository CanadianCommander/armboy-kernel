#include "debug.h"
#include "../config.h"
#include "../hardware/hardware.h"

#include <stdio.h>
#include <memory.h>

void printSysInfo(void){
  //read stack pointer
  register int stackPtr asm("sp");

  //read flash descriptor
  struct FlashDescriptor fd;
  uint32_t planeBuf[25];
  uint32_t lockRBuf[25];
  fd.bytesInPlane = (uint32_t *)&planeBuf;
  fd.bytesPerLockRegion = (uint32_t *)&lockRBuf;

  printf("========== DEBUG INFO ============\n");
  printf("Stack Pointer: 0x%x\n", stackPtr);

  //print information about the flash memory
  for(int i =0;i < FLASH_CONTROLLER_COUNT; i++){
    readFlashDescriptor(&fd,0,25,25);

    printf("=== Flash Descriptor %d \n", i);
    printf("Flash ID: %d\nFlash Size: %d\nPage Size: %d\nPlane Count: %d\nLock Region Count: %d\n",
            fd.flashID, fd.flashSize, fd.pageSize, fd.numberOfPlanes, fd.numberOfLockBits);

    //clear flash descriptor
    memset(&fd, 0, sizeof(FlashDescriptor));
    memset(&planeBuf, 0, sizeof(uint32_t)*25);
    memset(&lockRBuf, 0, sizeof(uint32_t)*25);
    fd.bytesInPlane = (uint32_t *)&planeBuf;
    fd.bytesPerLockRegion = (uint32_t *)&lockRBuf;
  }


  printf("END ------ DEBUG INFO ------- END\n");
}
