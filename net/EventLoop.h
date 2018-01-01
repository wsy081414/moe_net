#ifndef MOE_EVENTLOOP_H
#define MOE_EVENTLOOP_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/Timestamp.h>

#include <functional>
#include <vector>
#include <memory>

namespace moe 
{
namespace net 
{


class Channel;


class EventLoop : aux::Noncopyable
{
public:
    typedef std::function<void ()> Func;
private:
    typedef std::vector<Channel *> ChannelVector;

    ChannelVector mc_active_channels;

    bool mb_looping;
    bool mb_quited;
    bool mb_handling;
    const pid_t m_tid;
    std::unique_ptr<Channel> mp_wakeup_channel;
    std::unique_ptr<Channel> mp_poll;

    Timestamp m_poll_return;

    Mutex m_mutex;
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp poll_return();

    bool is_in_loop_thread();

};
}
}



#endif //MOE_EVENTLOOP_H