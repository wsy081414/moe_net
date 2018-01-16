#include <moe_net/net/TcpServer.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/EventLoopThread.h>

#include <moe_net/net/Acceptor.h>
#include <memory>


using namespace moe;
using namespace moe::net;

Atomic64 TcpServer::m_tcp_index;

TcpServer::TcpServer(EventLoop *loop,const std::string& name,const SockAddr& listen_addr,int option)
    :mp_loop(loop),m_acceptor(new Acceptor(loop,listen_addr,option)),
    m_thread_pool(new EventLoopThreadPool(loop,name)),
    m_counts()
{
    m_acceptor->set_net_conn_cb(
        std::bind(&TcpServer::new_conn,this,std::placeholders::_1,std::placeholders::_2)
    );
}


TcpServer::~TcpServer()
{
    assert(mp_loop->is_in_loop_thread());
    for(ConnectionMap::iterator it = mc_connections.begin();it!=mc_connections.end();++it)
    {
        TcpConnectionPtr conn(it->second);
        it->second.reset();
        conn->loop()->add_task(
            std::bind(&TcpConnection::connect_destroy,conn)
        );
    }
}

void TcpServer::set_threads(int i)
{
    assert(i>=0);
    m_thread_pool->set_threads(i);
}

void TcpServer::start()
{
    // 这里的判断没懂

    m_thread_pool->start(m_thread_cb);
    assert(!m_acceptor->listening());

    // 开始 listen
    mp_loop->add_task(
        std::bind(&Acceptor::listen,m_acceptor.get())
    );
}


// 
void TcpServer::new_conn(int fd,const SockAddr &peer_addr)
{
    // TRACELOG<<" TcpServer::new_conn";
    assert(mp_loop->is_in_loop_thread());
    
    char name[32];

    // 这个算是线程的负载均衡了.
    EventLoop *io_loop = m_thread_pool->next_loop();
    // TRACELOG<<" TcpServer::new_conn io_loop"<<io_loop;
    
    // char buf[64];

    SockAddr local_addr(sockops::local_addr(fd));

    // 在这里 channel 将在 io_loop 的 epoll 中注册
    int64_t index = m_tcp_index.inc_get();
    TcpConnectionPtr conn(new TcpConnection(io_loop,index,fd,local_addr,peer_addr));
    TRACELOG<<"TcpServer : establis new connection "<<fd;


    mc_connections[index] = conn;
    
    conn->set_write_cb(m_write_cb);
    conn->set_msg_cb(m_msg_cb);
    conn->set_conn_cb(m_conn_cb);
    conn->set_close_cb(
        std::bind(&TcpServer::remove_conn,this,std::placeholders::_1)
    );
    
    // 相应的开始监听 读 
    io_loop->add_task(
        std::bind(&TcpConnection::connect_establised,conn)
    );
}

void TcpServer::remove_conn(const TcpConnectionPtr conn)
{
    // TRACELOG<<"TcpServer::remove_conn";
    
    mp_loop->add_task(
        std::bind(&TcpServer::remove_conn_run_in_loop,this,conn)
    );
}
void TcpServer::remove_conn_run_in_loop(const TcpConnectionPtr conn)
{
    // TRACELOG<<"TcpServer::remove_conn_run_in_loop";
    assert(mp_loop->is_in_loop_thread());

    size_t n = mc_connections.erase(conn->index());
    assert(n==1);
    EventLoop *io_loop = conn->loop();
    io_loop->add_task(
        std::bind(&TcpConnection::connect_destroy,conn)
    );    
}