#include <moe_net/base/Everythread.h>
#include <moe_net/base/String.h>
#include <moe_net/base/Thread.h>
#include <moe_net/base/Logger.h>
#include <moe_net/base/ThreadPool.h>

#include <stdio.h>

using namespace moe;

void func()
{
    printf("thread run :  %d \n",everythread::tid());
}

int main()
{
    Thread t(func,"test thread");
    t.start();
    Logger::level(Logger::TRACE);
    TRACELOG<<"haha";
    ThreadPool tp;
    tp.start(4);
    for(int i=0;i<10;i++)
    {
        tp.add_task(func);
    }
}

