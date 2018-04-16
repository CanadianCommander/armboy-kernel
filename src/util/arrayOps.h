/**
@file
@brief handy array operations
*/
#ifndef arrayOps_H_
#define arrayOps_H_
#include <stdint.h>

/**
  delete a section from an array and fill it in.
  @param array target array of delete operation
  @param removeAt index in to array at which to start remove
  @param lenRemove the number of elements to remove
  @param lenArray the length of the target array
  @param elSize size of a single element  
*/
void deletFromArray(uint8_t * array, uint32_t removeAt, uint32_t lenRemove, uint32_t lenArray, uint32_t elSize);

#endif /*arrayOps_H_*/
