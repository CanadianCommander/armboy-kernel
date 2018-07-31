/**
@file
@brief handy array operations
*/
#ifndef arrayOps_H_
#define arrayOps_H_
#include <stdint.h>

/**
  delete a section from an array and collapse other sections in such that there is no gap left
  by the deletion 
  @param array target array of delete operation
  @param removeAt index in to array at which to start remove
  @param lenRemove the number of elements to remove
  @param lenArray the length of the target array
  @param elSize size of a single element
*/
void deletFromArray(uint8_t * array, uint32_t removeAt, uint32_t lenRemove, uint32_t lenArray, uint32_t elSize);

/**
  walk through a buffer one bit at a time!
  @param pos the current position in bits in to the buffer (incremented on every call)
  @param buffer that you want to walk
  @return the next bit
*/
uint8_t getBit(uint32_t pos,uint8_t * buffer);

/**
  set the bitPos'th bit of the buffer
  @param bitPos the bit you whish to set (counting up from zero)
  @param value to set (1 or 0 ofc)
  @param the buffer to set the bit in
*/
void setBit(uint32_t bitPos, uint8_t value, uint8_t * buffer);

/**
  checks the given checksum against the given array and returns the result. zero means checksum match.
  @param array the array of data to check. must be word aligned
  @param len the length of the array in words (4 bytes)
  @param checksum the checksum for the data
  @return array checksum
*/
uint32_t evalChecksum(uint32_t * array, uint32_t len, uint32_t checksum);

#endif /*arrayOps_H_*/
