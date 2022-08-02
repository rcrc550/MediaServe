#ifndef THREADPOOL_H
#define THREADPOOL_H
#include"packdef.h"
#define _DEF_COUNT 10
#define NUMBER 1000
typedef struct
{
    void *(*consu)(void *);
    void *arg;
}task_t;
class pool_t
{
public:
  pthread_mutex_t lock;
  pthread_cond_t cond_consume;
  pthread_cond_t cond_create;
  task_t *queue_jobs;
  int front;
  int queuemax;
  int cur;
  int back;
  int pthread_busy;
  int pthread_min;
  int pthread_max;
  int pthread_wait;
  int pthread_alive;
  int thread_shutdown;
  pthread_t *tids;
  pthread_t manager_tid;

};
class Threadpool
{
public:
    Threadpool();
public:

public:
    static void *cosume(void *);
    static void *manager(void *);
     int pool_destroy(pool_t*);
    pool_t *pool_create(int threadmax,int threadmin,int queueamx);
    int add_job(pool_t * p,void *(task)(void *arg),void *arg);
     static int thread_alive(pthread_t);
};

#endif // THREADPOOL_H
