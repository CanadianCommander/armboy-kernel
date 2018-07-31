/**
@file
@brief system call dispatcher
*/
#ifndef SYSCALL_DISPATCHER_H_
#define SYSCALL_DISPATCHER_H_
#include <stdint.h>


//********* system call codes
#define SYS_CALL_YIELD 0x0 // yield caller
#define SYS_CALL_ABORT 0x1 // kill caller
#define SYS_CALL_BLOCK 0x2 // block caller
#define SYS_CALL_MOD   0x3 // call kernel module function
#define SYS_CALL_LOAD_MOD 0x4
// ***************************


/**
  call kernel module function at jVec on module defined by cid and pass arg as an argument
  @param cid the module id
  @param jVec offset in to vector table. i.e. 0 == first method,  1 == second ... ect
  @param arg kernel module argument most likely a pointer
*/
void doModuleCall(uint32_t cid, uint16_t jVec, uint32_t arg);

#endif /*SYSCALL_DISPATCHER_H_*/
