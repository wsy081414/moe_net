/*
EventLoop 类是主要处理事件的类。
该类将 Channel 和 EpollPoller 链接在一起。所有的 Channel 都必须附属于一个 EventLoop 
而 EventLoop 将 Channel 又注册到 EpollPoller 中。

EventLoop 还会处理 TimerQueue ，在 TimerQueue 构造的时候也绑定了一个 EventLoop 
TimerQueue 有一个 timerfd ，该fd封装在一个 Channel 中，在构造的时候 Channel 绑定了 TimerQueue 绑定的 EventLoop
被  EventLoop 一起注册到 EpollPoller 中。

EventLoop 还会处理临时添加进来的一些函数？或是说一些需要执行的任务。
通过 add_task 或是 add_task_in_queue 添加到 EventLoop 中的一个容器中，
EventLoop 内部有一个 eventfd ，用于在添加新的任务以后直接唤醒 EpollPoller
然后在 poll 唤醒中运行。
*/


#ifndef MOE_EVENTLOOP_H
#define MOE_EVENTLOOP_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/Timestamp.h>
#include <moe_net/base/Mutex.h>
#include <moe_net/base/Logger.h>
#include <moe_net/net/TimerQueue.h>
#include <moe_net/net/Timer.h>

#include <functional>
#include <vector>
#include <memory>

namespace moe
{
namespace net
{

class Channel;
class EpollPoller;

class EventLoop : aux::Noncopyable
{
  public:
    typedef std::function<void()> Func;
    typedef Timer::TimerCallBack TimerCallBack;

    EventLoop();
    ~EventLoop();

  private:
    typedef std::vector<Channel *> ChannelVector;

    ChannelVector mc_active_channels;

    bool mb_looping;
    bool mb_quited;
    bool mb_handling;
    const pid_t m_tid;

    int m_wakeup_fd;
    std::unique_ptr<Channel> mp_wakeup_channel;

    std::unique_ptr<EpollPoller> mp_poll;

    Timestamp m_poll_return;

    Mutex m_mutex;

    const int s_poll_time;

    TimerQueue m_timer_queue;
    std::vector<Func> mc_tasks;
    bool m_handle_tasks;
    void handle_tasks();

    void wakeup_channel_handle_read();
    void add_in_queue(const Func &);
    void update(Channel *);
    void remove(Channel *);

  public:
    void loop();
    void quit();
    void wakeup();

    Timestamp poll_return();

    bool is_in_loop_thread(bool need = true);
    bool has_channel(Channel *);

    friend class Channel;
    friend class EpollPoller;

    int64_t add_timer(const TimerCallBack &, int64_t when, bool repeat = false);
    void cancel_timer(int64_t);
    void add_task(const Func &);
    void add_task_in_queue(const Func &task) { add_in_queue(task); };
};
}
}

#endif //MOE_EVENTLOOP_H