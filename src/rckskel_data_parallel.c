#include "rckskel.h"

// 
void MAP(int ue_count, int *ue_ids, int (*check_ready)(int), int task_count,
		...) {
	if (task_count > ue_count) {
		printf("\nWARNING: task count exceeds ue count in MAP function.\n");
		printf("\tOnly first %d (ue count) number of tasks will process.\n\n",
				task_count);
		task_count = ue_count;
	}

	va_list ap;
	va_start(ap, task_count);
	rckskel_prepare_process(MAP_rckskel, ue_count, ue_ids, check_ready,
			task_count, &ap);
}

//
void COLLECT(int ue_count, int *ue_ids, int (*collector)(int)) {
	int i, test_flags[ue_count];

	for (i = 0; i < ue_count; i++)
		test_flags[i] = (i == MASTER_ID) ? RCKSKEL_TRUE : RCKSKEL_FALSE;

	int stop;
	do {
		stop = RCKSKEL_TRUE;
		for (i = 0; i < ue_count; i++) {
			if (test_flags[i] == RCKSKEL_TRUE)
				continue;
			test_flags[i] = collector(ue_ids[i]);
			if (test_flags[i] == RCKSKEL_FALSE)
				stop = RCKSKEL_FALSE;
		}
	} while (stop == RCKSKEL_FALSE);
}

// TO BE DONE
//FORK
//REDUCE

