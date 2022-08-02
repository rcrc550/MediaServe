#include <TCPKernel.h>
#include<pthread.h>

int main(int argc,char *argv[])
{

    int port = 8000;
    if( argc >= 2 )
    {
        port = atoi(argv[1]);
    }
    TcpKernel * pKernel =  TcpKernel::GetInstance();

    //开启服务 给定端口, 可以使用输入的port
    pKernel->Open( port);
     //pKernel->m_redis->zset("rtmp2","123 ");
    pKernel->m_redis->zset("123");
//    char sz[500]=" ";
//    char ack[30]=" ";

//    sprintf(ack,"%d",0);
//    sprintf(sz,"insert into t_music( tel, rtmpaddress , hot , label) values('%s','%s',%d ,'%s');"
//           ,ack,"rtmp://192.168.0.103:1935/vod//123.mp4",0," " );
//    if( !pKernel->m_sql->UpdataMysql(sz ))
//    {
//        printf("UpdataMysql error: %s \n", sz);

//    }
//    sprintf(sz,"insert into t_music( tel, rtmpaddress , hot , label) values('%s','%s',%d ,'%s');"
//           ,ack,"rtmp://192.168.0.103:1935/vod//456.mp4",1," " );
//    sprintf(sz,"insert into t_music( tel, rtmpaddress , hot , label) values('%s','%s',%d ,'%s');"
//           ,ack,"rtmp://192.168.0.103:1935/vod//789.mp4",1," " );
//    string rtmpaddres("rtmp://192.168.0.103:1935/vod//123.mp4");
//    string name("123");
//    pKernel->m_redis->setHashVal("rtmp",name,rtmpaddres);
//    pKernel->m_redis->setzseVal("rtmp2",1,name);
//    string rtmpaddres2("rtmp://192.168.0.103:1935/vod//456.mp4");
//    string name2("456");
//    pKernel->m_redis->setHashVal("rtmp",name2,rtmpaddres2);
//     pKernel->m_redis->setzseVal("rtmp2",1,name2);
//    string rtmpaddres3("rtmp://192.168.0.103:1935/vod//789.mp4");
//    string name3("789");
//    pKernel->m_redis->setHashVal("rtmp",name3,rtmpaddres3);
//    pKernel->m_redis->setzseVal("rtmp2",1,name3);
    pKernel->loop();
//    while(1)
//    {
//        printf("serve running\n");
//        sleep(3);
//    }

    pKernel->Close();

    return 0;
}
