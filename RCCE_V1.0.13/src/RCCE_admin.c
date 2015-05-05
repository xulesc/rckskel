//***************************************************************************************
// Administrative routines. 
//***************************************************************************************
//
// Author: Rob F. Van der Wijngaart
//         Intel Corporation
// Date:   008/30/2010
//
//***************************************************************************************
//
// 
// Copyright 2010 Intel Corporation
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// 

#include "RCCE_lib.h"
#ifdef RC_POWER_MANAGEMENT
  #include "RCCE_lib_pwr.h"
#endif

#ifdef COPPERRIDGE
  #include <sys/mman.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include "SCC_API.h"
#endif

// En-/ or disable debug prints...
#define DEBUG 0

//......................................................................................
// GLOBAL VARIABLES USED BY THE LIBRARY
//......................................................................................
int       RCCE_NP;               // number of participating cores
double    RC_REFCLOCKGHZ;        // baseline CPU frequency (GHz)
int       RC_MY_COREID;          // physical ID of calling core
int       RC_COREID[RCCE_MAXNP]; // array of physical core IDs for all participating 
                                 // cores, sorted by rank
int       RCCE_IAM=-1;           // rank of calling core (invalid by default)
RCCE_COMM RCCE_COMM_WORLD;       // predefined global communicator
int       RCCE_BUFF_SIZE;        // available MPB size
t_vcharp  RCCE_comm_buffer[RCCE_MAXNP]; // starts of MPB, sorted by rank
#ifndef GORY
  // ......................... non-GORY communication mode .............................
  // synchronization flags are predefined and maintained by the library
  RCCE_FLAG RCCE_sent_flag[RCCE_MAXNP], RCCE_ready_flag[RCCE_MAXNP];
  // payload part of the MPBs starts at a specific address, not malloced space
  t_vcharp RCCE_buff_ptr;
  // maximum chunk size of message payload is also specified
  size_t RCCE_chunk;
  // synchronization flags will be allocated at this address
  t_vcharp  RCCE_flags_start;
#endif

#ifdef COPPERRIDGE
  // virtual addresses of test&set registers
  t_vcharp virtual_lockaddress[RCCE_MAXNP];
#endif
//......................................................................................
// END GLOBAL VARIABLES USED BY THE LIBRARY
//......................................................................................

#ifdef COPPERRIDGE
#ifndef __INTEL_COMPILER
    inline volatile long long _rdtsc() {
      register long long TSC __asm__("eax");
      __asm__ volatile (".byte 15, 49" : : : "eax", "edx");
      return TSC;
    }
#endif
#endif

//--------------------------------------------------------------------------------------
// FUNCTION: RC_cache_invalidate
//--------------------------------------------------------------------------------------
// invalidate (not flush!) lines in L1 that map to MPB lines
//--------------------------------------------------------------------------------------
void RC_cache_invalidate() {
#ifdef COPPERRIDGE
  __asm__ volatile ( ".byte 0x0f; .byte 0x0a;\n" ); // CL1FLUSHMB
#endif
  return;
}

//--------------------------------------------------------------------------------------
// FUNCTION: RC_COMM_BUFFER_SIZE
//--------------------------------------------------------------------------------------
// return total available MPB size on chip
//--------------------------------------------------------------------------------------
int RC_COMM_BUFFER_SIZE() {
  return RCCE_BUFF_SIZE_MAX*RCCE_MAXNP;
}

//--------------------------------------------------------------------------------------
// FUNCTION: RC_COMM_BUFFER_START
//--------------------------------------------------------------------------------------
// return (virtual) start address of MPB for UE with rank ue
//--------------------------------------------------------------------------------------
t_vcharp RC_COMM_BUFFER_START(int ue){
#ifdef COPPERRIDGE
  // "Allocate" MPB, using memory mapping of physical addresses
  t_vcharp retval;
  MPBalloc(&retval, X_PID(RC_COREID[ue]), Y_PID(RC_COREID[ue]), Z_PID(RC_COREID[ue]), 
           (X_PID(RC_COREID[ue]) == X_PID(RC_COREID[RCCE_IAM])) && 
           (Y_PID(RC_COREID[ue]) == Y_PID(RC_COREID[RCCE_IAM]))
          );
  return retval;
#else
  // even in functional emulation mode we leave gaps in the global MPB
  return RC_comm_buffer + RC_COREID[ue]*RC_COMM_BUFFER_SIZE()/RCCE_MAXNP;
#endif
}

//--------------------------------------------------------------------------------------
// FUNCTION: RC_SHM_BUFFER_START
//--------------------------------------------------------------------------------------
// return (virtual) start address of off-chip shared memory
//--------------------------------------------------------------------------------------
t_vcharp RC_SHM_BUFFER_START(){
#ifdef COPPERRIDGE
  t_vcharp retval;
  SHMalloc(&retval);
  return retval;
#else
  return RC_shm_buffer;
#endif
}

