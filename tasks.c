#include <stdlib.h>

#include "tasks.h"
#include "tasks_implem.h"
#include "debug.h"
#include "utils.h"

system_state_t sys_state;

__thread task_t *active_task;


void runtime_init(void)
{
    /* a random number generator might be useful towards the end of
       the lab */
    rand_generator_init();

    create_queues();
    create_thread_pool();

    sys_state.task_counter = 0;    
}

void runtime_init_with_deps(void)
{
#ifndef WITH_DEPENDENCIES
    fprintf(stderr, "ERROR: dependencies are not supported by the runtime. This application cannot be executed\n");
    exit(EXIT_FAILURE);
#endif

    runtime_init();
}



void runtime_finalize(void)
{
    task_waitall();

    PRINT_DEBUG(1, "Terminating ... \t Total task count: %lu \n", sys_state.task_counter);
    
    delete_queues();

    delete_threads();
}


task_t* create_task(task_routine_t f)
{
    task_t *t = malloc(sizeof(task_t));

    t->task_id = ++sys_state.task_counter;    
    t->fct = f;
    t->step = 0;

    t->tstate.input_list = NULL;
    t->tstate.output_list = NULL;

#ifdef WITH_DEPENDENCIES
    t->tstate.output_from_dependencies_list = NULL;
    t->task_dependency_count = 0;
    t->parent_task = NULL;
    pthread_mutex_init(&t->mtx_dep, NULL);
#endif
    
    t->status = INIT;
    
    PRINT_DEBUG(10, "task created with id %u\n", t->task_id);    
    
    return t;
}

void submit_task(task_t *t)
{
    t->status = READY;

#ifdef WITH_DEPENDENCIES    
    if(active_task != NULL){
        t->parent_task = active_task;

        pthread_mutex_lock(&(active_task->mtx_dep));
        active_task->task_dependency_count++;
        pthread_mutex_unlock(&(active_task->mtx_dep));
        
        PRINT_DEBUG(100, "Dependency %u -> %u\n", active_task->task_id, t->task_id);
    }
#endif
    
    pthread_mutex_lock(&mtx_tasks);
    tasks_counter++;
    pthread_mutex_unlock(&mtx_tasks);

    dispatch_task(t);
}


void task_waitall(void)
{
    /*active_task = get_task_to_execute();

    while(active_task != NULL){
        task_return_value_t ret = exec_task(active_task);

        if (ret == TASK_COMPLETED){
            terminate_task(active_task);
        }
#ifdef WITH_DEPENDENCIES
        else{
            active_task->status = WAITING;
        }
#endif

        active_task = get_task_to_execute();
    }*/

    //Add a global counter that will be :
    //  - incremented by the main when a task is added
    //  - decremented by the threads when a task is terminated
    //When a task terminates, it should signal (through condition variables) if the queue is empty.
    //Change the following line to avoid busy waiting and wait for being signaled
    
    pthread_mutex_lock(&mtx_tasks);
    while(tasks_counter > 0){
        pthread_cond_wait(&cond_tasks, &mtx_tasks);
    }
    pthread_mutex_unlock(&mtx_tasks);

}
