#ifndef _PACKDEF_H
#define _PACKDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "err_str.h"
#include <malloc.h>
#include"tcpnet.h"
#include<iostream>
#include<map>
#include<list>
#include"redistool.h"
//#include<qtime>
class TcpNet;
typedef struct mythis
{
    TcpNet *pthis;
    int clientfd;
}mythis;
//边界值
#define _DEF_SIZE 45
#define _DEF_BUFFERSIZE 1000
#define DEF_PORT 8000
#define DEF_SERVERIP "127.0.0.1"  //TODO

#define WOLf_NUM 10
#define TRUE true
#define FALSE false


#define _DEF_LISTEN 128
#define _DEF_EPOLLSIZE 4096
#define _DEF_IPSIZE 16
#define _DEF_COUNT 10
#define _DEF_TIMEOUT 10
#define _DEF_SQLIEN 400



#define BOOL bool
#define DEF_PACK_BASE  (10000)
#define DEF_PACK_COUNT  (100)//注册
#define  DEF_PACK_REGISTER_RQ    (DEF_PACK_BASE + 0)
#define  DEF_PACK_REGISTER_RS    (DEF_PACK_BASE + 1)//登录
#define  DEF_PACK_LOGIN_RQ    (DEF_PACK_BASE + 2)
#define  DEF_PACK_LOGIN_RS    (DEF_PACK_BASE + 3)//创建房间
#define DEF_PACK_UPLOAD_RQ  (DEF_PACK_BASE + 4)
#define DEF_PACK_UPLOAD_RS  (DEF_PACK_BASE + 5)//加入房间
#define DEF_PACK_FILEBLOCK_RQ   (DEF_PACK_BASE + 6)
#define DEF_PACK_FILEBLOCK_RS   (DEF_PACK_BASE + 7)
#define DEF_PACK_GetRandrq  (DEF_PACK_BASE + 8)
#define DEF_PACK_GetRandrs (DEF_PACK_BASE + 9)
#define DEF_PACK_GetHotrq  (DEF_PACK_BASE + 10)
#define DEF_PACK_GetHotrs  (DEF_PACK_BASE + 11)
#define DEF_Hotrs  (DEF_PACK_BASE + 12)
#define Path   "/home/colin/videoo/flv/"


#define userid_is_exist      0
#define register_sucess      1
#define name_is_exist        2

//登录请求结果
#define userid_no_exist      0
#define password_error       1
#define login_sucess         2
//#define user_online          3

//创建房间结果
#define room_is_exist        0
#define create_success       1

//加入房间结果
#define room_no_exist        0
#define join_success         1
typedef struct AFILEINFO
{
public:
    AFILEINFO():m_nFileID(0),m_VideoID(0),m_nFileSize(0),m_nPos(0),
        m_nUserId(0),File(0)
    {
        memset(m_szFilePath, 0 , 300);
        memset(m_szFileName, 0 , 300);

        memset(m_UserName, 0 , 300);
        memset(m_szRtmp ,0 , 300);
    }
    int m_nFileID;//下載的時候是用來做UI控件編號的， 上傳的時候是一個隨機數， 區分文件。
    int m_VideoID;//真是文件ID 與Mysql的一致
    int64_t m_nFileSize;
    int64_t m_nPos;
    int m_nUserId;
    int File;
    char m_szFileName[300];
    char m_szFilePath[300];
    char m_UserName[300];
    char m_szRtmp[300];

}FileInfo;
template<class K , class V>
struct MyMap
{
public:
    MyMap(){
        pthread_mutex_init(&m_lock , NULL);
    }
    map<K , V> m_map;
    //获取的结果如果找不到就是对象v 如果是指针应该是NULL 规定该函数使用时，先判断他有没有,确保一定有。
    V find( K k)
    {

        pthread_mutex_lock(&m_lock );

              V v= m_map[k];
            pthread_mutex_unlock(&m_lock );
            return v;



    }
    V get(K k)
    {
        pthread_mutex_lock(&m_lock );
           V c=m_map[k];
        pthread_mutex_unlock(&m_lock );
        return c;
    }
    void add(K k)//只能整形用
    {

        pthread_mutex_lock(&m_lock );
        if(m_map[k]<WOLf_NUM)
           m_map[k]=m_map[k]+1;
        pthread_mutex_unlock(&m_lock );

    }
    void ded(K k)//只能整形用
    {
        pthread_mutex_lock(&m_lock );
        if(m_map[k]>0)
           m_map[k]=m_map[k]-1;
        pthread_mutex_unlock(&m_lock );
    }
    void insert( K k , V v)
    {
        pthread_mutex_lock(&m_lock );
        m_map[k] = v;
        pthread_mutex_unlock(&m_lock );
    }
    void erase(K k )
    {
        pthread_mutex_lock(&m_lock );
        if(m_map.count(k)>0)
        m_map.erase(k);
        pthread_mutex_unlock(&m_lock );
    }
    bool IsExist( K k )
    {
        bool flag = false;
        pthread_mutex_lock(&m_lock );
        if( m_map.count(k) > 0 )
            flag = true;
        pthread_mutex_unlock(&m_lock );
        return flag;
    }
private:
    pthread_mutex_t m_lock;
};

#define MAX_PATH            (260 )
#define MAX_SIZE            (60  )
#define DEF_HOBBY_COUNT     (8  )
#define MAX_CONTENT_LEN     (4096 )


/////////////////////网络//////////////////////////////////////


#define DEF_MAX_BUF	  1024
#define DEF_BUFF_SIZE	  4096


