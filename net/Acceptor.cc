#include <moe_net/net/Acceptor.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/SockOps.h>

#include <stdio.h>

using namespace moe;
using namespace moe::net;

Acceptor::Acceptor(EventLoop *loop, const SockAddr &listen_addr, bool reuseport)
    : mp_loop(loop),
      m_socket(sockops::nonblock_fd(listen_addr.family())),
      mb_listening(false),
      m_channel(mp_loop, m_socket.fd())
{
    m_socket.set_reuse_port(reuseport);
    m_socket.set_reuse_addr(reuseport);

    m_channel.set_read_cb(
        std::bind(&Acceptor::handle_read, this));
    m_socket.bind(listen_addr);
}

Acceptor::~Acceptor()
{
    m_channel.disable_all();
    m_channel.remove();
}

void Acceptor::handle_read()
{
    assert(mp_loop->is_in_loop_thread());
    SockAddr peer_addr;

    // peer_addr 被填充
    int connfd = m_socket.accept(peer_addr);

    TRACELOG << "Acceptor::handle_read peer_addr :" << peer_addr.to_ip_port().c_str();

    if (connfd > 0)
    {
        // 新连接以后,要么调用回调函数,直接处理.否则就直接关闭链接.
        if (m_new_conn_cb)
        {
            m_new_conn_cb(connfd, peer_addr);
        }
        else
        {
            sockops::close(connfd);
        }
    }
    else
    {
        // 如果 accept 返回的文件描述符 < 0 ,表示是错误,应该要处理错误
        ERRORLOG<<"Acceptor::handle_read error: "<<strerror(errno);

        /* 当 errno==EMFILE ,也就是文件描述符上限的时，muduo 的方法是：
            在 Accept 的构造的时候就额外的占用了一个 文件描述符 idle，
            当 EMFILE 的时候就关闭 idle ，然后accept，然后关闭 idle
        */
    }
}

void Acceptor::listen()
{
    assert(mp_loop->is_in_loop_thread());
    m_socket.listen();
    mb_listening = true;

    // fd 是监听描述符.
    // 当开始 listen 以后, 就可以接受新的连接.
    // 所以这里是 enable_read
    m_channel.enable_read();
}
