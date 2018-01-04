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
    typedef std::function<void ()> Func;
    typedef Timer::TimerCallBack TimerCallBack;
private:
    typedef std::vector<Channel *> ChannelVector;

    ChannelVector mc_active_channels;

    bool mb_looping;
    bool mb_quited;
    bool mb_handling;
    const pid_t m_tid;
    std::unique_ptr<Channel> mp_wakeup_channel;
    std::unique_ptr<EpollPoller> mp_poll;

    Timestamp m_poll_return;

    Mutex m_mutex;

    int m_wakeup_fd;
    Channel *m_wakeup_channel;
    const int s_poll_time;

    TimerQueue m_timer_queue;

    void wakeup_channel_handle_read();
    void add_in_queue(const Func&);
    void update(Channel *);
    void remove(Channel *);
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void wakeup();

    Timestamp poll_return();

    bool is_in_loop_thread();
    bool has_channel(Channel *);

    
    friend class Channel;
    friend class EpollPoller;
    
    int64_t add_timer(const TimerCallBack& ,int64_t when,bool repeat=false );
};
}
}



#endif //MOE_EVENTLOOP_H