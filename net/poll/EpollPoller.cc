#include <moe_net/net/poll/EpollPoller.h>

#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Sock.h>

#include <epoll.h>

using namespace moe;
using namespace moe::net;
using namespace moe::net::parts;

// epoll_create1 和 epoll_create 一样，第一个接受一个flag 只有一个可选 EPOLL_CLOEXEC
// 如果为 0 ，那么和 epoll_create 一样，但是没定义 size 是多少
// https://stackoverflow.com/questions/10011252/whats-the-advantage-of-using-epoll-create1-instead-of-epoll-create
// epoll_create 是一个老的，旧的
EpollPoller::EpollPoller(EventLoop *loop)
    : m_loop(loop), m_epoll_fd(::epoll_create1(EPOLL_CLOEXEC))
{
    if (m_epoll_fd)
    {
        // log
    }
}
EpollPoller::~EventLoop()
{
    ::close(m_epoll_fd);
}

Timestamp EpollPoller::poll(int time_out, ChannelVector *wall_fill)
{
    // log

    int active_counts=::epoll_wait(m_epoll_fd,
                                &*mc_events.begin(),static_cast<int>(mc_events.size(),
                                time_out
                                );
    int err_tmp = error;
    Timestamp poll_return;
    if(active_counts >0)
    {
        // log
        fill_active_vector(active_counts, wall_fill);

        // 预先调整
        if (active_counts == mc_events.size())
        {
            mc_events.erase(active_counts * 2);
        }
    }else if(active_counts == 0)
    {
        // log
    }else{
        // log 
    }

    return poll_return;
}

void EpollPoller::fill_active_vector(int active_counts, ChannelVector *will_fill)
{
    assert(static_cast<size_t>(active_counts) <= mc_events.size());

    for (int i = 0; i < active_counts; ++i)
    {
        Channel *channel = static_cast<Channel *>(mc_events[i].data.ptr);
        channel->revents(mc_events[i].events);
        will_fill.push_back(channel);
    }
}

void EpollPoller::update(Channel *channel)
{
    assert(is_in_loop_thread());
    const int status = channel->status();

    // log

    if (status == s_new || status == s_del)
    {
        // 新添加的 channel
        if (status == s_new)
        {
            assert(mc_channels.find(fd) == mc_channels.end());
            mc_channels[fd] = channel;
        }
        else if (status = s_del)
        {
            // 删除
            assert(mc_channels.find(fd) != mc_channels.end());
        }
        channel->status(s_old);
        update_epoll(EPOLL_CTL_ADD,channel);
    }else{
        // 更新新的
        int fd = channel->fd();

        assert(mc_channels.find(fd) != mc_channels.end());
        assert(mc_channels[fd]==channel);

        assert(status == s_old);

        // 不在监听
        if(channel->is_no_event())
        {
            update_epoll(EPOLL_CTL_DEL, channel);
            channel->status(s_del);
        }else{
            // 更新
            update_epoll(EPOLL_CTL_MOD, channel);
        }

        
    }
}

void EpollPoller::update_epoll(int cmd, Channel *channel)
{
    struct epoll_event event;
    bezero(event,sizeof(event));
    event.events = channel->events();
    event.data.ptr=channel;

    int fd = channel->fd();
    
    // log

    if(::epoll_ctl(m_epoll_fd,cmd,fd,&event)<0)
    {
        // log
    }
}

bool EpollPoller::has_channel(Channel *channel)
{
    assert(is_in_loop_thread());
    ChannelMap::iterator it = mc_channels.find(channel->fd);
    return it != mc_channels.end() && it->second == channel;
}

bool EpollPoller::is_in_loop_thread()
{
    return mp_loop->is_in_loop_thread();
}
