#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "tasks_queue.h"




tasks_queue_t* create_tasks_queue(void)
{
    tasks_queue_t *q = (tasks_queue_t*) malloc(sizeof(tasks_queue_t));

    //Initialization of semaphores for thread safe queue
    sem_init(&q->mutex,0,1);
    sem_init(&q->fullCount,0,0);
    sem_init(&q->emptyCount,0,QUEUE_SIZE);

    q->task_buffer_size = QUEUE_SIZE;
    q->task_buffer = (task_t**) malloc(sizeof(task_t*) * q->task_buffer_size);

    q->index = 0;

    return q;
}


void free_tasks_queue(tasks_queue_t *q)
{
    /* IMPORTANT: We chose not to free the queues to simplify the
     * termination of the program (and make debugging less complex) */
    
    /* free(q->task_buffer); */
    /* free(q); */
}


void enqueue_task(tasks_queue_t *q, task_t *t)
{
    /*if(q->index == q->task_buffer_size){
        fprintf(stderr,"ERROR: the queue of tasks is full\n");
        exit(EXIT_FAILURE);
    }*/

    //Tasks Producer
    sem_wait(&q->emptyCount); //If the queue is full, we wait to add the new task
    sem_wait(&q->mutex);
    q->task_buffer[q->index] = t;
    q->index++;
    sem_post(&q->mutex);
    sem_post(&q->fullCount);
}


task_t* dequeue_task(tasks_queue_t *q)
{
    /*if(q->index == 0){
        return NULL;
    }*/

    //Tasks Consumer
    sem_wait(&q->fullCount); //If there is no more tasks, we wait
    sem_wait(&q->mutex);
    task_t *t = q->task_buffer[q->index-1];
    q->index--;
    sem_post(&q->mutex);
    sem_post(&q->emptyCount);

    return t;
}

