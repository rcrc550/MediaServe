#include "tcpnet.h"

TcpNet::TcpNet(IKernel *kernel1)
{
    kernel=kernel1;
    pool=NULL;
}

int TcpNet::Initnetwork()
{

       loop=1;
       t_pool =new Threadpool;
       pool=t_pool->pool_create(400,10,400);
       printf("c%d\n",pool->pthread_max);
       if(pool==NULL)printf("create pool failed\n");

       sockaddr_in server;
       bzero(&server,sizeof(server));
       server.sin_family = AF_INET;
       server.sin_addr.s_addr=inet_addr("192.168.0.103");
       server.sin_port = htons(DEF_PORT);
       //创建Socket
       if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
       {
           perror("Create Accept fd Error:");
           return false;
       }
       int op;
       setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char*)&op,sizeof(op));
       //绑定端口号
       if(bind(sockfd,(struct sockaddr*)&server,sizeof(server)) == -1)
       {
           perror("Bind Socket Error:");
           return false;
       }
       //监听socket
       if(listen(sockfd,128) == -1)
       {
           perror("Listen Error:");
           return false;
       }
       epfd = epoll_create(4096);

       addfd(sockfd);



       return true;
}

void TcpNet::Unitnetwork()
{
    loop=0;
    close(sockfd);
    close(epfd);
}

int TcpNet::SendData(int fd, char *szbuf, int len)
{
    if(send(fd,(const char*)&len,sizeof(int),0)<=0)
        return false;
    if(send(fd,szbuf,len,0)<=0)
        return false;
    return true;
}

void *TcpNet::Epoll_loop(void *arg)
{

    int ready=0;


    while(loop)
    {

        if((ready = epoll_wait(epfd,epoll_ready,4096,-1)) == -1)
                   perror("Epoll Wait Failed:");

            Epoll_deal(ready,pool);
               bzero(epoll_ready,sizeof(epoll_ready));
    }

}

void TcpNet::Epoll_deal(int ready, pool_t *c)
{
    //printf("deal\n");
       int i = 0;
       for(i=0; i<ready; i++)
       {
           // printf("deal\n");
           int fd = epoll_ready[i].data.fd;
           if(sockfd == fd)   //客户端建立链接Epoll_deal
           {
               t_pool->add_job(c,accept_job,(void*)(this));
               //printf("deal\n");
           }
           else if(epoll_ready[i].events & EPOLLIN)
           {
                mythis *my=new mythis;
               my->clientfd=fd;
               my->pthis=this;
               t_pool->add_job(c,recv_job,(void*)my);
              // printf("deal\n");
           }
            //printf("deal\n");
       }
      //  printf("deal\n");
}

void *TcpNet::recv_job(void *arg)
{
    mythis* p=(mythis*)arg;//64为下指针为8位，用long来转。
    while(1)
{
    int buffersize=0;
    int readnum=0;
    readnum=recv(p->clientfd,&buffersize,sizeof(buffersize),MSG_DONTWAIT);
   // printf("readnum%d",readnum);
    if(readnum<=0)
    {
        break;
        if(readnum==EAGAIN)
        {
           //break;
        }
        else {
            printf("fd %d disconnect\n",p->clientfd);
            close(p->clientfd);
            return NULL;
        }

    }
    char *szbuf=(char*)malloc(sizeof(char)*buffersize);
    int offset = 0;
    readnum = 0;
    //接收包的数据
    while(buffersize)
    {
        readnum = recv(p->clientfd,szbuf+offset,buffersize,MSG_DONTWAIT);
        if(readnum > 0)
        {
            offset += readnum;
            buffersize -= readnum;
        }
    }
    p->pthis->kernel->DealData(p->clientfd,szbuf,offset);

    if(szbuf != NULL)
    {
        free(szbuf);
        szbuf = NULL;
    }
}

    delete p;
    p=NULL;

    return 0;


}

void TcpNet::setbuffersize(int clientfd)
{
    int nRecvBuf = 256*1024;//设置为 256 K
    setsockopt(clientfd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
    int nSendBuf=128*1024;//设置为 128 K
    setsockopt(clientfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
}
void *TcpNet::accept_job(void *arg)
{
    printf("accept \n");
    TcpNet*pthis=(TcpNet*)arg;
    struct sockaddr_in clientaddr;
    int clientsize = sizeof(clientaddr);
    int clientfd=0;
    pthread_mutex_lock(&pthis->acclock);
    if((clientfd = accept(pthis->sockfd,(struct sockaddr*)&clientaddr,(socklen_t*)&clientsize)) == -1)
    {
        perror("Thread Accept Error");
        pthread_mutex_unlock(&pthis->acclock);
        return 0;
    }
    printf("accept \n");
    pthread_mutex_unlock(&pthis->acclock);
    pthis->setbuffersize( clientfd );
    pthis->setNoDelay(clientfd);
    pthis->addfd(clientfd);


    return 0;
}
#include <netinet/tcp.h>
void TcpNet::setNoDelay( int fd)
{
    //nodelay
    int value = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY ,(char *)&value, sizeof(int));
}

void TcpNet::addfd(int fd)
{
       struct epoll_event ep;

       ep.data.fd = fd;
       ep.events=EPOLLIN;/*EPOLLET|*/
       ep.events|=EPOLLET ;
       epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ep);
}

void TcpNet::deletefd(int fd)
{
    epoll_ctl(epfd,EPOLL_CTL_DEL,fd,0);
}
