/**
@file
@brief core functionality provided by the kernel for use by user mode applications
*/
#ifndef CORE_FUNCTIONS_H_
#define CORE_FUNCTIONS_H_

/** function ids **/
#define CF_RANDOM     0
#define CF_SLEEP      1

void doCoreFunction(uint16_t functionId, void * arg);


// fill memory pointed to by rnd with 32bit random number
void getRandom(void * rnd);

// sleep for the amount of milliseconds pointed to by ms
void doSleep(void * ms);

#endif /* CORE_FUNCTIONS_H_ */
