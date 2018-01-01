#ifndef MOE_EPOLLPOLLER_H
#define MOE_EPOLLPOLLER_H


#include <moe_net/base/Noncopyalbe.h>
#include <moe_net/base/Timestamp.h>

#include <map>

namespace moe
{
namespace net
{
namespace parts
{

using namespace moe;

class EventLoop;

class EpollPoller : moe::aux::Noncopyalbe
{
public:
    typedef std::vector<Channel *> ChannelVector;
    EpollPoller(EventLoop *);
    ~EventLoop();

    Timestamp poll(int,ChannelVector *);
    void update(Channel *);
    void remove(Channel *);

    bool has_channel(Channel *);
    bool is_in_loop_thread();
private:
    void fill_active_channel(int,ChannelVector *);

    typedef std::vector<struct epoll_event> EventVector;

    int m_epoll_fd;
    EventVector m_events;

    typedef std::map<int, Channel*> ChannelMap;
    ChannelMap mc_channels;

    EventLoop *mp_loop;

    static const int s_new = -1;
    static const int s_del = 0;
    static const int s_old =1;

};



}
}
}

#endif