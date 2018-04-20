/**
@file
@brief Flash memory manager
*/
#ifndef FLASH_MANAGER_H_
#define FLASH_MANAGER_H_
#include <stdint.h>

/**
  the PAGE_ALLOCATION_PAGE is a flash page (FLASH_PAGE_SIZE bytes). it contains bit field page indicating what pages are allocated
*/
#define PAGE_ALLOCATION_PAGE 1025

struct PageAllocationStruct {
  uint32_t startPage;
  uint32_t nxtPage;
  uint32_t pageCount;
} PageAllocationStruct;

/**
  start the process of writing a binary to flash.
  @param pas a empty structure used to keep track of the flash write operation
  @param headerPage a buffer of page size (FLASH_PAGE_SIZE bytes) containing header data.
  @param countPages the total number of pages in the binary (including header page)
  @return pas will be populated and must be passed when calling writeNextPage
  @see writeNextPage
*/
void writeBinaryToFlash(struct PageAllocationStruct * pas, uint8_t * headerPage,uint32_t countPages);

/**
  write the next page of a binary flash operation started by writeBinaryToFlash.
  @param pas the flash book keeping structure returned from writeBinaryToFlash
  @param page data the next page of data to write
  @see writeBinaryToFlash
*/
void writeNextPage(struct PageAllocationStruct * pas, uint8_t * pageData);

/**
  zero out allocation page. i.e. deallocate all blocks
*/
void clearAllocationPage();

/**
  add flash manager kernel monitor functions
*/
void addFlashKernelMonitorFunctions(void);

#endif /*FLASH_MANAGER_H_*/
