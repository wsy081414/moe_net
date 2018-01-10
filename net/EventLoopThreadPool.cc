#include <moe_net/net/EventLoopThreadPool.h>


using namespace moe;
using namespace moe::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop,const String& name)
    :mp_base_loop(loop),m_name(name),mb_started(false),m_thread_counts(0),m_next_loop(0)
{

}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!mb_started);
    assert(mp_base_loop->is_in_loop_thread());

    mb_started =true;

    for(int i= 0;i<m_thread_counts;++i)
    {   
        // name

        std::shared_ptr<EventLoopThread> t(new EventLoopThread(cb,String("name")));
        mc_threads.push_back(t);
        mc_loops.push_back(t->start());
    }
    if(m_thread_counts ==0 &&cb)
    {
        cb(mp_base_loop);
    }  
}

EventLoop *EventLoopThreadPool::next_loop()
{
    assert(mp_base_loop->is_in_loop_thread());

    assert(mb_started);

    EventLoop *loop=mp_base_loop;
    if(!mc_loops.empty())
    {
        loop=mc_loops[m_next_loop];
        ++m_next_loop;
    }    
    if(m_next_loop >= mc_loops.size())
    {
        m_next_loop=0;
    }
}

