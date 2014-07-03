#include "rckskel.h"

// local function prototypes
void __parse_inorder(task_t *node, int *ue_status, task_t **ue_tasks);
void __decreament_counts(task_t *node);
int __check_ready(int id);
void __terminate_ue(int id);

//
void send_ue_ready(rckskel_env_t *env) {
	READY_T ready_t;
	ready_t.ready = 1;
	RCCE_send((char *) &ready_t, sizeof(READY_T), MASTER_ID);
}

//
int __check_ready(int id) {
	READY_T ready_t;
	int test_flag;
	RCCE_recv_test((char *) &ready_t, sizeof(READY_T), id, &test_flag);
	if (test_flag == 0)
		return RCKSKEL_FALSE;
	return (ready_t.ready == 1) ? RCKSKEL_TRUE : RCKSKEL_FALSE;
}

//
void check_ues_up(int *ue_count) {
	int ue_ids[*ue_count];
	for (int i = 0; i < *ue_count; i++)
		ue_ids[i] = i;
	COLLECT(*ue_count, ue_ids, &__check_ready);
	printf("All UEs are up\n");
}

//
int ue_die() {
	int die;
	RCCE_recv((char *) &die, sizeof(int), MASTER_ID);
	return die;
}

//
void terminate_ues(int *ue_count) {
	int i;
	for (i = 0; i < *ue_count; i++)
		__terminate_ue(i);
}

//
void __terminate_ue(int id) {
	if (id == MASTER_ID) {
		printf("not terminating master.\n");
		return;
	}
	printf("terminate ue %d\n", id);
	int i = -1;
	RCCE_send((char *) &i, sizeof(int), id);
}

//
void __decreament_counts(task_t *node) {
	node->unfinished_count--;
	task_t *parent = node->parent;
	while (parent != NULL) {
		parent->unfinished_count--;
		parent = parent->parent;
	}
}

//
void rckskel_process_task_tree(task_t *root, int ue_count, int *ue_ids) {
	int ue_status[ue_count], i;
	// make array of pointers to jobs/tasks as many as UEs
	// the array will be used to update the tree when a UE
	// finishes processing a job.
	task_t * ue_task, * ue_tasks[ue_count];
	// set all UEs unassigned
	for (i = 0; i < ue_count; i++)
		ue_status[i] = RCKSKEL_UE_UNASSIGNED;
	while (root->unfinished_count > 0) {
		// visit job lists and issue jobs where possible
		__parse_inorder(root, ue_status, ue_tasks);

		// check all UEs for completed jobs
		for (i = 0; i < ue_count; i++) {
			// if UE is not processing anything dont do anything
			if (ue_status[i] == RCKSKEL_UE_UNASSIGNED)
				continue;

			// check if UE has completed its task
			ue_task = ue_tasks[i];
			if (ue_task == NULL) // no task at UE
				continue;
#ifdef _RCKSKEL_VERBOSE_
			printf("checking status on ue id: %d.\n", ue_ids[i]);
#endif
			if (ue_task->collector(ue_ids[i]) == RCKSKEL_FALSE)
				continue;
#ifdef _RCKSKEL_VERBOSE_
			printf("node finished job.\n");
#endif
			ue_status[i] = RCKSKEL_UE_COMPLETED;
			__decreament_counts(ue_task);
			printf("unfinished job count: %d\n", root->unfinished_count);
		}
	}

	free(ue_tasks);
}

//
void __parse_inorder(task_t *node, int *ue_status, task_t **ue_tasks) {
	int i;
	if (node->child_count == 0) {
#ifdef _RCKSKEL_VERBOSE_
		printf("\tleaf: %c, unfinished count: %d, ue_count: %d\n", node->id,
				node->unfinished_count, node->ue_count);
#endif
		for (i = 0; i < node->ue_count; i++) {
#ifdef _RCKSKEL_VERBOSE_
			printf("\t\tue id: %d, status: %d\n", node->ue_ids[i],
					ue_status[node->ue_ids[i]]);
#endif
			if (ue_status[node->ue_ids[i]] == RCKSKEL_UE_PROCESSING) {
				if (node->skel == SEQ_rckskel)
					break;
				continue;
			}
			if (node->current_job_index >= node->job_count) {
				ue_status[node->ue_ids[i]] = RCKSKEL_UE_UNASSIGNED;
				continue;
			}
			//
			if (ue_status[node->ue_ids[i]] != RCKSKEL_UE_UNASSIGNED
					&& ue_status[node->ue_ids[i]] != RCKSKEL_UE_COMPLETED)
				continue;

			ue_status[node->ue_ids[i]] = RCKSKEL_UE_PROCESSING;
			ue_tasks[node->ue_ids[i]] = node;
#ifdef _RCKSKEL_VERBOSE_
			printf("\t\tsending job: %d of task: %c to ue: %d\n",
					node->job_idxs[node->current_job_index], node->id,
					node->ue_ids[i]);
#endif
			node->worker(node->job_idxs[node->current_job_index],
					node->ue_ids[i]);
			node->current_job_index++;

			// if the node specifies sequential processing we can only process 1 job
			if (node->skel == SEQ_rckskel) {
#ifdef _RCKSKEL_VERBOSE_
				printf("sequential leaf. not processing next job.\n");
#endif
				break;
			}
		}
		return;
	}

#ifdef _RCKSKEL_VERBOSE_
	printf("at node: %c, unfinished count: %d\n", node->id,
			node->unfinished_count);
#endif
	for (i = 0; i < node->child_count; i++) {
		// if the jobs of this child are completed we dont need to visit it or its
		// children
		if (node->children[i]->unfinished_count == 0)
			continue;
		__parse_inorder(node->children[i], ue_status, ue_tasks);
		// if the node specifies sequential processing and current child still
		// has unfinished jobs we can not process the next child.
		if (node->skel == SEQ_rckskel
				&& node->children[i]->unfinished_count > 0)
			return;
	}
}

//
void rckskel_prepare_process(skeltag_t skel, int ue_count, int *ue_ids,
		int (*check_ready)(int), int task_count, va_list *ap) {
	if (task_count == 1) {
		printf("Single task to prepare.\n");
		task_t *task = va_arg(*ap, task_t *);
		task->check_ready = check_ready;
		rckskel_process_task_tree(task, ue_count, ue_ids);
		va_end(*ap);
		return;
	}

	int i;
	task_t *task = (task_t *) malloc(sizeof(task_t));
	create_task(task, skel, '0');
	task->check_ready = check_ready;
	task->children = (task_t **) malloc(task_count * sizeof(task_t *));

	for (i = 0; i < task_count; i++) {
		add_child(task, va_arg(*ap, task_t *));
	}
	va_end(*ap);

	rckskel_process_task_tree(task, ue_count, ue_ids);

	free_task(task);
}

