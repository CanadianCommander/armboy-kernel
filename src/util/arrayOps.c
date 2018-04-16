#include "arrayOps.h"
#include <memory.h>

void deletFromArray(uint8_t * array, uint32_t removeAt, uint32_t lenRemove, uint32_t lenArray, uint32_t elSize){
  if(removeAt + 1 != lenArray && lenRemove != 0){
    memcpy(array + removeAt*elSize, array + elSize*(removeAt + lenRemove), elSize*(lenArray - (removeAt + lenRemove)));
  }
  else {
    //remove at end of array is nop.
  }
}
