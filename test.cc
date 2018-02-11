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

void func2(const TcpConnectionPtr& tcp_conn)
{
    tcp_conn->connect_destroy();
    sleep(1);
}

int main()
{
    RingBuffer a;
    Thread t(func,"test thread");
    // t.start();
    Logger::level(Logger::TRACE);
    // TRACELOG<<"haha";
    // ThreadPool tp;
    // tp.start(4);
    // for(int i=0;i<10;i++)
    // {
    //     // tp.add_task(func);
    // }



    EventLoopThread loop;
    // TRACELOG<<loop.is_in_loop_thread();
    EventLoop *loop_ptr=loop.start();
    

    SockAddr serv_addr("127.0.0.1",7777);

    TcpClient client(loop_ptr,serv_addr,"client 127.0.0.1");

    // client.set_close_cb(func2);
    
    client.connect();

    // EventLoop loop_serv;
    // SockAddr listen_addr("127.0.0.1",7777);

    // TcpServer serv(&loop_serv,"server",listen_addr,1);
    // serv.set_threads(4);

    // serv.start();

    sleep(100);
}

