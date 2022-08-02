#include<TCPKernel.h>
#include "packdef.h"
#include<stdio.h>
#include<sys/time.h>

using namespace std;

TcpNet* TcpKernel::m_tcp2= NULL ;
#define NetPackMap(a)  TcpKernel::GetInstance()->m_NetPackMap[ a - DEF_PACK_BASE ]
//设置网络协议映射

void TcpKernel::setNetPackMap()
{
    //清空映射
    bzero( m_NetPackMap , sizeof(m_NetPackMap) );
    //协议映射赋值
    NetPackMap(DEF_PACK_REGISTER_RQ)    = &TcpKernel::RegisterRq;
    NetPackMap(DEF_PACK_LOGIN_RQ)       = &TcpKernel::LoginRq;
    NetPackMap(DEF_PACK_UPLOAD_RQ)    = &TcpKernel::Upladrq;
    NetPackMap(DEF_PACK_FILEBLOCK_RQ)       = &TcpKernel::blockdownrq;
    NetPackMap(DEF_PACK_GetRandrq)    =&TcpKernel::getrandnrq;
     NetPackMap(DEF_PACK_GetHotrq)     =&TcpKernel::getrandnhot;
       NetPackMap(DEF_Hotrs)     =&TcpKernel::hotgo;




}

TcpKernel::TcpKernel()
{
    pthread_mutex_init(&m_lock2,NULL);
    myinfor=new Myinfor;


    myu *arc=new myu;
    arc->my_infor=myinfor;
    c=&m_lock2;
    arc->arr=&c;

   // pthread_create(&alive_tid,NULL, alive,(void*)arc);
}

TcpKernel::~TcpKernel()
{
     if(myinfor)
         delete myinfor;
     myinfor=NULL;
     pthread_cancel(alive_tid);
     if(m_sql)
     {
         delete m_sql;
         m_sql=NULL;
     }

     if(m_redis)
     {
         delete m_redis;
         m_redis=NULL;
     }
}


TcpKernel *TcpKernel::GetInstance()
{
    static TcpKernel kernel;
    return &kernel;
}

int TcpKernel::Open( int port)
{
    initRand();
    setNetPackMap();
    m_sql = new CMysql;
    m_redis= new RedisTool;

    
    if(  !m_sql->ConnectMysql("localhost","root","123456","wechat")  )
    {
        printf("Conncet Mysql Failed...\n");
        return FALSE;
    }
    else
    {
        printf("MySql Connect Success...\n");
    }
    //初始网络

    m_tcp = new TcpNet(this);

    bool res = m_tcp->Initnetwork() ;
     if(res!=TRUE)
     {
         printf("false\n");
     }
    m_tcp2=m_tcp;
    if( !res )
        err_str( "net init fail:" ,-1);

    return TRUE;
}

void TcpKernel::Close()
{
    m_sql->DisConnect();

}

//随机数初始化
void TcpKernel::initRand()
{
    struct timeval time;
    gettimeofday( &time , NULL);
    srand( time.tv_sec + time.tv_usec );
}

void TcpKernel::DealData(int clientfd,char *szbuf,int nlen)
{
    PackType type = *(PackType*)szbuf;
    if( (type >= DEF_PACK_BASE) && ( type < DEF_PACK_BASE + DEF_PACK_COUNT) )
    {
        PFUN pf = NetPackMap( type );
        if( pf )
        {
            (TcpKernel::GetInstance()->*pf)( clientfd , szbuf , nlen);
        }
    }

    return;
}





void TcpKernel::sendData2(int clientfd , char *szbuf, int nlen)
{
    m_tcp2->SendData(clientfd , szbuf ,nlen );
}

void TcpKernel::SendData(int clientfd, char *szbuf, int nlen)
{
    m_tcp->SendData(clientfd,szbuf,nlen);
}

void TcpKernel::loop()
{
    m_tcp->Epoll_loop(NULL);
}


