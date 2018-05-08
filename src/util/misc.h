#ifndef MISC_H_
#define MISC_H_
#include <stdint.h>
#include "../config.h"
#include "../hardware/hardware.h"

/**
  adjust ptr DOWN! in order to align it.
*/
void * alignPtr(void * ptr, uint16_t align);


#endif /*MISC_H_*/
