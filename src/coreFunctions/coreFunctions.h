/**
@file
@brief core functionality provided by the kernel for use by user mode applications
*/
#ifndef CORE_FUNCTIONS_H_
#define CORE_FUNCTIONS_H_
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/** function ids **/
#define CF_RANDOM     0
#define CF_SLEEP      1
#define CF_IS_LOADED  2

void doCoreFunction(uint16_t functionId, void * arg);


// fill memory pointed to by rnd with 32bit random number
void getRandom(void * rnd);

// sleep for the amount of milliseconds pointed to by ms
void doSleep(void * ms);

// returns true if a process with custom id cid is running.
// Ex: isLoaded(1) checks if the display driver is loaded
bool isLoaded(uint32_t cid);

#endif /* CORE_FUNCTIONS_H_ */
