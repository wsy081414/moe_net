#include <moe_net/net/EventLoopThread.h>
#include <moe_net/net/EventLoop.h>


using namespace moe;
using namespace moe::net;


EventLoopThread::EventLoopThread(const InitFunc& cb,const String & name)
    :mp_loop(nullptr),mb_quit(false),m_thread(std::bind(&EventLoopThread::send_into_thread,this),name),
    m_mutex(),m_cond(m_mutex),init_cb(cb)
{
    INFOLOG<<"EventLoopThread : "<<name<<" start";
}

EventLoopThread::~EventLoopThread()
{
    mb_quit=true;
    if(mp_loop != nullptr)
    {
        mp_loop->quit();
        m_thread.join();
    }
    INFOLOG<<"EventLoopThread : "<<name<<" destory";
}

EventLoop *EventLoopThread::start()
{
    assert(m_thread.tid() == 0);
    m_thread.start();
    {
        MutexLock lock(m_mutex);
        while(mp_loop == nullptr)
        {
            m_cond.wait();
        }
    }
    return mp_loop;
}

void EventLoopThread::send_into_thread()
{
    EventLoop loop;
    
    if(init_cb)
    {
        init_cb(&loop);
    }

    {
        MutexLock lock(m_mutex);
        mp_loop = &loop;
        m_cond.notify();
    }
    
    loop.loop();
    mp_loop=nullptr;
}