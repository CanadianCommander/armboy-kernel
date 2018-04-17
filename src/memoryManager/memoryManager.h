#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_
#include <stdio.h>

#define MEMORY_CHUNK_MAX 128

struct MemoryHandle {
  pid_t owner;
  uint8_t * memptr;
  uint32_t len;
} MemoryHandle;


/**
  request memory from the system.
  @param size the size in bytes of the memory region.
  @param owner the owner of the new memory region
  @return memory handle structure or null if memory could not be assigned.
*/
struct MemoryHandle * requestMemory(uint32_t size, pid_t owner);

/**
  release the memory pointed to by the memory handle
  @param memH memory to free
*/
void releaseMemory(struct MemoryHandle * memH);

/**
  add memory dump kernel monitor function.
  "memdump"
*/
void addMemoryDebugKernelMonitor();

#endif /*MEMORY_MANAGER_H_*/
