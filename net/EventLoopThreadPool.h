/*
EventLoopThreadPool 类，在应用中是一个干活的类。
这个类不是 ThreadPool ， 这个类直接实例化了几个 EventLoop ，用来以后干活用的。
同时一个 next_loop() 简单的做负载均衡。

EventLoopThreadPool 仍然要绑定一个 base_loop 的原因在于， base_loop 中获取需要处理的套接字，然后根据 
next_loop() 分配给 EventLoopThreadPool 中的 EventLoop。

*/

#ifndef MOE_EVENTLOOPTHREADPOOL_H
#define MOE_EVENTLOOPTHREADPOOL_H


#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/Thread.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/EventLoopThread.h>
#include <moe_net/base/ThreadPool.h>


#include <memory>
#include <functional>
#include <map>
#include <vector>

namespace moe 
{
namespace net 
{
class EventLoop;
// class ThreadPool;
class EventLoopThread;

class EventLoopThreadPool :aux::Noncopyable
{
public:
  typedef std::function<void (EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* loop,const String& name);
    ~EventLoopThreadPool();

    void set_threads(int i) {m_thread_counts = i;}

    void start(const ThreadInitCallback& cb=ThreadInitCallback());

    EventLoop *base_loop(){ return mp_base_loop;}
    EventLoop *next_loop();

    // std::vector<EventLoop *>& all_loops() ;
    
    bool started() {return mb_started;}

    const String name() {return m_name;}
private:
    EventLoop *mp_base_loop;

    String m_name;

    int m_thread_counts;
    bool mb_started;
    size_t m_next_loop;

    std::vector<std::shared_ptr<EventLoopThread>> mc_threads;
    std::vector<EventLoop *> mc_loops;


};
}
}


#endif //MOE_EVENTLOOPTHREADPOOL_H