typedef int PackType;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct STRU_LOGIN_RQ
{
    STRU_LOGIN_RQ()
    {
        m_nType = DEF_PACK_LOGIN_RQ;
        memset(m_tel,0,MAX_SIZE);
        memset(m_szPassword,0,MAX_SIZE);
    }

    PackType m_nType;   //包类型
    char     m_tel[MAX_SIZE] ; //用户ID
    char     m_szPassword[MAX_SIZE];  //密码
} STRU_LOGIN_RQ;




//登录回复
typedef struct STRU_LOGIN_RS
{
    STRU_LOGIN_RS()
    {
        m_nType= DEF_PACK_LOGIN_RS;
        m_userID = 0;
        m_lResult = 0;
         memset(m_szName,0,MAX_SIZE);
    }
    PackType m_nType;   //包类型
    int  m_userID;
    int  m_lResult ; //注册结果
    char m_szName[MAX_SIZE];
} STRU_LOGIN_RS;
//注册请求
typedef struct STRU_REGISTER_RQ
{
    STRU_REGISTER_RQ()
    {
        m_nType = DEF_PACK_REGISTER_RQ;
        memset(m_tel,0,MAX_SIZE);
        memset(m_Password,0,MAX_SIZE);
        memset(m_name,0,MAX_SIZE);
    }

    PackType m_nType;   //包类型
    char     m_tel[MAX_SIZE] ; //用户名
    char     m_Password[MAX_SIZE];  //密码
    char      m_name[MAX_SIZE];

} STRU_REGISTER_RQ;

//注册回复
typedef struct STRU_REGISTER_RS
{
    STRU_REGISTER_RS()
    {
        m_nType= DEF_PACK_REGISTER_RS;
        m_lResult = 0;
    }
    PackType m_nType;   //包类型
    int  m_lResult ; //注册结果

} STRU_REGISTER_RS;
typedef struct STRU_UPLOAD_RQ
{
    STRU_UPLOAD_RQ()
    {
        m_nType = DEF_PACK_UPLOAD_RQ;
        m_nFileId = 0;
        m_UserId = 0;
        m_nFileSize = 0;

        memset(m_szFileName , 0 ,MAX_PATH);

    }
    PackType m_nType; //包类型
    int m_UserId; //用于查数据库, 获取用户名字, 拼接路径
    int m_nFileId; //区分不同文件
    int64_t m_nFileSize; //文件大小, 用于文件传输结束
    char m_szFileName[MAX_PATH]; //文件名, 用于存储文件

}STRU_UPLOAD_RQ;

//上传文件请求回复
typedef struct STRU_UPLOAD_RS
{
    STRU_UPLOAD_RS()
    {
        m_nType = DEF_PACK_UPLOAD_RS;
        m_nResult = 0;
    }
    PackType m_nType; //包类型
    int m_nResult;
}STRU_UPLOAD_RS;

typedef struct STRU_FILEBLOCK_RQ
{
    STRU_FILEBLOCK_RQ()
    {
        m_nType = DEF_PACK_FILEBLOCK_RQ;
        m_UserId = 0;
        m_FileId =0;
        m_BlockLen =0;
        memset(m_FileContent,0,MAX_CONTENT_LEN);
    }
    PackType m_nType; //包类型
    int m_UserId; //用户 ID
    int m_FileId; //文件 id 用于区分文件
    int m_BlockLen; //文件写入大小
    char m_FileContent[MAX_CONTENT_LEN];
}STRU_FILEBLOCK_RQ;

typedef struct STRU_FILEBLOCK_RS
{
    STRU_FILEBLOCK_RS()
    {
        m_nType = DEF_PACK_FILEBLOCK_RS;
        m_result=0;

    }
    PackType m_nType; //包类型
    int m_result;

}STRU_FILEBLOCK_RS;
typedef struct STRU_DEF_PACK_GetRandrq
{
    STRU_DEF_PACK_GetRandrq()
    {
        m_nType = DEF_PACK_GetRandrq;


    }
    PackType m_nType; //包类型


}STRU_DEF_PACK_GetRandrq;
typedef struct STRU_DEF_PACK_GetRandrs
{
    STRU_DEF_PACK_GetRandrs()
    {
        m_nType = DEF_PACK_GetRandrs;
        memset(arr,0,sizeof(arr));
        memset(ark,0,sizeof(ark));

    }
    PackType m_nType; //包类型
    char arr[3][40];
    char ark[3][40];

}STRU_DEF_PACK_GetRandrs;

typedef struct STRU_DEF_PACK_GetHotrq
{
    STRU_DEF_PACK_GetHotrq()
    {
        m_nType = DEF_PACK_GetHotrq;
   }
    PackType m_nType; //包类型


}STRU_DEF_PACK_GetHotrq;
typedef struct STRU_DEF_PACK_GetHotrs
{
    STRU_DEF_PACK_GetHotrs()
    {
        m_nType = DEF_PACK_GetHotrs;
        memset(arr,0,sizeof(arr));
        memset(ark,0,sizeof(ark));
   }
    PackType m_nType; //包类型
    char arr[3][40];
    char ark[3][40];

}STRU_DEF_PACK_GetHotrs;
typedef struct UserInfo
{
    UserInfo()
    {
         m_sockfd = 0;
         m_id = 0;
         m_roomid = 0;
         memset(m_userName, 0 , MAX_SIZE);
         m_videofd = 0;
         m_audiofd = 0;
         m_wolfid=0;
         ready=0;
         myvec=0;
    }
  int m_sockfd;
    int  m_id;
    int  m_roomid;
    char m_userName[MAX_SIZE];
    int m_wolfid;
    int m_videofd;

    int m_audiofd;
    int myvec;
    int ready;

}UserInfo;

typedef struct STRU_Hot_rs
{
    STRU_Hot_rs()
    {
        m_nType = DEF_Hotrs;

        memset(ark,0,sizeof(ark));
   }
    PackType m_nType; //包类型

    char ark[10];

}STRU_Hot_rs;



#endif



