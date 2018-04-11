/**
@file
@brief debuging functions for the kernel
*/
#ifndef UTIL_DEBUG_H_
#define UTIL_DEBUG_H_

#include <stdint.h>

/**
  print system information to std out (UART)
*/
void printSysInfo(void);

/**
  dump the hex from hexBuffer to std out
  @param hexBuffer ptr to the data to be dumped
  @param len the number of bytes to dump
*/
void dumpHex(uint8_t * hexBuffer, uint16_t len);

#endif /*UTIL_DEBUG_H_*/
