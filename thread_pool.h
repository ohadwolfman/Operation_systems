#ifndef THREAD_POOL_H
#define THREAD_POOL_H

typedef struct ThreadPool ThreadPool;
typedef struct Task Task;

void *thread_worker(void *arg);
ThreadPool *thread_pool_create(int num_threads);
void thread_pool_add_task(ThreadPool *pool, void (*function)(void *), void *arg);
void destroy_thread_pool(ThreadPool *pool);

#endif
