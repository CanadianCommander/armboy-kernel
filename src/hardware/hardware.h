#ifndef HARDWARE_H_
#define HARDWARE_H_

#ifdef MCU_SAM3X8E
//sam3x8e includes
#include "sam3x8eHardwareAPI/src/EEFC/eefc.h"
#include "sam3x8eHardwareAPI/src/UART/uart.h"
#include "sam3x8eHardwareAPI/src/Timers/timers.h"
#include "sam3x8eHardwareAPI/src/general/general.h"

#else
#error no MCU defined
#endif

#endif /*HARDWARE_H_*/
