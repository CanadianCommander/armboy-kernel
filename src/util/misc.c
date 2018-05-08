#include "misc.h"

void * alignPtr(void * ptr, uint16_t align){
  return ptr - ((uint32_t)ptr % align);
}
