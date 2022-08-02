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
    pKernel->loop();


    while(1)
    {
        printf("serve running\n");
        sleep(3);
    }
    pKernel->Close();

    return 0;
}
