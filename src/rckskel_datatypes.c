#include "rckskel.h"

// local function protypes
void __update_unfinished_counts(task_t *parent, int unfinished_count);

//
void rckskel_env_init(rckskel_env_t *env, int *argc, char ***argv) {
	RCCE_init(argc, argv);
	env->ue_id = RCCE_ue();
	env->ue_count = RCCE_num_ues();
}

//
void create_task(task_t *task, skeltag_t skel, char id) {
	init_task(task);
	task->id = id;
	task->skel = skel;
}

//
void create_task(task_t *task, skeltag_t skel, char id, int ctask_count, ...) {
	int i;
	va_list ap;

	task->children = (task_t **) malloc(ctask_count * sizeof(task_t *));
	va_start(ap, ctask_count);
	for (i = 0; i < ctask_count; i++) {
		add_child(task, va_arg(ap, task_t *));
	}
	va_end(ap);
}

//
void init_task(task_t *task) {
	task->child_count = 0;
	task->unfinished_count = 0;
	task->current_job_index = 0;
	task->ue_ids = NULL;
	task->parent = NULL;
	task->children = NULL;
}

//
void __update_unfinished_counts(task_t *parent, int unfinished_count) {
	// update unfinished job counts along the path to root
	task_t *p = parent;
	while (parent != NULL) {
		parent->unfinished_count += unfinished_count;
		parent = parent->parent;
	}
}

//
void reset_unfinished_job_counts(task_t *task) {
	if (task->child_count == 0) {
		// we are at a leaf. set unfinished to job count & update parents
		task->unfinished_count = task->job_count;
		task->current_job_index = 0;
		if (task->parent != NULL)
			__update_unfinished_counts(task->parent, task->unfinished_count);
		return;
	}

	int i;
	for (i = 0; i < task->child_count; i++)
		reset_unfinished_job_counts(task->children[i]);
}

// expects the parents children pointer has been correctly
// initialized
void add_child(task_t *parent, task_t *child) {
	if (parent == NULL) {
		printf("Parent cannot be NULL.\n");
		return;
	}

	// add child to parent
	parent->children[parent->child_count++] = child;
	child->parent = parent;
	__update_unfinished_counts(parent, child->unfinished_count);
}

//
void add_children(task_t *parent, int child_count, ...) {
	va_list ap;
	int i;
	va_start(ap, child_count);
	parent->children = (task_t **) malloc(child_count * sizeof(task_t *));
	for (i = 0; i < child_count; i++)
		add_child(parent, va_arg(ap, task_t *));
	va_end(ap);
}

//
void set_unfinished_count(task_t *node, int unfinished_count) {
	if (node->child_count > 0) {
		printf("Invalid operation (set_unfinished_count) on non-leaf node.\n");
		return;
	}

	node->unfinished_count = unfinished_count;
	node->job_count = unfinished_count;
	node->current_job_index = 0;
	__update_unfinished_counts(node->parent, unfinished_count);
}

//
void set_task_ues(task_t *node, int ue_count, ...) {
	int i;
	va_list ap;

	node->ue_count = ue_count;
	node->ue_ids = (int *) malloc(ue_count * sizeof(int));
	va_start(ap, ue_count);
	for (i = 0; i < ue_count; i++) {
		node->ue_ids[i] = va_arg(ap, int);
	}
	va_end(ap);
}

//
void set_task_ues(task_t *node, int ue_count, int *ue_ids) {
	node->ue_count = ue_count;
	node->ue_ids = (int *) malloc(ue_count * sizeof(int));
	for (int i = 0; i < ue_count; i++) {
		node->ue_ids[i] = ue_ids[i];
	}
}

//
void free_task(task_t *task) {
	if (task->ue_ids != NULL)
		free(task->ue_ids);

	if (task->children != NULL)
		free(task->children);
	free(task);
}

