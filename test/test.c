#include "rckskel.h"
#include <string.h>

#ifndef _OPENMP
#define RCCE_APP main
#endif

// local function prototypes
void worker(int, int);
int collect(int);
int check_ready(int id);
void terminate(int idx, int id);

int RCCE_APP(int argc, char **argv) {
	rckskel_env_t env;
	rckskel_env_init(&env, &argc, &argv);

	int job_count = 10, ue_count = env.ue_count, i, ue_id = env.ue_id;

	if (ue_count < 6) {
		printf("The program needs at least 6 UEs\n");
		return -1;
	}

	if (ue_id == MASTER_ID) {
		int job_idxs[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

		int *ue_ids = (int *) malloc(ue_count * sizeof(int));
		task_t *n011 = (task_t *) malloc(sizeof(task_t)), *n012 =
				(task_t *) malloc(sizeof(task_t)), *n021 = (task_t *) malloc(
				sizeof(task_t));
		create_task(n011, SEQ_rckskel, '2'), create_task(n012, MAP_rckskel,
				'3'), create_task(n021, MAP_rckskel, '5');

		for (i = 0; i < ue_count; i++) {
			ue_ids[i] = i;
		}

		n011->worker = &worker;
		n011->collector = &collect;
		n012->worker = &worker;
		n012->collector = &collect;
		n021->worker = &worker;
		n021->collector = &collect;

		n011->job_idxs = &job_idxs[0];
		n012->job_idxs = &job_idxs[2];
		n021->job_idxs = &job_idxs[4];

		task_t *n01 = (task_t *) malloc(sizeof(task_t)), *n02 =
				(task_t *) malloc(sizeof(task_t)), *n0 = (task_t *) malloc(
				sizeof(task_t));
		printf("create tasks\n");
		create_task(n01, SEQ_rckskel, '2');
		create_task(n02, MAP_rckskel, '4');
		create_task(n0, MAP_rckskel, '0');

		set_task_ues(n011, 2, ue_ids[1], ue_ids[2]);
		set_task_ues(n012, 2, ue_ids[3], ue_ids[4]);
		set_task_ues(n021, 1, ue_ids[5]);

		printf("add children\n");
		add_children(n0, 2, n01, n02);
		add_children(n01, 2, n011, n012);
		add_children(n02, 1, n021);

		n0->check_ready = &check_ready;

		/// check if all UEs are ready
		COLLECT(ue_count, ue_ids, &check_ready);
		printf("done collecting\n");

		/// test complex tree
		printf("testing complex tree\n");
		set_unfinished_count(n011, 2);
		set_unfinished_count(n012, 2);
		set_unfinished_count(n021, 4);
		//rckskel_process_task_tree(n0, ue_count, ue_ids);

		//process&n0, ue_count, ue_ids);
		printf("testing MAP\n");
		set_unfinished_count(n011, 2);
		set_unfinished_count(n012, 2);
		set_unfinished_count(n021, 4);
		MAP(ue_count, ue_ids, &check_ready, 3, n011, n012, n021);

		printf("testing SEQ\n");
		set_unfinished_count(n011, 2);
		set_unfinished_count(n012, 2);
		set_unfinished_count(n021, 4);
		//SEQ(ue_count, ue_ids, &check_ready, 2, n021, n011);

		printf("testing FARM\n");
		set_unfinished_count(n011, 2);
		set_unfinished_count(n012, 2);
		set_unfinished_count(n021, 4);
		//FARM(ue_count, ue_ids, &check_ready, 2, n021, n011);

		printf("testing FOR\n");
		//FOR(10, ue_count, ue_ids, n011);
		//////
		printf("send terminate to all ues\n");
		set_unfinished_count(n011, 2);

		for (i = 0; i < ue_count; i++) {
			terminate(0, i);
		}
		printf("master specific free stuff.\n");
		free(ue_ids);
		free_task(n011);
		free_task(n012);
		free_task(n021);
	} else {
		/// send the master ready signal
		READY_T ready_t;
		ready_t.ready = 1; //IS_READY;
		RCCE_send((char *) &ready_t, sizeof(READY_T), MASTER_ID);

		/// start recieving jobs
		int die, b;
		while (die != -1) { //DIE
			b = 100;
			printf("ue %d waiting next input\n", ue_id);
			RCCE_recv((char *) &die, sizeof(int), MASTER_ID);
			if (die == -1)
				continue;
			/// the job
			while (b > 10)
				b = rand() % 100;
			/// end job
			RCCE_send((char *) &b, sizeof(int), MASTER_ID);
		}
	}

	printf("ue: %d exiting\n", ue_id);
	//RCCE_finalize();
	return 0;
}

void worker(int idx, int id) {
	int i = rand() % 100;
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

int check_ready(int id) {
	READY_T ready_t;
	int test_flag;
	RCCE_recv_test((char *) &ready_t, sizeof(READY_T), id, &test_flag);
	if (test_flag == 0)
		return RCKSKEL_FALSE;
	return (ready_t.ready == 1) ? RCKSKEL_TRUE : RCKSKEL_FALSE;
}

void terminate(int idx, int id) {
	if (id == MASTER_ID) {
		printf("not terminating master.\n");
		return;
	}
	printf("terminate ue %d\n", id);
	int i = -1;
	RCCE_send((char *) &i, sizeof(int), id);
}

