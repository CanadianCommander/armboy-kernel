#ifndef SYS_TICK_HANDLER_H_
#define SYS_TICK_HANDLER_H_
#include <stdio.h>
#include <stdbool.h>

volatile bool toUser;

void launchProcess(uint32_t * sp);

#endif /*SYS_TICK_HANDLER_H_*/
