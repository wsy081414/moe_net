#include <moe_net/net/Channel.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/base/Logger.h>
#include <moe_net/net/EpollPoller.h>
#include <poll.h>
#include <assert.h>
#include <sstream>

using namespace moe;
using namespace moe::net;

int Channel::s_read_event = POLLIN | POLLPRI;
int Channel::s_write_event = POLLOUT;
int Channel::s_none_event = 0;

Channel::Channel(EventLoop *loop, int fd)
    : mp_loop(loop), m_fd(fd), m_events(0), m_revents(0), m_status(EpollPoller::s_new),
      mb_is_handling(false), mb_is_in_loop(false)
{
}

Channel::~Channel()
{
    // 确保 本 channel 没有在没有正在被 eventloop 监听事件，同时没有在处理事件
    // assert(!mb_is_handling);
    assert(!mb_is_in_loop);

    if (mp_loop->is_in_loop_thread())
    {
        assert(!mp_loop->has_channel(this));
    }
}

void Channel::update()
{
    mb_is_in_loop = true;
    mp_loop->update(this);
}

void Channel::remove()
{
    // 只有在 channel 没有设置要监听的事件的时候才能移除
    assert(is_no_event());
    mb_is_in_loop = false;
    mp_loop->remove(this);
}
/*
有新连接请求，触发EPOLLIN
对端发送普通数据，触发EPOLLIN
带外数据，只触发EPOLLPRI

对端正常关闭EPOLLIN和EPOLLRDHUP,不触发EPOLLERR和EPOLLHUP
本端（server端）出错EPOLLERR或者EPOLLHUP

对端异常断开连接（只测了拔网线），没触发任何事件。

EPOLLRDHUP 这个好像有些系统检测不到，可以使用EPOLLIN，read返回0，删除掉事件，关闭close(fd);
EPOLLERR 是服务器这边出错才返回的，向已经断开的socket写或者读，会发生EPollErr，即表明已经断开，会发生EPOLLERR。
出现Error把对方DEL掉，close就可以

*/
void Channel::handle_event(Timestamp receive_time)
{
    mb_is_handling = true;
  
    // POLLHUP 实在对端关闭套接字的时候返回的，如果有 POLLHUP 而且不可读 POLLIN
    // 那么就关闭连接
    if ((m_revents & POLLHUP) && !(m_revents & POLLIN))
    {
        if (m_close_cb)
        {
            TRACELOG << "channel run m_close_cb: "<<m_fd;
            m_close_cb();
        }
    }

    // POLLNVAL:指定的文件描述符非法。
    if (m_revents & POLLNVAL)
    {
        WARNLOG << "channel run m_close_cb: "<<m_fd;
    }

    // POLLERR:指定的文件描述符发生错误，自己方socket发生错误，
    // 同时还会触发epollin和epollrdhup
    // 向已经断开的socket写或者读，会发生EPollErr
    if (m_revents & (POLLNVAL | POLLERR))
    {

        if (m_error_cb)
        {
            TRACELOG << "channel run m_close_cb: "<<m_fd;
            m_error_cb();
        }
    }

    // POLLIN:数据可读 ，POLLPRI:高优先级数据可读
    // 当socket的另一端关闭close时，这一端会捕获 POLLHUP 事件
    if (m_revents & (POLLIN | POLLPRI | POLLHUP))
    {
        if (m_read_cb)
        {
            TRACELOG << "channel run m_read_cb: "<<m_fd;
            m_read_cb(receive_time);
        }
    }

    // POLLOUT 数据可写
    if (m_revents & POLLOUT)
    {
        if (m_write_cb)
        {
            TRACELOG << "channel run m_write_cb: "<<m_fd;
            m_write_cb();
        }
    }
    mb_is_handling = false;
}

String Channel::events_to_string()
{
    return to_string(m_events);
}
String Channel::revents_to_string()
{
    return to_string(m_revents);
}

String Channel::to_string(int ev)
{
    int ev = m_revents;
    std::ostringstream oss;
    oss << m_fd << ": ";
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";
    return oss.str().c_str();
}
