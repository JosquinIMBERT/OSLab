#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "tasks_implem.h"
#include "tasks_queue.h"
#include "debug.h"

tasks_queue_t *tqueue= NULL;
pthread_t *thread_pool= NULL;

//Thread protection for counting the tasks
pthread_mutex_t mtx_tasks = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_tasks =  PTHREAD_COND_INITIALIZER;
int tasks_counter = 0;

void create_queues(void)
{
    tqueue = create_tasks_queue();
}

void delete_queues(void)
{
    free_tasks_queue(tqueue);
}    

void delete_threads(void)
{
    free(thread_pool);
}  

void *tasks_consumer(void *arg){
    while(1){
        //Getting a task
        active_task = dequeue_task(tqueue);
        
        //Running the task
        unsigned int result = exec_task(active_task);
        
        //Checking the result
        if (result == TASK_COMPLETED){
            terminate_task(active_task);
            pthread_mutex_lock(&mtx_tasks);
            tasks_counter--;
            pthread_cond_signal(&cond_tasks); //Signal for main thread's task_waitall function
            pthread_mutex_unlock(&mtx_tasks);
        }
#ifdef WITH_DEPENDENCIES
        else { //TASK_TO_BE_RESUMED
            active_task->status = WAITING;

            pthread_mutex_lock(&(active_task->mtx_dep));            
            task_check_runnable(active_task);
            pthread_mutex_unlock(&(active_task->mtx_dep));

        }
#endif
    }
}

void create_thread_pool(void)
{
    thread_pool = malloc(sizeof(pthread_t) * THREAD_COUNT);

    //Creating the threads
    for(int i=0; i<THREAD_COUNT; i++) {
        if( pthread_create(&thread_pool[i], NULL, tasks_consumer, NULL) != 0 ) {
            fprintf(stderr, "Failed to create the tasks consuming thread.\n");
            exit(1);
        }
    }

    //free(thread_pool);
}

void dispatch_task(task_t *t)
{
    enqueue_task(tqueue, t);
}

task_t* get_task_to_execute(void)
{
    return dequeue_task(tqueue);
}

unsigned int exec_task(task_t *t)
{
    t->step++;
    t->status = RUNNING;

    PRINT_DEBUG(10, "Execution of task %u (step %u)\n", t->task_id, t->step);
    
    unsigned int result = t->fct(t, t->step);
    
    return result;
}

void terminate_task(task_t *t)
{
    t->status = TERMINATED;
    
    PRINT_DEBUG(10, "Task terminated: %u\n", t->task_id);

#ifdef WITH_DEPENDENCIES
    if(t->parent_task != NULL){
        task_t *waiting_task = t->parent_task;

        pthread_mutex_lock(&(waiting_task->mtx_dep));
        waiting_task->task_dependency_done++;
        
        task_check_runnable(waiting_task);
        
        pthread_mutex_unlock(&(waiting_task->mtx_dep));
    }
#endif

}

void task_check_runnable(task_t *t)
{
#ifdef WITH_DEPENDENCIES
    if(t->task_dependency_done == t->task_dependency_count && t->status==WAITING && t->task_dependency_count>0){
        t->task_dependency_done = 0;
        t->task_dependency_count = 0;
        t->status = READY;
        dispatch_task(t);
    }
#endif
}
