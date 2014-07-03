/**
 * Functions described at:
 * - http://en.wikipedia.org/wiki/Algorithmic_skeleton
 * - "A Survey of Algorithmic Skeleton Frameworks: High-Level
 *    Structured Parallel Programming Enablers", Velez, G. H.
 *    and Leyton, M., Software Practice and Experience, 2010. 
 */

#include "RCCE.h"
#include "rckskel_datatypes.h"
#include <stdarg.h>

#ifndef _RCKSKEL_

#define _RCKSKEL_
#define _RCKSKEL_VERBOSE_

#define MASTER_ID 0

#define DIE = -1;
#define IS_READY = 1;
#define RCKSKEL_FALSE 0
#define RCKSKEL_TRUE 1
#define RCKSKEL_UE_UNASSIGNED 0
#define RCKSKEL_UE_PROCESSING 1
#define RCKSKEL_UE_COMPLETED 3

// RCKSKEL util functions
void send_ue_ready(rckskel_env_t *env);
void check_ues_up(int *ue_count);
int ue_die();
void terminate_ues(int *ue_count);

// RCKSKEL task processing methods

/**
 The functions accepts the root node of a task tree leaf nodes of which are
 expected to contain jobs that must be processed. The ue_count and ue_ids
 reflect all compute nodes that may be used during the processing. It is
 left to the user of the API to ensure that these correspond to the ones
 used by the jobs specified in the tree.
 */
void rckskel_process_task_tree(task_t *root, int ue_count, int *ue_ids);
/**
 The function prepares the data structures used by the tree processing
 function. The root node of the task tree created is defined by the parameters
 of the function.
 */
void rckskel_prepare_process(skeltag_t skel, int ue_count, int *ue_ids,
		int (*check_ready)(int), int task_count, va_list *ap);

// Data Parallel

/**
 * MAP, semantics specify that a function or a sub-skeleton can be 
 * applied simultaneously to all the elements of a list to achieve
 * parallelism. The data parallelism occurs because a single data 
 * element can be split into multiple data, then the sub-skeleton 
 * is executed on each data element. The map skeleton can be 
 * conceived as single instruction, multiple data parallelism.
 */
void MAP(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...);

/**
 * FORK behaves like map. The difference is that instead of applying 
 * the same function to all elements of a list, a different one is 
 * applied to each element. Thus fork can be construed as multiple 
 * instruction, multiple data parallelism.
 */

/**
 * REDUCE, also known as scan, is employed to compute prefix operations 
 * in a list by traversing the list from left to right and then applying 
 * a function to each pair of elements, typically summation. Note that 
 * as opposed to map, it maintains aggregated partial results.
 */

/**
 * COLLECT, is employed to collect the results from specified compute
 * elements. The return data from each element is assumed to be the same.
 */
void COLLECT(int ue_count, int *ue_ids, int (*collector)(int));

// Task Parallel

/**
 * SEQ does not represent parallelism, but it is often used as a convenient 
 * tool to wrap code at the leafs of the skeleton nesting.
 */
void SEQ(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...);

/**
 * FARM is also known as master-slave. Farm represents task replication
 * where the execution of different tasks by the same farm are replicated 
 * and executed in parallel.
 */
void FARM(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...);

/**
 * PIPE represents staged computation. The implementation supports nested 
 * patterns to be passed however no parallelism is supported between the
 * stages of the pipe. Essentially the pipe works exactly like SEQ. It is
 * upto the user to ensure the data passing between stages is correctly
 * handled in the worker and collector functions.
 */
void PIPE(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...);

/**
 * FOR represents fixed iteration, where a task is executed a fixed number
 * of times. In some implementations the executions may take place in 
 * parallel in this implementation it is serial.
 */
void FOR(int iteration_count, int ue_count, int *ue_ids, task_t *task);

/**
 * WHILE represents conditional iteration, where a given skeleton is 
 * executed until a condition is met.
 */
void WHILE(int (*condition)(void), int ue_count, int *ue_ids, task_t *task);

/**
 * IF represents conditional branching, where the execution choice between 
 * two skeleton patterns is decided by a condition.
 */
void IF(int (*condition)(void), int ue_count, int *ue_ids, task_t *task1,
		task_t *task2);

// Resolution

/**
 * D&C represents divide and conquer parallelism. A task is recursively 
 * sub-divided until a condition is met, then the sub-task is executed and 
 * results are merged while the recursion is unwound.
 */

/**
 * Branch & Bound (B&B) is a resolution skeleton which also features a 
 * generic use of a map skeleton. b&b divides recursively the search space 
 * (branch) and then determines the elements in the resulting sub-spaces 
 * by mapping an objective function (bound).
 */

#endif

