/*

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

typedef union epoll_data {
    void *ptr;
    int fd;
    __uint32_t u32;
    __uint64_t u64;
} epoll_data_t;

struct epoll_event {
    __uint32_t events; 
    epoll_data_t data; 
};

epoll_data_t 存在的意义：
网络模型有两种， reactor 和 proactor ：

reactor 模型是同步模型，也就是说注册文件描述符，当文件描述符可读写或是错误的时候通知用户，
然后由用户去调用相关程序判断发生的事件并且执行对应的操作。

proactor 模型则是异步模型，指定要读写的数据，当数据读写完毕的时候通知用户。

两者都是通过io复用来通知用户。

大多库都是 reactor 模型， asio 是 proactor 模型。

使用 reactor 模型时，因为不同的文件描述符要执行对应的不同的操作，因此每个文件描述符都要绑定读写关闭等操作。
而 epoll_event 中一个是事件，另一个  epoll_data_t 是一个联合体，其中的指针就可以指向文件描述符绑定的读写操作。
这样就不需要额外的 ，map 容器来建立对应关系。

*/
#include <moe_net/net/EpollPoller.h>

#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Channel.h>

#include <moe_net/net/SockOps.h>
#include <moe_net/base/Logger.h>

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
    : mp_loop(loop), m_epoll_fd(::epoll_create1(EPOLL_CLOEXEC)), mc_events(16)
{
    if (m_epoll_fd < 1)
    {
        FATAlLOG << "EpollPoller init epollfd error";
    }
}
EpollPoller::~EpollPoller()
{
    ::close(m_epoll_fd);
}

Timestamp EpollPoller::poll(int time_out, ChannelVector *wall_fill)
{

    int active_counts = ::epoll_wait(m_epoll_fd, &*mc_events.begin(),
                                     static_cast<int>(mc_events.size()),
                                     100000);
    
    Timestamp poll_return;
    if (active_counts > 0)
    {
        fill_active_vector(active_counts, wall_fill);

        // 预先调整
        if (active_counts == mc_events.size())
        {
            mc_events.resize(active_counts * 2);
        }
    }
    else if (active_counts == 0)
    {
        TRACELOG << "epoll poll return : nothing";
    }
    else
    {
        TRACELOG << "epoll poll return , errno: " << strerror(errno);
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

    int fd = channel->fd();

    if (status == s_new)
    {
        assert(mc_channels.find(fd) == mc_channels.end());
        mc_channels[fd] = channel;
        update_epoll(EPOLL_CTL_ADD, channel);
        channel->status(s_old);
    }
    else if (status == s_old)
    {
        assert(mc_channels.find(fd) != mc_channels.end());
        update_epoll(EPOLL_CTL_MOD, channel);
    }else if(status==s_del)
    {
        update_epoll(EPOLL_CTL_MOD, channel);
        channel->status(s_old);       
    }
    
    if(channel->is_no_event())
    {
        assert(mc_channels.find(fd) != mc_channels.end());
        update_epoll(EPOLL_CTL_DEL, channel);
        channel->status(s_del);
    }
}

void EpollPoller::remove(Channel *channel)
{
    int status = channel->status();
    assert(status == s_old || status==s_del);

    assert(is_in_loop_thread());
    int fd = channel->fd();

    assert(mc_channels.find(fd) != mc_channels.end());
    assert(mc_channels[fd] == channel);

    // 删除的 channel 必须是没有任何监听事件的
    assert(mc_channels[fd]->is_no_event());

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

    if (::epoll_ctl(m_epoll_fd, cmd, fd, &event) < 0)
    {
        FATAlLOG << "epoll_ctl error: "<<(cmd==EPOLL_CTL_DEL);
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
