/*
EventLoopThread 类，使用 Thread 类，使得 EventLoop 能在一个另一个线程中运行
并且返回了 EventLoop 的指针。

也就是说 Thread 类要运行的函数中建立了一个 EventLoop ，返回 EventLoop 的指针，然后开始 loop

为了避免在 start 中，在 mp_loop 还为空的时候就返回，所以添加了条件变量来进行限制。
因为 Thread 中建立 EventLoop 以后还可能要有对 EventLoop 进行设置的一个回调函数，因此需要一些执行时间，
所以加了一个 条件变量。

*/

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
    typedef std::function<void (EventLoop*)> InitFunc;

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