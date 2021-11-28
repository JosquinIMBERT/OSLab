#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

#include "tasks_queue.h"




tasks_queue_t* create_tasks_queue(void)
{
    tasks_queue_t *q = (tasks_queue_t*) malloc(sizeof(tasks_queue_t));

    //Initialization of semaphores for thread safe queue
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->fullCount, NULL);

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
    //Tasks Producer
    pthread_mutex_lock(&q->mutex);

    if(q->index==q->task_buffer_size) {
        resize(q);
    }
    q->task_buffer[q->index] = t;
    q->index++;

    pthread_cond_signal(&q->fullCount);
    pthread_mutex_unlock(&q->mutex);
}


task_t* dequeue_task(tasks_queue_t *q)
{
    //Tasks Consumer
    pthread_mutex_lock(&q->mutex);
    while(q->index<=0) {
        pthread_cond_wait(&q->fullCount, &q->mutex);
    }

    task_t *t = q->task_buffer[q->index-1];
    q->index--;

    pthread_mutex_unlock(&q->mutex);

    return t;
}

void resize(tasks_queue_t *q) {
    unsigned int new_size = 2 * q->task_buffer_size;

    //Doube size
    task_t **q_old = q->task_buffer;
    
    q->task_buffer = malloc(new_size * sizeof(task_t *));
    if(q->task_buffer==NULL) {
        fprintf(stderr, "ERROR: malloc failed for queue resizing.\n");
        exit(1);
    }
    memcpy(q->task_buffer, q_old, q->task_buffer_size * sizeof(task_t *));
    free(q_old);

    q->task_buffer_size = new_size;
}