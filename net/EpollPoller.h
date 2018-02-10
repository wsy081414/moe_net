#ifndef MOE_EPOLLPOLLER_H
#define MOE_EPOLLPOLLER_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/Timestamp.h>
#include <moe_net/base/Mutex.h>

#include <map>
#include <vector>
#include <sys/epoll.h>

namespace moe
{
namespace net
{

using namespace moe;
using namespace moe::net;

class Channel;
class EventLoop;

class EpollPoller : moe::aux::Noncopyable
{
  public:
    typedef std::vector<Channel *> ChannelVector;
    EpollPoller(EventLoop *);
    ~EpollPoller();

    Timestamp poll(int, ChannelVector *);
    void update(Channel *);
    void remove(Channel *);

    bool has_channel(Channel *);
    bool is_in_loop_thread();

    static const int s_new = -1; // 第一次添加的时候channel 的状态是new
    static const int s_del = 0;  // 暂时没用
    static const int s_old = 1;  // 在修改的时候，channel 都是这个
  private:
    void fill_active_vector(int, ChannelVector *);
    void update_epoll(int, Channel *);
    typedef std::vector<struct epoll_event> EventVector;

    int m_epoll_fd;
    EventVector mc_events; //在loop返回的时候被内核填充

    typedef std::map<int, Channel *> ChannelMap;
    ChannelMap mc_channels;

    EventLoop *mp_loop;
};
}
}

#endif