#ifndef TCPNET_H
#define TCPNET_H
#include"packdef.h"
#include"threadpool.h"
class pool_t;
class Threadpool;
using namespace  std;
class IKernel
{
public:
    virtual void DealData(int,char*,int) = 0;
};
typedef void (*DealData_CallBack)(int, char*, int );


class TcpNet
{
public:
    TcpNet(IKernel *kernel1);
public:
    int Initnetwork();
    void Unitnetwork();
    int SendData(int fd,char *szbuf,int len);
    void *Epoll_loop(void *);
    void Epoll_deal(int ready,pool_t *c);
    static void *recv_job(void *);
    static void *accept_job(void *);
    void addfd(int fd);
     void deletefd(int fd);
    void setbuffersize(int clientfd);
    void setNoDelay( int fd);
public:
    int epfd;
    int sockfd;
    struct epoll_event epoll_ready[4096];
    pthread_mutex_t acclock;
    Threadpool *t_pool;
    pool_t *pool;
    IKernel *kernel;
    int loop=1;

};

#endif // TCPNET_H
