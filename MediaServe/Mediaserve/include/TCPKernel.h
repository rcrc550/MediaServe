#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H


#include"threadpool.h"
#include "Mysql.h"
#include"tcpnet.h"
#include"pthread.h"


//类成员函数指针 , 用于定义协议映射表

class TcpKernel;
typedef void (TcpKernel::*PFUN)(int fd,char*,int nlen);
typedef  struct Myark
{
      int id;

      time_t time;
      Myark()
      {
          time=0;
          id=0;

      }

}Myark;
typedef struct Myinfor
{
     MyMap<int,UserInfo*>m_mapIdToUserInfo;
     vector<Myark>myvec;
}Myinfor;
typedef  struct myu
{
    Myinfor *my_infor;
    pthread_mutex_t **arr;
}myu;
class TcpKernel:public IKernel
{
public:
    //单例模式
    static TcpKernel* GetInstance();


    int Open(int port);
    //初始化随机数
    void initRand();
    //设置协议映射
    void setNetPackMap();
    //关闭核心服务
    void Close();
    //处理网络接收
     void DealData(int clientfd, char*szbuf, int nlen);

     static void sendData2(int clientfd, char*szbuf, int nlen);

    //发送数据
    void SendData( int clientfd, char*szbuf, int nlen );

    /************** 网络处理 *********************/
    void loop();
    //注册
    void RegisterRq(int clientfd, char*szbuf, int nlen);
    //登录
    void LoginRq(int clientfd, char*szbuf, int nlen);
    void Upladrq(int clientfd, char*szbuf, int nlen);
    void  blockdownrq(int clientfd, char*szbuf, int nlen);
    void getrandnrq(int clientfd, char*szbuf, int nlen);
     void getrandnhot(int clientfd, char*szbuf, int nlen);
      void hotgo(int clientfd, char*szbuf, int nlen);



public:
       static void * alive(void* arg);

public:
       CMysql * m_sql;
       //网络
       TcpNet* m_tcp;

private:
    TcpKernel();
    ~TcpKernel();
    //数据库

     static TcpNet* m_tcp2;
    pthread_t alive_tid;
    //协议映射表
    PFUN m_NetPackMap[DEF_PACK_COUNT];
    Myinfor *myinfor;
    //需要建立一个map来映射每个id的socket，房间号，名字 来转发数据音频画面，判断一个房间内的转发，但是map不是线程安全的，我们自己用的时候需要加锁,避免移除元素的时候出现问题.
   // MyMap<int,UserInfo*>m_mapIdToUserInfo;

    pthread_mutex_t m_lock2;
     pthread_mutex_t *c;
public:
     RedisTool *m_redis;
     map<int,FileInfo*>m_mapFileIDToFileInfo;

};

#endif
