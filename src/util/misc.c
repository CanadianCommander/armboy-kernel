#include "misc.h"

void * alignPtr(void * ptr, uint16_t align){
  return ptr - ((uint32_t)ptr % align);
}

Pin toPin(uint8_t bank, uint32_t pin){
  Pin p;
  p.bank = bank;
  p.pin = pin;
  return p;
}
