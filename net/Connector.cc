#include <moe_net/net/Connector.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Channel.h>


using namespace moe;
using namespace moe::net;

const int Connector::s_max_retry_delay=30*1000;
const int Connector::s_init_retry_delay=500;

Connector::Connector(EventLoop *loop, const SockAddr &addr)
    : mp_loop(loop), m_addr(addr), mb_connect(false), m_status(e_disconnected),
      m_retry_ms(s_max_retry_delay)
{
    //  log
}
Connector::~Connector()
{


}

void Connector::start()
{
    mb_connect = true;
    mp_loop->add_task(std::bind(
        &Connector::start_in_loop, this));
}
void Connector::restart()
{
    assert(mp_loop->is_in_loop_thread());
    status(e_disconnected);
    m_retry_ms = s_init_retry_delay;
    mb_connect = true;
    start_in_loop();
}
void Connector::stop()
{
    mb_connect = false;
    mp_loop->add_task(std::bind(
        &Connector::stop_in_loop, this));
}

void Connector::start_in_loop()
{
    assert(mp_loop->is_in_loop_thread());
    assert(m_status == e_disconnected);
    if (mb_connect)
    {
        connect();
    }
    else
    {
        // log
    }
}

void Connector::stop_in_loop()
{
    assert(mp_loop->is_in_loop_thread());
    if (m_status == e_connecting)
    {
        status(e_disconnected);
        int sockfd = remove();
        retry(sockfd);
    }
}

void Connector::connect()
{
    // 复杂的判断

    int sockfd = sockops::nonblock_fd(m_addr.family());

    int ret = sockops::connect(sockfd, m_addr.sockaddr());

    int save_error = (ret == 0) ? 0 : errno;

    switch (save_error)
    {
    case 0:
    case EINPROGRESS: //套接字为非阻塞套接字，且连接请求没有立即完成
    case EINTR:       //系统调用的执行由于捕获中断而中止
    case EISCONN:     //已经连接到该套接字
        connecting(sockfd);
        break;

    case EAGAIN:        //没有足够空闲的本地端口
    case EADDRINUSE:    //本地地址处于使用状态
    case EADDRNOTAVAIL: //在本地机器上找不到所指的地址
    case ECONNREFUSED:  //连接尝试被强制拒绝
    case ENETUNREACH:   //当前无法从本主机访问网络
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:        //操作不许可,权限所致
    case EAFNOSUPPORT: //family不支持的protocol
    case EALREADY:     //操作符已经就绪
    case EBADF:        //错误的文件数
    case EFAULT:       //错误的地址
    case ENOTSOCK:     //文件描述符不是一个socket描述符
        // log
        sockops::close(sockfd);
        break;

    default:
        // log
        sockops::close(sockfd);
        // connectErrorCallback_();
        break;
    }
}
void Connector::connecting(int fd)
{
    assert(mp_loop->is_in_loop_thread());
    assert(!m_channel);
    m_channel.reset(new Channel(mp_loop, fd));

    m_channel->set_write_cb(
        std::bind(&Connector::handle_write, this));
    m_channel->set_error_cb(
        std::bind(&Connector::handle_error, this));
    m_channel->enable_write();
}

// 这个没太懂
// 防止回调的时候之前在callback里面所绑定的指针已经失效
void Connector::retry(int fd)
{
    sockops::close(fd);
    status(e_disconnected);

    if (mb_connect)
    {
        //
    }
    else
    {
        //
    }
}

int Connector::remove()
{
    m_channel->disable_all();
    m_channel->remove();
    int sockfd = m_channel->fd();

    mp_loop->add_task(
        std::bind(&Connector::reset, this));
    return sockfd;
}
void Connector::reset()
{
    m_channel->remove();
}

// 干了些什么?
void Connector::handle_write()
{
    if (m_status == e_connecting)
    {
        int sockfd = remove();

        int err = sockops::sock_error(sockfd);
        if (err)
        {
            // log
            retry(sockfd);
        }
        else
        {
            if (mb_connect)
            {
                m_new_conn_cb(sockfd);
            }
            else
            {
                sockops::close(sockfd);
            }
        }
    }
    else
    {
        assert(m_status == e_disconnected);
    }
}

void Connector::handle_error()
{
    // log
    if (m_status == e_connecting)
    {
        int sockfd = remove();
        int err = sockops::sock_error(sockfd);

        // log

        retry(sockfd);
    }
}