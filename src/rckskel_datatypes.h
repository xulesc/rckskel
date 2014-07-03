#ifndef _RCKSKEL_DATATYPES_
#define _RCKSKEL_DATATYPES_

/**
 * All functions supported by the library or intended to be supported.
 */
typedef enum {
	MAP_rckskel,
	FORK_rckskel,
	REDUCE_rckskel,
	COLLECT_rckskel,
	SEQ_rckskel,
	FARM_rckskel,
	PIPE_rckskel,
	FOR_rckskel,
	WHILE_rckskel,
	IF_rckskel,
	DNC_rckskel,
	BNB_rckskel
} skeltag_t;

/**
 * Environment used by an application built with rckskel. All variables
 * commonly required at different points of the application relevant to
 * rckskel are present in the structure.
 */
typedef struct rckskel_env {
	/// id of the processing node. this maps to a core on the SCC processor.
	int ue_id;
	/**
	 The total number of SCC cores allocated to the application as specified 
	 in the command line arguments.
	 */
	int ue_count;
} rckskel_env_t;

/**
 The structure contains the description of a task as defined by the rckskel
 library. A task can contain one or more tasks as its children, failing which
 it should containing one or more jobs. A task is never formally 'run' jobs
 are run. Depending on the environment described by the task structure the 
 order and manner of execution of the jobs is determined.
 */
typedef struct rckskel_task {
	skeltag_t skel;
	int ue_count;
	int *ue_ids;
	void (*worker)(int, int);
	void (**workers)(int, int);
	int (*collector)(int);
	int (*check_ready)(int);
	void (*terminate)(int, int);

	char id;
	int job_count;
	int current_job_index;
	int *job_idxs;
	int child_count;
	int unfinished_count;
	struct rckskel_task *parent;
	struct rckskel_task **children;
} task_t;

typedef struct {
	int value;
} JOB;

typedef struct {
	int ready;
} READY_T;

/**
 This function initialize's the rckskel environment. The function serves to
 setup the RCCE environment as well fill the environment structure used by
 the library.
 */
void rckskel_env_init(rckskel_env_t *env, int *argc, char ***argv);
void create_task(task_t *task, skeltag_t skel, char id);
void create_task(task_t *task, skeltag_t skel, char id, int ctask_count, ...);
void init_task(task_t *task);
void add_child(task_t *parent, task_t *child);
void add_children(task_t *parent, int child_count, ...);
void set_unfinished_count(task_t *node, int unfinished_count);
void reset_unfinished_job_counts(task_t *task);
void set_task_ues(task_t *node, int ue_count, ...);
void set_task_ues(task_t *node, int ue_count, int *ue_ids);
void free_task(task_t *task);

#endif

