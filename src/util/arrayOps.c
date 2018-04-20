#include "arrayOps.h"
#include <memory.h>
#include <stdio.h>

void deletFromArray(uint8_t * array, uint32_t removeAt, uint32_t lenRemove, uint32_t lenArray, uint32_t elSize){
  if(removeAt + 1 != lenArray && lenRemove != 0){
    memcpy(array + removeAt*elSize, array + elSize*(removeAt + lenRemove), elSize*(lenArray - (removeAt + lenRemove)));
  }
  else {
    //remove at end of array is nop.
  }
}


uint8_t getBit(uint32_t pos,uint8_t * buffer){
  uint32_t bytes = (pos) / 8;
  return buffer[bytes] >> -(pos - (bytes*8 + 7));
}

void setBit(uint32_t bitPos, uint8_t value, uint8_t * buffer){
  uint32_t bytes = bitPos / 8;
  buffer[bytes] = (buffer[bytes] & ~(1 << -(bitPos - (bytes*8 + 7)))) | ((0x1 & value) << -(bitPos - (bytes*8 + 7)));
}