//注册
void TcpKernel::RegisterRq(int clientfd,char* szbuf,int nlen)//注册到来的回复 主要是看有没有重名重复id的
{
    printf("clientfd:%d RegisterRq\n", clientfd);
        //1.拆包
        STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ *)szbuf;
        STRU_REGISTER_RS rs;
        //获取tel password  name
        string c;
        string na=rq->m_name;
        string key="name";
        if(m_redis->isExist(rq->m_tel))
        {
                  rs.m_lResult = userid_is_exist;
                  m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
                  return ;
        }
        else if(m_redis->getHashVal(key,na,c))
        {
                  rs.m_lResult = name_is_exist;
                  m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
                  return ;
        }
        else
        {
        //查表 t_user 根据tel 查tel
        char sqlStr[1024]={0}/*""*/;
        sprintf( sqlStr , "select tel from t_user where tel = '%s';",rq->m_tel );
        list<string> resList;
        if( !m_sql->SelectMysql(sqlStr , 1, resList ))
        {
              printf("select wrong\n");
              return ;
        }


        //有 user存在 返回
        if( resList.size() > 0){
            rs.m_lResult = userid_is_exist;
        }else
        {
        //没有 查表 t_user 根据name myevent_s查name  name有没有
            char sqlStr[1024]={0}/*""*/;
            resList.clear();
            sprintf( sqlStr , "select name from t_user where name = '%s';",rq->m_name );

            if( !m_sql->SelectMysql(sqlStr , 1, resList ))
            {
                printf("SelectMysql error: %s \n", sqlStr);
                return;
            }
            if( resList.size() > 0 )
            {   //有 name存在 返回
                rs.m_lResult = name_is_exist;
            }else{
                //没有 写表 tel pass  name 头像和签名的默认值  返回注册成功
                rs.m_lResult = register_sucess;
                sprintf( sqlStr , "insert into t_user( tel, password , name , icon , feeling) values('%s','%s','%s',%d ,'%s');"
                         ,rq->m_tel,rq->m_Password , rq->m_name , 1 , "比较懒,什么也没写" );
                string tel=rq->m_tel;
                 m_redis->setHashVal("everyone",tel,"1");
                 string pssword=rq->m_Password;
                 string name=rq->m_name;
                 vector<string>myve;
                 string c="password";
                 string k="name";
                 myve.push_back(c);
                 myve.push_back(k);
                 vector<string>myvec;
                 myvec.push_back(pssword);
                 myvec.push_back(name);
                 m_redis->setHashVals(tel,myve,myvec);
                 m_redis->setHashVal("name",name,"1");
                if( !m_sql->UpdataMysql(sqlStr ))
                {
                    printf("UpdataMysql error: %s \n", sqlStr);
                }
            }
        }
        m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
        }


}




void *TcpKernel::alive(void *arg)//心跳
{
        myu *arc=(myu*)arg;
        Myinfor *myin=arc->my_infor;
        pthread_mutex_t *m_lock2= *arc->arr;
        while(1)
        {
            pthread_mutex_lock(m_lock2);
            int c=myin->myvec.size();
             pthread_mutex_unlock(m_lock2);

            for(int i=0;i<c;i++)
            {

                time_t c;
                time(&c);
                pthread_mutex_lock(m_lock2);
                if((c-myin->myvec[i].time)>4)
                {

                     int id=myin->myvec[i].id;
                    if(myin->m_mapIdToUserInfo.IsExist(myin->myvec[i].id))
                    {

                      UserInfo *user=myin->m_mapIdToUserInfo.find(id);
                      printf("fd  is timeout\n");
                      if(user->m_sockfd!=0)
                      {
                        //close(user->m_sockfd );
                        printf("sockt quit1\n");
                        user->m_sockfd=0;
                      }
                      if(user->m_audiofd!=0)
                      {
                           // close(user->m_audiofd );
                         printf("audio quit2\n");
                          user->m_audiofd=0;
                      }
                      if(user->m_videofd!=0)
                     {
                           // close(user->m_videofd );
                        printf("vidio quit3\n");
                        user->m_videofd=0;
                      }
                      myin->m_mapIdToUserInfo.erase(id);

                    }

                    myin->myvec.erase(myin->myvec.begin()+i);

                    for(int k=0;k<myin->myvec.size();k++)
                    {
                        int ac=myin->myvec[k].id;
                       UserInfo*user=myin->m_mapIdToUserInfo.find(ac);
                       user->myvec=k;
                    }


                }
                pthread_mutex_unlock(m_lock2);


            }

            usleep(1000000);
        }

        pthread_exit((void*)1);
}





