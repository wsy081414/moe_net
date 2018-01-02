#ifndef MOE_EVENTLOOPTHREAD_H
#define MOE_EVENTLOOPTHREAD_H


#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/Thread.h>
#include <moe_net/base/Mutex.h>
#include <moe_net/base/Condition.h>


#include <functional>

namespace moe 
{
namespace net 
{

class EventLoop;

class EventLoopThread : aux::Noncopyable
{
public:
    typedef std::function<void ()> InitFunc;

    EventLoopThread(const InitFunc& cb=InitFunc(),const String & name="EventLoopThread");
    ~EventLoopThread();

    EventLoop *start();

private:
    void send_into_thread();
    Thread m_thread;

    EventLoop *mp_loop;
    bool mb_quit;
    Mutex m_mutex;
    Condition m_cond;
    InitFunc init_cb;

};



}
}



#endif //MOE_EVENTLOOPTHREAD_H