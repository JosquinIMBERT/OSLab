#ifndef __TASKS_IMPLEM_H__
#define __TASKS_IMPLEM_H__

#include <pthread.h>

#include "tasks_types.h"

extern pthread_mutex_t mtx_tasks;
extern pthread_cond_t cond_tasks;
extern int tasks_counter;

void create_queues(void);
void delete_queues(void);
void delete_threads(void);

void create_thread_pool(void);

void dispatch_task(task_t *t);
task_t* get_task_to_execute(void);
unsigned int exec_task(task_t *t);
void terminate_task(task_t *t);

void task_check_runnable(task_t *t);

#endif