void TcpKernel::LoginRq(int  clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);
    STRU_LOGIN_RQ* loginrq= (STRU_LOGIN_RQ*)szbuf;
    printf("%s\n",loginrq->m_tel);
    if(m_redis->isExist(loginrq->m_tel))
    {
        string c;
        m_redis->getHashVal(loginrq->m_tel,"password",c);
        string k=loginrq->m_szPassword;
        if(k==c)
        {
               char tel[30];
               int wo=atoi(loginrq->m_tel);
               sprintf(tel,"%d",wo);
               m_redis->setHashVal("everyone",tel,"1");
             STRU_LOGIN_RS rs;
               rs.m_lResult=login_sucess;
               rs.m_userID=atoi(loginrq->m_tel);
               printf("%d\n",rs.m_userID);
               string cc;
                 m_redis->getHashVal(loginrq->m_tel,"name",cc);
                 int id=atoi(loginrq->m_tel);
                 if(myinfor->m_mapIdToUserInfo.IsExist(id))//用户存在
                 {
                      rs.m_lResult=10;//强制下线
                         UserInfo *Info=myinfor->m_mapIdToUserInfo.find(id);
                          SendData( Info->m_sockfd , (char*)&rs , sizeof rs );
                         Info->m_sockfd=clientfd;
                         return ;
                 }
                 strcpy(rs.m_szName,cc.c_str());
                 UserInfo *pInfo=new  UserInfo;
                 pInfo->m_id=id;
                 pInfo->m_sockfd=clientfd;
                 pInfo->m_roomid=0;
                 pthread_mutex_lock(&m_lock2);
                 pInfo->myvec=myinfor->myvec.size();
                  pthread_mutex_unlock(&m_lock2);
                 strcpy(pInfo->m_userName,cc.c_str());
                 Myark c;
                 c.id=pInfo->m_id;
                 time(&c.time);
                  pthread_mutex_lock(&m_lock2);
                 myinfor->myvec.push_back(c);
                 pthread_mutex_unlock(&m_lock2);
                 myinfor->m_mapIdToUserInfo.insert( pInfo->m_id , pInfo );

                 strcpy(rs.m_szName,pInfo->m_userName);

               printf("redis\n");
                SendData( clientfd , (char*)&rs , sizeof rs );
                printf("youyou\n");
                while(1)
                {

                }
                return ;
        }
        else
        {
            STRU_LOGIN_RS rs;
              rs.m_lResult=password_error;
              printf("redis\n");
               SendData( clientfd , (char*)&rs , sizeof rs );
               return ;
        }


    }
    char selbuf[1024]={0};
    sprintf(selbuf,"select password,tel,name from t_user where tel= '%s';",loginrq->m_tel);
    list<string>rsstr;
    if(!m_sql->SelectMysql(selbuf,3,rsstr))
    {
        printf("loginrs select name password wrong\n");

    }
     STRU_LOGIN_RS rs;
    if(rsstr.size()<=0)
    {
        rs.m_lResult=userid_no_exist;


    }
    else
    {

    if(strcmp(loginrq->m_szPassword,rsstr.front().c_str())!=0)
    {
        rs.m_lResult=password_error;
    }
    else
    {

         //printf("comeon\n");
        string ii="password";
        string k="name";
        vector<string>p;
        p.push_back(ii);
        p.push_back(k);
          vector<string>pp;
          pp.push_back(rsstr.front());


        rsstr.pop_front();
        rs.m_lResult=login_sucess;
        int id=atoi(rsstr.front().c_str());
        rs.m_userID=id;
        rsstr.pop_front();
        pp.push_back(rsstr.front());
        UserInfo *pInfo=new  UserInfo;
        pInfo->m_id=id;
        pInfo->m_sockfd=clientfd;
        pInfo->m_roomid=0;
        pthread_mutex_lock(&m_lock2);
        pInfo->myvec=myinfor->myvec.size();
         pthread_mutex_unlock(&m_lock2);
        strcpy(pInfo->m_userName,rsstr.front().c_str());
        if(myinfor->m_mapIdToUserInfo.IsExist(pInfo->m_id))//用户存在
        {
            //强制下线
        }


        Myark c;
        c.id=pInfo->m_id;
        time(&c.time);
         pthread_mutex_lock(&m_lock2);
        myinfor->myvec.push_back(c);
        pthread_mutex_unlock(&m_lock2);
        myinfor->m_mapIdToUserInfo.insert( pInfo->m_id , pInfo );

        strcpy(rs.m_szName,pInfo->m_userName);
          m_redis->setHashVals(loginrq->m_tel,p,pp);

    }


    }
    SendData( clientfd , (char*)&rs , sizeof rs );

}
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"unistd.h"
void TcpKernel::Upladrq(int clientfd, char *szbuf, int nlen)
{
    printf("load \n");
    STRU_UPLOAD_RQ * rq = (STRU_UPLOAD_RQ *)szbuf;

       FileInfo * info = new FileInfo;
       info->m_nPos = 0;


       info->m_nUserId = rq->m_UserId;
       info->m_nFileID = rq->m_nFileId;
       info->m_VideoID = 0;

       info->m_nFileSize=rq->m_nFileSize;

       strcpy( info->m_szFileName , rq->m_szFileName );
       char sqlstr[_DEF_SQLIEN ] ="";
       sprintf(sqlstr , "select name from t_user where tel = '%d';" ,  info->m_nUserId );
       list<string> resList;
       if( !m_sql->SelectMysql(sqlstr, 1, resList) )
       {
           cout<< "SelectMysql errot" << sqlstr <<endl;
           delete info;
           return;
       }
       if( resList .size() <= 0 )
       {
           delete info;
           return;
       }

       strcpy ( info->m_UserName   ,resList.front().c_str() );
       sprintf( info->m_szFilePath , "%s/%s%s"  , Path , info->m_UserName , info->m_szFileName) ;
       sprintf ( info->m_szRtmp , "rtmp://192.168.0.103:1935/vod//%s%s" ,info->m_UserName , info->m_szFileName );

       info->File = open( info->m_szFilePath , O_WRONLY|O_CREAT,0664);
       perror("error:");
       printf("file%d\n",info->File);
       truncate(info->m_szFilePath,  info->m_nFileSize);

       m_mapFileIDToFileInfo[ info->m_nFileID ] =  info;
}
/*create table t_music ( id bigint unsigned  AUTO_INCREMENT primary key, tel varchar (260) ,
   rtmpaddress varchar(260)  , hot int, label varchar(260));*/
