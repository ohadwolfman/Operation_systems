#include "thread_pool.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Task {
    void (*function)(void *arg); //Pointer to a function to be called when the task is executed.
    void *arg;
    struct Task *next;
} Task;

typedef struct ThreadPool {
    pthread_t *threads;
    int thread_count;

    Task *task_queue_head;
    Task *task_queue_tail;

    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond; //condition variable that allows the processes to "sleep" until there is a new task
    // A process waits on it when the queue is empty, and someone else "wakes" it up when a task comes in

    int stop; // if stop = 1, release all thread
} ThreadPool;

void* thread_worker(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);

        while (pool->task_queue_head == NULL) { // Checks if there are no tasks currently
            if (pool->stop) { // Prevent threads from leaving when there are more tasks in the queue
                pthread_mutex_unlock(&pool->queue_mutex);
                return NULL;
            }
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex); // process wait until "pthread_cond_signal"
            // While sleeping, the mutex is automatically released so as not to block others.
        }

        if (pool->stop) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }

        // Treats the first task in the queue
        Task *task = pool->task_queue_head;
        if (task) {
            pool->task_queue_head = task->next;
            if (pool->task_queue_head == NULL)
                pool->task_queue_tail = NULL;
        }

        // Allow other processes to enter the queue at the same time as the task is being executed
        pthread_mutex_unlock(&pool->queue_mutex);

        if (task) {
            task->function(task->arg);
            free(task); //after sending the args to the function
        }
    }

    return NULL;
}

ThreadPool* thread_pool_create(int num_threads) {
    ThreadPool *pool = malloc(sizeof(ThreadPool));
    if (!pool){
        perror("Failed to allocate ThreadPool\n");
        return NULL;
    }

    pool->thread_count = num_threads;
    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    if (!pool->threads) {
        perror("Failed to allocate threads array\n");
        free(pool);
        return NULL;
    }
    pool->task_queue_head = pool->task_queue_tail = NULL;
    pool->stop = 0;

    if (pthread_mutex_init(&pool->queue_mutex, NULL) != 0) {
        perror("Mutex init failed\n");
        free(pool->threads);
        free(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->queue_cond, NULL) != 0) {
        perror("Cond init failed\n");
        pthread_mutex_destroy(&pool->queue_mutex);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, thread_worker, (void *)pool);
    }
    return pool;
}

void thread_pool_add_task(ThreadPool *pool, void (*function)(void *), void *arg) {
    Task *task = malloc(sizeof(Task));
    if (!task){
        perror("Failed to allocate task\n");
        return;
    }

    task->function = function;
    task->arg = arg;
    task->next = NULL;

    pthread_mutex_lock(&pool->queue_mutex);

    if (pool->task_queue_tail) {
        pool->task_queue_tail->next = task;
        pool->task_queue_tail = task;
    } else {
        pool->task_queue_head = pool->task_queue_tail = task;
    }

    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
}

void thread_pool_destroy(ThreadPool *pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);

    // clean tasks queue
    Task *task = pool->task_queue_head;
    while (task) {
        Task *next = task->next;
        free(task);
        task = next;
    }

    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
    free(pool);
}