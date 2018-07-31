/**
@file
@brief functions for managing dynamic loading and unloading of processes 
*/

#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_PROCESS 64

/* PROCESS TYPES */
#define PROC_TYPE_USER 0
#define PROC_TYPE_KMOD 1
//#################

/* SPECIAL PID VALUES */
#define BAD_PID    0
#define KERNEL_PID 1
//######################

/* PROCESS STATES */
#define PROCS_DEAD    0
#define PROCS_READY   1
#define PROCS_RUNNING 2
#define PROCS_BLOCK   3
//##################

typedef void (*JumpVectorPtr) (void *);

struct ProcessDescriptor{
  pid_t pid;
  uint32_t cid;//custom id
  uint8_t proc_type;
  uint8_t proc_state;
  uint16_t block_code;
  void * staticBase;
  void * jumpTableStart;
  void * binaryAddress;
  void * stackPtr;
} ProcessDescriptor;

volatile struct ProcessDescriptor * currentPd;

/**
  returns the process descriptor with the given pid.
  @param pid the process id to look for
  @return a ptr to the process descriptor or NULL if process with pid is not found.
*/
struct ProcessDescriptor * findProcessDescriptor(pid_t pid);

/**
  like findProcessDescriptor but uses cid instead of pid.
  @see findProcessDescriptor
*/
struct ProcessDescriptor * findProcessDescriptorCid(uint32_t cid);

/**
  "loades" a process from the given memory address. for a simple PC relative binary in SRAM this could
  be as simple as filling in the ProcessDescriptor structure. On the other hand if flash is true then
  it will involve memory allocation + code relocation
  @param binaryStartPtr a pointer to the first byte of the binary you wish to load
  @param flash set true if binary is in flash memory
  @param proc type the type of process being loaded
  @return new process descriptor or NULL on error
*/
struct ProcessDescriptor * loadProcess(void * binaryStartPtr,bool flash,uint8_t proc_type);

/**
  like loadProcess but loads the kernel module with id "mid".
  @return process handle or NULL on failure
*/
struct ProcessDescriptor * loadKernelModule(uint16_t mid);

/**
  unload the process described by process descriptor pd
  @param pd the descriptor of the process to be unloaded
  @return true if unloaded false otherwise
*/
bool unloadProcess(struct ProcessDescriptor * pd);

/**
  unload the prcoess with pid (free all resources).
  @param pid the process id of the process to unload
  @return true if module unloaded false if not.
*/
bool unloadProcessPid(pid_t pid);

/**
  like unloadProcess but is based on cid instead of pid.
  @see unloadProcessPid
*/
bool unloadProcessCid(uint32_t cid);

/**
  return the pid of the next ready process in a RR fashion. NOTE only PROC_TYPE_USER
  process can be returned here.
*/
struct ProcessDescriptor * getNextReadyProcess();

/**
  runs the process described by pd. this function blocks until
  process exits / yields / or systick expires.... i.e. this is a context switch
*/
void runProcess(volatile struct ProcessDescriptor * pd);

/**
  add flash manager kernel monitor functions
*/
void addProgManagerKernelMonitorFunctions(void);

#endif /*PROCESS_MANAGER_H_*/
