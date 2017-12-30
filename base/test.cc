#include <Everythread.h>
#include <String.h>
#include <Thread.h>
#include <Logger.h>

#include <stdio.h>

using namespace moe;

void func()
{
    printf("thread run\n");
}

int main()
{
    Thread t(func,"test thread");
    t.start();
    TRACELOG<<"haha";
}
