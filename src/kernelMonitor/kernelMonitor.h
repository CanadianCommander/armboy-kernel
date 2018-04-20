/**
@file
@brief serial monitor interface for the kernel over programming port
*/
#ifndef kernelMonitor_H_
#define kernelMonitor_H_

#include <stdbool.h>
#include <stdint.h>

#define KERNEL_MONITOR_LINE_BUFFER 256
#define KERNEL_MONITOR_HANDLER_MAX 256

/**
  monitoring function. this function is to examin the passed input line. If
  this function decides to take action on said input then it should return true
  to stop further handlers from being invoked.
  @param input line
  @return true if input line handled, false if not
*/
typedef bool (* kMonitorHandler) (char *);

/**
  load some default / standard kernel monitor handler functions
*/
void loadDefaultMonitorHandlers(void);

/**
  is there pending kernel monitor operations
  @return true if pending kernel monitor operations
*/
bool hasPending();

/**
  service pending kernel monitor operations. most likely
  read user input do some thing - n produce output.
*/
void servicePendingOperations();

/**
  add kernel monitor handler
  @param mon handler to add
*/
void addMonitor(kMonitorHandler mon);

/**
  like addMonitor but the mon function is wraped in a name checking function.
  that is, the user must type: <name> bla bla bla to invoke mon.
  @param mon handler to add
  @param name the name the user must type to invoke the command
*/
void addMonitorWName(kMonitorHandler mon, char * name);

/**
  remove kernel monitor handler
  @param address of handler to remove
*/
void removeMonitor(kMonitorHandler mon);

#endif /*kernelMonitor_H_*/
