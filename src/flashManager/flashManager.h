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
#define PAGE_ALLOCATION_PAGE 1024

struct PageAllocationStruct {
  uint32_t startPage;
  uint32_t nxtPage;
  uint32_t pageCount;
} PageAllocationStruct;

/**
  holds information about a program stored in flash. used by loader.
*/
struct FlashHeader {
  char modName[60];
  uint32_t id;

  uint32_t text_start;
  uint32_t text_end;
  uint32_t got_start;
  uint32_t got_end;
  uint32_t data_start;
  uint32_t data_end;
  uint32_t bss_start;
  uint32_t bss_end;

  uint32_t reqHeapSize;

  void * jumpTableStart;
} FlashHeader;

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
  reads the flash header at page, parses it and stores it in header
  @param page the page the contains the header data
  @param header the structure to fill out with the data
*/
void parseFlashHeader(uint32_t page, struct FlashHeader * header);

/**
  return the page number of the module with id, id. zero if not found
*/
uint32_t locateModule(uint32_t id);

/**
  zero out allocation page. i.e. deallocate all blocks
*/
void clearAllocationPage();

/**
  add flash manager kernel monitor functions
*/
void addFlashKernelMonitorFunctions(void);

#endif /*FLASH_MANAGER_H_*/
