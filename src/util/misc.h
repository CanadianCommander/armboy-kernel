#ifndef MISC_H_
#define MISC_H_
#include <stdint.h>
#include "../config.h"
#include "../hardware/hardware.h"

/**
  adjust ptr DOWN! in order to align it.
*/
void * alignPtr(void * ptr, uint16_t align);


typedef struct {
  uint8_t bank;
  uint32_t pin;
} Pin;

#endif /*MISC_H_*/
