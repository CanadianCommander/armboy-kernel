/**
@file
@brief SRAM memory managment
*/
#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_
#include <stdio.h>

#define MEMORY_CHUNK_MAX 128

struct MemoryHandle {
  pid_t owner;
  uint8_t * memptr;
  uint32_t heapStart;
  uint32_t len;
} MemoryHandle;


// heap data fields (of the 4 byte block header)
#define HEAP_HEADER_SIZE 4
#define BLOCK_ALLOCATED  (1 << 0)
#define BLOCK_LAST_ALLOC (1 << 1)
#define HEAP_BASE        (1 << 2)
#define HEAP_END         (1 << 3)
#define BLOCK_LEN(header)(header >> 8)
#define SET_BLOCK_LEN(len)((0xffffff & (len)) << 8)
#define BLOCK_HEADER(len, alloc, lalloc, base, end) ((0xffffff & (len)) << 8) | ((0x1 & (alloc)) << 0) | ((0x1 & (lalloc)) << 1) | ((0x1 & (base)) << 2) | ((0x1 & (end)) << 3)

/**
  request memory from the system.
  @param size the size in bytes of the memory region.
  @param offset at which the heap starts, set to zero for no heap.
  @param owner the owner of the new memory region
  @return memory handle structure or null if memory could not be assigned.
*/
struct MemoryHandle * requestMemory(uint32_t size, uint32_t heapStart, pid_t owner);

/**
  release the memory pointed to by the memory handle
  @param memH memory to free
*/
void releaseMemory(struct MemoryHandle * memH);

/**
  force allocate memory region defined by startAddr and size to the kernel.
  VERY UNSAFE ONLY USE AT START UP.
  @param startAddr the start address of the kernel region
  @param size the size of the region
*/
void allocateKernelMemory(uint8_t * startAddr, uint32_t size);

/**
  get memory regions owned by target process
  @param owner the process for which memory regions are to be fetched
  @param iter location at which to store the iterator. call this function multiple times
         to retrieve multiple regions. iter should be initialized to zero.
  @return memor region or NULL if process owns no regions.
*/
struct MemoryHandle * getAllocatedMemory(pid_t owner, uint32_t * iter);

/**
  initalize a region of memory for heap managment
  @param memptr address of the memory region to intialize
  @param len the length of the memory region to initalize
*/
void initializeMemoryRegion(uint8_t * memptr, uint32_t len);

/**
  allocate memory for the current process.
  works just like malloc
*/
void * malloc(size_t size);
/**
  allocate memory for the specified process.
  works just like malloc
*/
void * malloc_pid(size_t size, pid_t pid);

/**
  like normal free
*/
void free(void * ptr);

/**
  add memory dump kernel monitor function.
  "memdump"
*/
void addMemoryDebugKernelMonitor();

#endif /*MEMORY_MANAGER_H_*/
