#include <moe_net/net/Acceptor.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/SockOps.h>
using namespace moe;
using namespace moe::net;

Acceptor::Acceptor(EventLoop *loop,const SockAddr &listen_addr,bool reuseport)
    :mp_loop(loop),
    m_socket(sockops::nonblock_fd(listen_addr.family())),
    mb_listening(false),
    m_channel(mp_loop,m_socket.fd())
{
    m_channel.set_read_cb(
        std::bind(&Acceptor::handle_read,this)
    );
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
    
    int connfd  = m_socket.accept(peer_addr);
    if(connfd>0)
    {
        if(m_new_conn_cb)
        {
            m_new_conn_cb(connfd,peer_addr);
        }else{
            sockops::close(connfd);
        }
    }else{
        // log
    }
}

void Acceptor::listen()
{
    assert(mp_loop->is_in_loop_thread());
    m_socket.listen();
    mb_listening = true;
    m_channel.enable_read();
}

