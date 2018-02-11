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
    m_counts(),m_conn_cb(aux::default_connect_cb),m_msg_cb(aux::default_msg_cb)
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
        // it reset ,引用-1
        it->second.reset();
        // 要在 loop 中销毁  TcpConnection 原因在于，多线程，可能一个线程还在用 TcpConnection
        // 而在 loop 中休会，则可以保证 TcpConnection 用完了，才处理 添加的 task ，总之就是不并行。
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
    m_thread_pool->start(m_thread_cb);
    assert(!m_acceptor->is_listening());

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
    
    // char buf[64];

    SockAddr local_addr(sockops::local_addr(fd));

    // 在这里 channel 将在 io_loop 的 epoll 中注册
    int64_t index = m_tcp_index.inc_get();
    TcpConnectionPtr conn(new TcpConnection(io_loop,index,fd,local_addr,peer_addr));

    TRACELOG<<"TcpServer : establis new connection ";

    // 使用map保存 TcpConnectionPtr
    // TcpConnectionPtr 只有这么一个副本
    // 只要这里的没了，那么其他地方的也就跟着要没了。
    mc_connections[index] = conn;
    
    conn->set_write_complete_cb(m_write_cb);
    conn->set_msg_cb(m_msg_cb);
    conn->set_conn_cb(m_conn_cb);
    conn->set_close_cb(
        std::bind(&TcpServer::remove_conn,this,std::placeholders::_1)
    );
    
    // 相应的开始监听读 
    io_loop->add_task(
        std::bind(&TcpConnection::connect_establised,conn)
    );
}

void TcpServer::remove_conn(const TcpConnectionPtr conn)
{
    mp_loop->add_task(
        std::bind(&TcpServer::remove_conn_run_in_loop,this,conn)
    );
}
void TcpServer::remove_conn_run_in_loop(const TcpConnectionPtr conn)
{
    assert(mp_loop->is_in_loop_thread());

    size_t n = mc_connections.erase(conn->index());
    assert(n==1);
    EventLoop *io_loop = conn->loop();
    io_loop->add_task(
        std::bind(&TcpConnection::connect_destroy,conn)
    );    
}