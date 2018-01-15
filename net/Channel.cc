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
int Channel::s_none_event =0;

Channel::Channel(EventLoop *loop,int fd)
    :mp_loop(loop),m_fd(fd),m_events(0),m_revents(0),m_status(EpollPoller::s_new),
    mb_is_handling(false),mb_is_in_loop(false)
{
    // TRACELOG<<"new channel, fd:"<<fd;
}

Channel::~Channel()
{
    // 确保 本 channel 没有在loop中，与loop没有关联
    assert(!mb_is_handling);
    assert(!mb_is_in_loop);

    if(mp_loop->is_in_loop_thread())
    {
        // 没有在 loop 中
        assert(!mp_loop->has_channel(this));
    }
    // TRACELOG<<"Channel::~Channel() fd: "<<m_fd;
}

void Channel::update()
{
    mb_is_in_loop = true;
    mp_loop->update(this);
}

void Channel::remove()
{
    assert(is_no_event());
    mb_is_in_loop = false;
    mp_loop->remove(this);
}

void Channel::handle_event(Timestamp receive_time)
{
    mb_is_handling = true; 
    int ev =m_revents;
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

    TRACELOG<<"event :"<<oss.str().c_str();

    if((m_revents  & POLLHUP) && !(m_revents & POLLIN))
    {
        TRACELOG<<"channel run close_cb"<<bool(m_close_cb);

        if(m_close_cb)
        {
            m_close_cb();
        }
    }

    // POLLNVAL:指定的文件描述符非法。
    if(m_revents & POLLNVAL)
    {
        WARNLOG<<" fd="<<m_fd<<" POLLNVAL ";
    }
    // POLLERR:指定的文件描述符发生错误
    if(m_revents &(POLLNVAL |POLLERR))
    {
        
        if(m_error_cb)
        {
            TRACELOG<<"channel run m_error_cb";
            m_error_cb();
        }
    }

    // POLLIN:数据可读 ，POLLPRI:高优先级数据可读
    // 当socket的另一端关闭close时，这一端会捕获 POLLHUP 事件
    if(m_revents & (POLLIN | POLLPRI | POLLHUP ))
    {
        if(m_read_cb)
        {
            TRACELOG<<"channel run m_read_cb "<<bool(m_read_cb);
            m_read_cb(receive_time);
        }
    }

    // POLLOUT 数据可写 
    if(m_revents & POLLOUT)
    {
        if(m_write_cb)
        {
        TRACELOG<<"channel run m_write_cb"<<bool(m_write_cb);
            
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
    return String(" ");
}
