#include <moe_net/net/EpollPoller.h>

#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Channel.h>

#include <moe_net/net/SockOps.h>

#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>

using namespace moe;
using namespace moe::net;

// epoll_create1 和 epoll_create 一样，第一个接受一个flag 只有一个可选 EPOLL_CLOEXEC
// 如果为 0 ，那么和 epoll_create 一样，但是没定义 size 是多少
// https://stackoverflow.com/questions/10011252/whats-the-advantage-of-using-epoll-create1-instead-of-epoll-create
// epoll_create 是一个老的，旧的
EpollPoller::EpollPoller(EventLoop *loop)
    : mp_loop(loop), m_epoll_fd(::epoll_create1(EPOLL_CLOEXEC)),mc_events(16)
{
    if (m_epoll_fd)
    {
        // log
    }
}
EpollPoller::~EpollPoller()
{
    ::close(m_epoll_fd);
}

Timestamp EpollPoller::poll(int time_out, ChannelVector *wall_fill)
{
    // log
    
    // TRACELOG<<"fd:"<<m_epoll_fd<<" size:"<<mc_events.size()<<" timeout:"<<time_out;
    int active_counts = ::epoll_wait(m_epoll_fd, &*mc_events.begin(),
                                     static_cast<int>(mc_events.size()),
                                     time_out);
    TRACELOG<<"epoll poll return ,counts:"<<active_counts<<" errno: "<<strerror(errno);
    int err_tmp = errno;
    Timestamp poll_return;
    if (active_counts > 0)
    {
        // log
        fill_active_vector(active_counts, wall_fill);

        // 预先调整
        if (active_counts == mc_events.size())
        {
            mc_events.resize(active_counts * 2);
        }
    }
    else if (active_counts == 0)
    {
        // log
    }
    else
    {
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
        will_fill->push_back(channel);
    }
}

void EpollPoller::update(Channel *channel)
{
    assert(is_in_loop_thread());
    const int status = channel->status();

    // log
    int fd = channel->fd();

    if (status == s_new || status == s_old)
    {
        // 新添加的 channel
        if (status == s_new)
        {
            assert(mc_channels.find(fd) == mc_channels.end());
            mc_channels[fd] = channel;
            update_epoll(EPOLL_CTL_ADD, channel);
            TRACELOG<<"epoll add channel :"<<mc_channels.size();
        }
        else if (status == s_old)
        {
            // 修改已有的
            assert(mc_channels.find(fd) != mc_channels.end());
            update_epoll(EPOLL_CTL_MOD, channel);
        }
        channel->status(s_old);
    }
    else
    {
        // // 删除已有的

        // assert(mc_channels.find(fd) != mc_channels.end());
        // assert(mc_channels[fd] == channel);

        // assert(status == s_del);

        // // 不在监听
        // if (channel->is_no_event())
        // {
        //     update_epoll(EPOLL_CTL_DEL, channel);
        //     channel->status(s_del);
        // }
        // else
        // {
        //     // 更新
        //     update_epoll(EPOLL_CTL_MOD, channel);
        // }
    }
}

void EpollPoller::remove(Channel *channel)
{
    assert(is_in_loop_thread());
    int fd = channel->fd();

    assert(mc_channels.find(fd) != mc_channels.end());
    assert(mc_channels[fd] == channel);
    assert(mc_channels[fd]->is_no_event());

    int status = channel->status();
    assert(status == s_old);

    size_t n = mc_channels.erase(fd);
    assert(n == 1);

    if (status == s_old)
    {
        update_epoll(EPOLL_CTL_DEL, channel);
    }
    channel->status(s_new);
}

void EpollPoller::update_epoll(int cmd, Channel *channel)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;

    int fd = channel->fd();

    // log

    if (::epoll_ctl(m_epoll_fd, cmd, fd, &event) < 0)
    {
        FATAlLOG<<"epoll_ctl error";
    }
}

bool EpollPoller::has_channel(Channel *channel)
{
    assert(is_in_loop_thread());
    ChannelMap::iterator it = mc_channels.find(channel->fd());
    return it != mc_channels.end() && it->second == channel;
}

bool EpollPoller::is_in_loop_thread()
{
    return mp_loop->is_in_loop_thread();
}
