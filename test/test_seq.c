#include "rckskel.h"
#include <string.h>

#ifndef _OPENMP
#define RCCE_APP main
#endif

// local function prototypes
void worker1(int, int);
void worker2(int, int);
void worker3(int, int);
int collect(int);

int RCCE_APP(int argc, char **argv) {
	rckskel_env_t env;
	rckskel_env_init(&env, &argc, &argv);

	if (env.ue_id == MASTER_ID) {
		check_ues_up(&env.ue_count);

		task_t *task1 = (task_t *) malloc(sizeof(task_t)), *task2 =
				(task_t *) malloc(sizeof(task_t)), *task3 = (task_t *) malloc(
				sizeof(task_t));
		task1->worker = &worker1;
		task1->collector = &collect;
		task2->worker = &worker2;
		task2->collector = &collect;
		task3->worker = &worker3;
		task3->collector = &collect;

		task_t *task0 = (task_t *) malloc(sizeof(task_t));

		create_task(task0, SEQ_rckskel, '0');
		create_task(task1, SEQ_rckskel, '1');
		create_task(task2, SEQ_rckskel, '2');
		create_task(task3, SEQ_rckskel, '3');

		terminate_ues(&env.ue_count);
	} else {
		send_ue_ready(&env);
		while (ue_die() != -1) {
			// do the job
		}
	}

	printf("ue: %d exiting\n", env.ue_id);
	//RCCE_finalize();
	return 0;
}

void worker1(int idx, int id) {
	int i = 10;
	RCCE_send((char *) &i, sizeof(int), id);
}

void worker2(int idx, int id) {
	int i = 20;
	RCCE_send((char *) &i, sizeof(int), id);
}

void worker3(int idx, int id) {
	int i = 30;
	RCCE_send((char *) &i, sizeof(int), id);
}

int collect(int id) {
	int i, test_flag;
	RCCE_recv_test((char *) &i, sizeof(int), id, &test_flag);
	if (test_flag == 0)
		return RCKSKEL_FALSE;
	printf("recieved %d from ue: %d\n", i, id);
	return RCKSKEL_TRUE;
}

