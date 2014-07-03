#include "rckskel.h"

int __for_index, __for_max;

// local function prototypes
int __for_condition();

//
void SEQ(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...) {
	va_list ap;
	va_start(ap, task_count);
	rckskel_prepare_process(SEQ_rckskel, ue_count, ue_ids, check_ready,
			task_count, &ap);
}

//
void FARM(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...) {
	va_list ap;
	va_start(ap, task_count);
	rckskel_prepare_process(FARM_rckskel, ue_count, ue_ids, check_ready,
			task_count, &ap);
}

//
void WHILE(int (*condition)(void), int ue_count, int *ue_ids, task_t *task) {
	while (condition() == RCKSKEL_TRUE) {
		reset_unfinished_job_counts(task);
		rckskel_process_task_tree(task, ue_count, ue_ids);
	}
}

//
void IF(int (*condition)(void), int ue_count, int *ue_ids, task_t *task1,
		task_t *task2) {
	if (condition() == RCKSKEL_TRUE) {
		rckskel_process_task_tree(task1, ue_count, ue_ids);
	} else {
		rckskel_process_task_tree(task2, ue_count, ue_ids);
	}
}

//
void FOR(int iteration_count, int ue_count, int *ue_ids, task_t *task) {
	__for_index = 0;
	__for_max = iteration_count;
	WHILE(&__for_condition, ue_count, ue_ids, task);
}

//
int __for_condition() {
	return (__for_index++ < __for_max) ? RCKSKEL_TRUE : RCKSKEL_FALSE;
}

// 
void PIPE(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...) {
	va_list ap;
	va_start(ap, task_count);
	rckskel_prepare_process(SEQ_rckskel, ue_count, ue_ids, check_ready,
			task_count, &ap);
}

