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
  uint32_t * setReg;
} Pin;

Pin toPin(uint8_t bank, uint32_t pin, uint32_t * setReg);

#define SET_PIN(p,v) setPin(p.bank,p.pin,v)
#define READ_PIN(p) readPin(p.bank,p.pin)

#endif /*MISC_H_*/