//--------------------------------------------------------------------------------------
// FUNCTION: MYCOREID
//--------------------------------------------------------------------------------------
// return physical core ID of calling core
//--------------------------------------------------------------------------------------
int MYCOREID() {
#ifdef COPPERRIDGE
  int tmp, x, y, z;
  tmp=ReadConfigReg(CRB_OWN+MYTILEID);
  x=(tmp>>3) & 0x0f; // bits 06:03
  y=(tmp>>7) & 0x0f; // bits 10:07
  z=(tmp   ) & 0x07; // bits 02:00
  return ( ( x + ( 6 * y ) ) * 2 ) + z; // True Processor ID!
#else
  // the COREIDs are read into the main program in potentially random order.
  // Each core can access its own Core ID. We simulate that by selecting
  // the value in the list of coreids that corresponds to the sequence
  // number of the OpenMP thread number                                  
  return RC_COREID[omp_get_thread_num()];
#endif
}

//--------------------------------------------------------------------------------------
// FUNCTION: RCCE_acquire_lock
//--------------------------------------------------------------------------------------
// acquire lock corresponding to core with rank ID
//--------------------------------------------------------------------------------------
int RCCE_acquire_lock(int ID) {

#ifdef COPPERRIDGE
  // semantics of test&set register: a read returns zero if another core has
  // previously read it and no reset has occurred since then. Otherwise, the read
  // returns one. Comparing (hex) one with the contents of the register forces a
  // read. As long as the comparison fails, we keep reading.
  while (!((*(virtual_lockaddress[ID])) & 0x01));
#else
  omp_set_lock(&(RCCE_corelock[ID]));
#endif
  return(RCCE_SUCCESS);
}

//--------------------------------------------------------------------------------------
// FUNCTION: RCCE_release_lock
//--------------------------------------------------------------------------------------
// release lock corresponding to core with rank ID
//--------------------------------------------------------------------------------------
int RCCE_release_lock(int ID) {
#ifdef COPPERRIDGE
  // semantics of test&set register: a write by _any_ core causes a reset
  *(virtual_lockaddress[ID]) = 0x0;
#else
  omp_unset_lock(&(RCCE_corelock[ID]));
#endif
  return RCCE_SUCCESS;
}

//--------------------------------------------------------------------------------------
// FUNCTION: RC_FREQUENCY
//--------------------------------------------------------------------------------------
// return actual core clock frequency (Hz)
//--------------------------------------------------------------------------------------
long long RC_FREQUENCY() {
return (long long)(RC_REFCLOCKGHZ*1.e9);
}

