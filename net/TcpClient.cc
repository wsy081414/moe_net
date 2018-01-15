#include <moe_net/net/TcpClient.h>
#include <moe_net/net/SockOps.h>
#include <moe_net/net/TcpConnection.h>
#include <moe_net/net/EventLoop.h>
using namespace moe;
using namespace moe::net;

Atomic64 TcpClient::m_tcp_index;

TcpClient::TcpClient(EventLoop *loop,const SockAddr& sevaddr,const String& name)
    :mp_loop(loop),m_name(name),
    m_conn(new Connector(loop,sevaddr)),
    mb_connect(false),
    m_mutex()
{
    m_conn->set_new_conn_cb(
        std::bind(&TcpClient::new_conn,this,std::placeholders::_1)
    );

    // log
}

TcpClient::~TcpClient()
{
    // 很多
}

void TcpClient::connect()
{
    TRACELOG<<"TcpClient connect()";
    mb_connect=true;
    m_conn->start();
}

void TcpClient::disconnect()
{
    mb_connect=false;
    {
        MutexLock lock(m_mutex);
        if(mb_connect)
        {
            m_tcp_conn->shutdown();
        }
    }
}

void TcpClient::stop()
{
    mb_connect=false;
    m_conn->stop();
}
 

// new_conn 会在连接建立以后,可读的时候调用
// fd 从 connector 移交给 TcpClient 
void TcpClient::new_conn(int fd)
{
    TRACELOG<<"TcpClient::new_conn ";
    assert(mp_loop->is_in_loop_thread());

    // 从 fd 中获取具体的 addr
    SockAddr peer_addr(sockops::peer_addr(fd));
    SockAddr local_addr(sockops::local_addr(fd));

    // int64_t index = m_tcp_index.inc_get();

    TcpConnectionPtr tcp_conn(new TcpConnection(mp_loop,1,fd,local_addr,peer_addr));

    TRACELOG<<"TcpClient : establis new connection "<<fd;

    tcp_conn->set_write_cb(m_write_cb);
    tcp_conn->set_conn_cb(m_conn_cb);
    tcp_conn->set_msg_cb(m_msg_cb);
    tcp_conn->set_close_cb(
        std::bind(&TcpClient::remove_conn,this,std::placeholders::_1)
    );

    {
        MutexLock lock(m_mutex);
        m_tcp_conn = tcp_conn;
    }
    // 开始监听 可读 事件
    m_tcp_conn->connect_establised();
}

void TcpClient::remove_conn(const TcpConnectionPtr& conn)
{
    assert(mp_loop->is_in_loop_thread());

    {
        MutexLock lock(m_mutex);
        m_conn.reset();
    }    

    mp_loop->add_task(std::bind(&TcpConnection::connect_destroy,conn));

    // 没写完
}