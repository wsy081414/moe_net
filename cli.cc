#include <moe_net/base/Everythread.h>
#include <moe_net/base/String.h>
#include <moe_net/base/Thread.h>
#include <moe_net/base/Logger.h>
#include <moe_net/base/ThreadPool.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/EventLoopThread.h>
#include <moe_net/net/TimerQueue.h>


#include <moe_net/net/EventLoopThreadPool.h>
#include <stdio.h>

#include <moe_net/net/TcpConnection.h>

#include <moe_net/net/SockAddr.h>
#include <moe_net/net/TcpClient.h>
#include <moe_net/net/RingBuffer.h>
#include <moe_net/net/TcpServer.h>




using namespace moe;
using namespace moe::net;


void func()
{
    printf("---------------------------------------thread run :  %d \n",everythread::tid());
    sleep(1);
}

// void func2(const TcpConnectionPtr& tcp_conn)
// {
//     tcp_conn->connect_destroy();
//     sleep(1);
// }

int main()
{
    RingBuffer a;
    Thread t(func,"test thread");
    Logger::level(Logger::TRACE);
    ThreadPool tp;




    EventLoopThread loop;
    EventLoop *loop_ptr=loop.start();
    

    SockAddr serv_addr("127.0.0.1",7777);

    TcpClient client(loop_ptr,serv_addr,"client 127.0.0.1");

    
    client.connect();

    TRACELOG<<"connect () return";
    TcpConnectionPtr tcp_ptr = client.connection(); 

    TRACELOG<<"connection () return "<<bool(tcp_ptr) ;



    while(true)
    {
        sleep(100);
    }
}