//--------------------------------------------------------------------------------------
// FUNCTION: RCCE_init
//--------------------------------------------------------------------------------------
// initialize the library and sanitize parameter list
//--------------------------------------------------------------------------------------
int RCCE_init(
  int *argc,   // pointer to argc, passed in from main program
  char ***argv // pointer to argv, passed in from main program
  ) {

  int i, ue;
#ifdef COPPERRIDGE
  int x, y, z;
  unsigned int physical_lockaddress;
#endif
  void *nothing = NULL;
  
#ifdef COPPERRIDGE
  // Copperridge specific initialization...
  InitAPI(0);fflush(0);
#endif  

  // save pointer to executable name for later insertion into the argument list
  char *executable_name = (*argv)[0];

  RCCE_NP        = atoi(*(++(*argv)));  
  RC_REFCLOCKGHZ = atof(*(++(*argv)));  

  // put the participating core ids (unsorted) into an array             
  for (ue=0; ue<RCCE_NP; ue++) {
    RC_COREID[ue] = atoi(*(++(*argv)));
  }

#ifndef COPPERRIDGE
  // if using the functional emulator, must make sure to have read all command line 
  // parameters up to now before overwriting (shifted) first one with executable
  // name; even though argv is made firstprivate, that applies only the pointer to 
  // the arguments, not the actual data
  #pragma omp barrier
#endif
  // make sure executable name is as expected                 
  (*argv)[0] = executable_name;

  RC_MY_COREID = MYCOREID();

  // adjust apparent number of command line arguments, so it will appear to main 
  // program that number of UEs, clock frequency, and core ID list were not on
  // command line        
  *argc -= RCCE_NP+2;

  // sort array of participating phyical core IDs to determine their ranks
  RCCE_qsort((char *)RC_COREID, RCCE_NP, sizeof(int), id_compare);

  // determine rank of calling core
  for (ue=0; ue<RCCE_NP; ue++) {
    if (RC_COREID[ue] == RC_MY_COREID) RCCE_IAM = ue;
  }

  // leave in one reassuring debug print
  if (DEBUG)
  printf("My rank is %d, physical core ID is %d\n", RCCE_IAM, RC_MY_COREID);

  if (RCCE_IAM<0)
    return(RCCE_error_return(RCCE_debug_comm,RCCE_ERROR_CORE_NOT_IN_HOSTFILE));

#ifdef COPPERRIDGE
  // compute and memory map addresses of test&set registers for all participating cores 
  for (ue=0; ue<RCCE_NP; ue++) { 
    z = Z_PID(RC_COREID[ue]);
    x = X_PID(RC_COREID[ue]);
    y = Y_PID(RC_COREID[ue]);
    physical_lockaddress = CRB_ADDR(x,y) + (z==0 ? LOCK0 : LOCK1);
    virtual_lockaddress[ue] = (t_vcharp) MallocConfigReg(physical_lockaddress);
  }
#endif

  // initialize MPB starting addresses for all participating cores
  for (ue=0; ue<RCCE_NP; ue++) 
    RCCE_comm_buffer[ue] = RC_COMM_BUFFER_START(ue);

  // gross MPB size is set equal to maximum
  RCCE_BUFF_SIZE = RCCE_BUFF_SIZE_MAX;

#ifdef RC_POWER_MANAGEMENT
#ifndef COPPERRIDGE
  // always store RPC queue data structure at beginning of MPB, so allocatable
  // storage needs to skip it. Only need to do this for functional emulator
  for (ue=0; ue<RCCE_NP; ue++) 
    RCCE_comm_buffer[ue] += REGULATOR_LENGTH;
  RCCE_BUFF_SIZE -= REGULATOR_LENGTH;
#endif
#endif
  
  // initialize RCCE_malloc
  RCCE_malloc_init(RCCE_comm_buffer[RCCE_IAM],RCCE_BUFF_SIZE);
  RCCE_shmalloc_init(RC_SHM_BUFFER_START(),RCCE_SHM_SIZE_MAX);

  // create global communicator (equivalent of MPI_COMM_WORLD); this will also allocate 
  // the two synchronization flags associated with the global barrier 
  RCCE_comm_split(RCCE_global_color, nothing, &RCCE_COMM_WORLD);

  // if power management is enabled, initialize more stuff; this includes two more 
  // communicators (for voltage and frequency domains), plus two synchronization flags
  // associated with the barrier for each communicator       
#ifdef RC_POWER_MANAGEMENT
  int error;
  if (error=RCCE_init_RPC(RC_COREID, RCCE_IAM, RCCE_NP)) 
       return(RCCE_error_return(RCCE_debug_RPC,error));
#endif

#ifndef GORY
  // if we use the simplified API, we need to define more flags upfront  
  for (ue=0; ue<RCCE_NP; ue++) {
    RCCE_flag_alloc(&RCCE_sent_flag[ue]);
    RCCE_flag_alloc(&RCCE_ready_flag[ue]);
  }
#endif

  return (RCCE_SUCCESS);
}

//--------------------------------------------------------------------------------------
// FUNCTION:  RCCE_finalize
//--------------------------------------------------------------------------------------
// clean up at end of library usage (memory unmapping) and resetting of memory and
// registers
//--------------------------------------------------------------------------------------
int RCCE_finalize(void){


#ifdef COPPERRIDGE
  int ue, iword;
  RCCE_barrier(&RCCE_COMM_WORLD);
  // each UE clears its own MPB and test&set register
  //ERROR: THIS IS NOT THE START OF THE COMM BUFFER, BUT OF THE PAYLOAD AREA!!
//  for (iword=0; iword<(RCCE_BUFF_SIZE_MAX)/sizeof(int); iword++)
//      ((int *)(RCCE_comm_buffer[ue]))[iword] = 0;
//    MPBunalloc(&(RCCE_comm_buffer[ue]));
  RCCE_release_lock(RCCE_IAM);
  // each core needs to unmap all special memory locations
  for (ue=0; ue<RCCE_NP; ue++) { 
    FreeConfigReg((int *)(virtual_lockaddress[ue]));
  }
  fflush(NULL);
#endif
  return (RCCE_SUCCESS);
}

//--------------------------------------------------------------------------------------
// FUNCTION:  RCCE_wtime
//--------------------------------------------------------------------------------------
// clean up at end of library usage (memory unmapping)
//--------------------------------------------------------------------------------------
double RCCE_wtime(void) {
#ifdef COPPERRIDGE
  return ( ((double)_rdtsc())/(RC_REFCLOCKGHZ*1.e9));
#else
  return (omp_get_wtime());
#endif
}

//--------------------------------------------------------------------------------------
// FUNCTION:  RCCE_ue
//--------------------------------------------------------------------------------------
// return rank of calling core
//--------------------------------------------------------------------------------------
int RCCE_ue(void) {return(RCCE_IAM);}

//--------------------------------------------------------------------------------------
// FUNCTION: RCCE_num_ues
//--------------------------------------------------------------------------------------
// return total number of participating UEs              
//--------------------------------------------------------------------------------------
int RCCE_num_ues(void) {return(RCCE_NP);}
