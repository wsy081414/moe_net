#include <moe_net/net/Connector.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Channel.h>
#include <errno.h>

using namespace moe;
using namespace moe::net;

// 这个值在重连的时候,判断时间,每次间隔时间越来越长.这个值是最大值.
const int Connector::s_max_retry_delay=30*1000;

// 这个值表示第一次连接失败以后,重连的时候需要等待的时间.
const int Connector::s_init_retry_delay=500;

Connector::Connector(EventLoop *loop, const SockAddr &addr)
    : mp_loop(loop), 
    m_addr(addr), 
    mb_connect(false),
    m_status(e_disconnected),
    m_retry_ms(s_max_retry_delay)
{
    //  log
}
Connector::~Connector()
{
    // log
}

void Connector::start()
{

    mb_connect = true;
    mp_loop->add_task(std::bind(
        &Connector::start_in_loop, this));
    TRACELOG<<"Connector start() add over";
        
}
void Connector::restart()
{
    assert(mp_loop->is_in_loop_thread());
    status(e_disconnected);

    // 这个值表示,应该在多久以后,再尝试一次.
    m_retry_ms = s_init_retry_delay;
    mb_connect = true;
    start_in_loop();
}
void Connector::stop()
{
    TRACELOG<<"Connector::stop()";
    
    mb_connect = false;
    mp_loop->add_task_in_queue(std::bind(
        &Connector::stop_in_loop, this));
}

void Connector::start_in_loop()
{
    TRACELOG<<"Connector start_in_loop()";
    
    assert(mp_loop->is_in_loop_thread());
    assert(m_status == e_disconnected);
    // mb_connect 表示应该要连接,所以 connect
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
    TRACELOG<<"Connector::stop_in_loop()";
    
    assert(mp_loop->is_in_loop_thread());
    // if (m_status == e_connecting)
    // {
        // status(e_disconnected);
        // 将 m_channel 从loop中撤销,然后重置 m_channel 
        // 然后,将m_fd返回
        int sockfd = remove();
        // retry(sockfd);
    // }
}

void Connector::connect()
{
    // 复杂的判断

    int sockfd = sockops::nonblock_fd(m_addr.family());

    int ret = sockops::connect(sockfd, m_addr.sockaddr());

    int save_error = (ret == 0) ? 0 : errno;
    
    TRACELOG<<"Connector connection() :"<< strerror(save_error)<<" "<< (EINPROGRESS == save_error)<<" "<<sockfd;
    

    switch (save_error)
    {
        // 下面的表示可以建立连接.因此可以创建 m_channel
    case 0:
    case EINPROGRESS: //套接字为非阻塞套接字，且连接请求没有立即完成
    case EINTR:       //系统调用的执行由于捕获中断而中止
    case EISCONN:     //已经连接到该套接字
        connecting(sockfd);
        break;


        // 下面的表示连接无法建立,因此需要关闭 m_fd
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
    status(e_connecting);
    // 因为在 connecting() 之前,套接字没有连接完成,因此 m_channel 是空.
    assert(!m_channel);
    m_channel.reset(new Channel(mp_loop, fd));

    m_channel->set_write_cb(
        std::bind(&Connector::handle_write, this));
    m_channel->set_error_cb(
        std::bind(&Connector::handle_error, this));

    // 当连接建立以后,就可以向 套接字里写东西了.
    // 因为是客户端,所以,应该是可写的.
    m_channel->enable_write();
}

// 表示连接失败,因此需要关闭套接字.
void Connector::retry(int fd)
{
    sockops::close(fd);
    status(e_disconnected);
    // 如果 mb_connect 表示应该建立连接.
    if (mb_connect)
    {
        TRACELOG<<"Connector::retry";
        // shared_from_this 的意思是,因为这个是异步的.因此当执行的时候,这个对象已经被析构了.
        // 因此将本对象的shared_ptr 传入.
        mp_loop->add_timer(
            std::bind(&Connector::start_in_loop,shared_from_this()),
            m_retry_ms
            );

        // moduo 这里有个退步算法
    }
    else
    {
        //
    }
}

int Connector::remove()
{
    TRACELOG<<"Connector::remove() connect channel close: "<<m_channel->fd();

    m_channel->disable_all();
    m_channel->remove();
    int sockfd = m_channel->fd();
    // 这里为什么要在 loop 里执行?
    mp_loop->add_task_in_queue(
        std::bind(&Connector::reset, this));
    return sockfd;
}

void Connector::reset()
{
    // TRACELOG<<"Connector::reset()";
    m_channel.reset();
}

// 向服务器发起连接以后,当连接可读的时候,调用的回调函数.
// 但是同时,又会从epoll 中移除监听.
// 也就是,只会一次.
// 此时,这个fd ,被移交给 TcpClient 中的 TcpConnection 对象
// 所以,要移除 m_channel 
// 这个类,只负责建立连接,并不负责具体的读写数据

// 非阻塞式connect连接完成，被认为是套接字可写。
void Connector::handle_write()
{
    if (m_status == e_connecting) 
    {
    TRACELOG<<"Connector::handle_write()";

        int sockfd = remove();

        int err = sockops::sock_error(sockfd);
        if (err)
        {
            // 当出现错误的时候,就重连.
            // 因为这是客户端,所以需要重新连接,再次发送数据
            // log
            retry(sockfd);
        }
        else
        {
            // 如果没出错,并且连接了
            if (mb_connect && m_new_conn_cb)
            {
                // 回调函数是必备的.
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
    TRACELOG<<"Connector::handle_error(): "<<m_status;
    if (m_status == e_connecting)
    {
        int sockfd = remove();
        int err = sockops::sock_error(sockfd);

        // log

        // 出现了错误,也需要重连
        retry(sockfd);
    }
    // stop();
}