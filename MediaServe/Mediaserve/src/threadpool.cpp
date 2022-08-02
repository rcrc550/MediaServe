#include "threadpool.h"

Threadpool::Threadpool()
{

}

void *Threadpool::cosume(void *arg)
{

    pool_t * p = (pool_t*)arg;
   // printf("%d\n",p->pthread_max);
   // printf("%d\n",p->pthread_min);

    task_t task;
   // printf("%d\n",p->thread_shutdown);
    while(p->thread_shutdown)
    {
        pthread_mutex_lock(&p->lock);
       // printf("con\n");
        while(p->cur == 0 && p->thread_shutdown  )
        {
            pthread_cond_wait(&p->cond_consume,&p->lock);
        }
        //printf("con\n");
        if(!p->thread_shutdown  )
        {
            pthread_mutex_unlock(&p->lock);
            pthread_exit(NULL);
        }
      //  printf("con\n");
        if(p->pthread_wait > 0 && p->pthread_alive > p->pthread_min)//动态缩减
        {
            --(p->pthread_wait);
            --(p->pthread_alive);
            pthread_mutex_unlock(&p->lock);
            pthread_exit(NULL);
        }

        task.consu = p->queue_jobs[p->back].consu;
        task.arg = p->queue_jobs[p->back].arg;
        p->back = (p->back + 1) % p->queuemax;

        --(p->cur);
        pthread_cond_signal(&p->cond_create);
        ++(p->pthread_busy);

        pthread_mutex_unlock(&p->lock);

        //执行核心工作
        //printf("con\n");
        (*task.consu)(task.arg);
        pthread_mutex_lock(&p->lock);
        --(p->pthread_busy);
        pthread_mutex_unlock(&p->lock);
         //printf("con\n");
    }
        return 0;
}

void *Threadpool::manager(void *arg)
{
    pool_t * p = (pool_t *)arg;
    int alive;
    int cur;
    int busy;
    int add = 0;
    while(p->thread_shutdown )
    {
        pthread_mutex_lock(&p->lock);
        alive = p->pthread_alive;
        busy = p->pthread_busy;
        cur = p->cur;
        pthread_mutex_unlock(&p->lock);
        if((cur > alive - busy || (float)busy / alive*100 >= (float)80 ) &&
                p->pthread_max > alive)
        {
            for(int j = 0;j<p->pthread_min;j++)
            {
                for(int i = 0;i<p->pthread_max;i++)
                {
                    if(p->tids[i] == 0 || !thread_alive(p->tids[i]))
                    {
                        pthread_mutex_lock(&p->lock);
                        pthread_create(&p->tids[i],NULL,cosume,(void*)p);
                        ++(p->pthread_alive);
                        pthread_mutex_unlock(&p->lock);
                        break;
                    }
                }
            }
        }

        if(busy *2 < alive - busy && alive > p->pthread_min)
          {
            pthread_mutex_lock(&p->lock);
            p->pthread_wait = _DEF_COUNT;
            pthread_mutex_unlock(&p->lock);
            for(int i=0; i<_DEF_COUNT; i++)
            {
                pthread_cond_signal(&p->cond_consume);
            }
         }
        }
        sleep(10);

    return 0;
}

int Threadpool::pool_destroy(pool_t *c)
{
/*
        free(c->tids);
        c->thread_shutdown=0;
        for(int i=0;i<c->pthread_max;i++)
        {
            pthread_cond_signal(&c->cond_consume);
        }
        sleep(3);
        pthread_mutex_destroy(&c->lock);
        pthread_cond_destroy(&c->cond_create);
        pthread_cond_destroy(&c->cond_consume);
        free(c->queue_jobs);
        free(c);
        c=NULL;*/

}

pool_t *Threadpool::pool_create(int threadmax, int threadmin, int queueamx)
{
    pool_t *pool=NULL;
    pool=(pool_t*)malloc(sizeof(pool_t));
    bzero(pool,sizeof(pool_t));
    if(pool==NULL)
    {
        printf("malloc err\n");
        return NULL;
    }
    pool->cur=0;
    pool->queuemax=queueamx;
    pool->back=0;
    pool->front=0;

    pool->queue_jobs=(task_t*)malloc(sizeof(task_t)*queueamx);
    if(pool->queue_jobs==NULL)
    {
        printf("new wrong\n");
        return NULL;
    }

    pool->pthread_min=threadmin;

    pool->pthread_busy=0;
    pool->pthread_max=threadmax;
    pool->pthread_alive=threadmin;
    pool->tids=(pthread_t*)malloc(sizeof(pthread_t)*threadmax);
     bzero(pool->tids,sizeof(pthread_t)*threadmax);
     int err=0;
     pool->thread_shutdown=1;
    for(int i=0; i<threadmin; i++)
   {

           if((err = pthread_create(&pool->tids[i],NULL, cosume,(void*)pool))>0)
           {
               printf("create custom error:%s\n",strerror(err));
               return NULL;
           }
           ++(pool->pthread_alive);
   }
    //pthread_mutexattr_init() 函数成功完成之后会返回零，其他任何返回值都表示出现了错误。
    if(pthread_cond_init(&pool->cond_create,NULL)!=0 ||pthread_cond_init(&pool->cond_consume,NULL)!=0 ||
        pthread_mutex_init(&pool->lock,NULL)!=0)
    {
           perror("init cond or mutex error:");
           return NULL;
    }
    return pool;

}

int Threadpool::add_job(pool_t *p, void *task(void *), void *arg)
{
    pthread_mutex_lock(&p->lock);
     while(p->cur == p->queuemax && p->thread_shutdown  )
     {
         pthread_cond_wait(&p->cond_create,&p->lock);
     }
     if(!p->thread_shutdown  )
     {
         pthread_mutex_unlock(&p->lock);
         return -1;
     }
     p->queue_jobs[p->front].consu = task;
     p->queue_jobs[p->front].arg = arg;
     p->front = (p->front + 1) % p->queuemax;
     ++(p->cur);
      pthread_cond_signal(&p->cond_consume);
     pthread_mutex_unlock(&p->lock);

     return 0;
}

int Threadpool::thread_alive(pthread_t thread)
{
    if((pthread_kill(thread,0)) == -1)
       {
           if(errno == ESRCH)
               return false;
       }

       return true;

}