void TcpKernel::blockdownrq(int clientfd, char *szbuf, int nlen)
{
    printf("block come in\n");
       STRU_FILEBLOCK_RQ *rq =(STRU_FILEBLOCK_RQ *)szbuf;
       if( m_mapFileIDToFileInfo .find ( rq->m_FileId ) == m_mapFileIDToFileInfo.end() ) return;

       FileInfo* info  = m_mapFileIDToFileInfo [ rq->m_FileId];

       int64_t res =write( info->File,rq->m_FileContent , rq->m_BlockLen  );
       info->m_nPos += res;


       if(  rq->m_BlockLen < MAX_CONTENT_LEN || info->m_nPos >= info->m_nFileSize  )
       {

               close( info->File);
               char sz[500]=" ";
               char ack[30]=" ";

               sprintf(ack,"%d",info->m_nUserId);
               sprintf(sz,"insert into t_music( tel, rtmpaddress , hot , label) values('%s','%s',%d ,'%s');"
                      ,ack,info->m_szRtmp,0," " );
               if( !m_sql->UpdataMysql(sz ))
               {
                   printf("UpdataMysql error: %s \n", sz);
                   return;
               }
               string rtmpaddres(info->m_szRtmp);
               string name(info->m_szFileName);
               string c="rtmp";
               string c2="rtmp2";
               m_redis->setHashVal(c,name,rtmpaddres);
               m_redis->setzseVal(c2,1,name);
               STRU_FILEBLOCK_RS rs;
               rs.m_result = 1;
               m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs));
       }

           m_mapFileIDToFileInfo.erase( rq->m_FileId );
           delete info;
           info = NULL;

}

void TcpKernel::getrandnrq(int clientfd, char *szbuf, int nlen)
{
    STRU_DEF_PACK_GetRandrs rs;
    string key="rtmp";
    vector<string>name(3," ");
    vector<string>stm(3," ");
    m_redis->heys(key,name,stm);
    for(int i=0;i<3;i++)
    {
         strcpy(rs.arr[i],name[i].c_str());
        strcpy(rs.ark[i],stm[i].c_str());
    }
    printf("this is %s\n",rs.arr[0]);
    printf("this is %s\n",rs.ark[0]);
    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs));
}

void TcpKernel::getrandnhot(int clientfd, char *szbuf, int nlen)
{
    STRU_DEF_PACK_GetHotrs rs;
    string key="rtmp2";
    vector<string>name(3," ");
    vector<string>stm(3," ");
    m_redis->zrange(key,name,stm);
    for(int i=0;i<3;i++)
    {
         strcpy(rs.arr[i],name[i].c_str());
        strcpy(rs.ark[i],stm[i].c_str());
    }
    printf("this is %s\n",rs.arr[0]);
    printf("this is %s\n",rs.ark[0]);
    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs));
}

void TcpKernel::hotgo(int clientfd, char *szbuf, int nlen)
{
    STRU_Hot_rs *rq=(STRU_Hot_rs*)szbuf;
    printf("%s\n",rq->ark);
    m_redis->zset(rq->ark);
}



