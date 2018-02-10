/*
TcpServer 类，主要是封装了 Accept 和 TcpConnection EventLoopThreadPool

TcpServer 类，首先使用 Accept 来监听，然后设置 Accept 可读的回调函数为 new_conn ，
new_conn 会进行 内部会进行 Accept 的accept，然后获得 sockfd ，本端和对端的 sockaddr 
然后 会从 EventLoopThreadPool 选取一个 EventLoop ，使用这四个数据新建一个  TcpConnection
并且设置 TcpConnection 的各种回调函数（这些函数是由使用 TcpServer 的的类新建的）
然后由 TcpConnection 具体的处理一个链接的各种事件
*/
#ifndef MOE_TCPSERVER_H
#define MOE_TCPSERVER_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/Channel.h>
#include <moe_net/net/TcpConnection.h>
#include <moe_net/base/String.h>
#include <moe_net/net/SockAddr.h>
#include <moe_net/net/CallBack.h>
#include <moe_net/base/Atomic.h>
#include <moe_net/base/String.h>
#include <moe_net/base/Atomic.h>

#include <moe_net/net/EventLoopThreadPool.h>

#include <functional>
#include <memory>
#include <string>

namespace moe
{
namespace net
{

class Acceptor;
class EventLoop;
class EventLoopThread;

class TcpServer : aux::Noncopyable
{

  public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    TcpServer(EventLoop *loop, const std::string &name, const SockAddr &listen_addr, int option);
    
    ~TcpServer();

    EventLoop *loop() { return mp_loop; }
    void set_threads(int i);
    void set_thread_cb(const ThreadInitCallback &cb) { m_thread_cb = cb; }

    std::shared_ptr<EventLoopThreadPool> thread_pool() { return m_thread_pool; }

    void start();

    void set_conn_cb(const ConnectionCallback &cb) { m_conn_cb = cb; }
    void set_msg_cb(const MessageCallback &cb) { m_msg_cb = cb; }
    void set_write_cb(const WriteCompleteCallback &cb) { m_write_cb = cb; }

  private:
    void new_conn(int sockfd, const SockAddr &peer_addr);
    void remove_conn(const TcpConnectionPtr conn);
    void remove_conn_run_in_loop(const TcpConnectionPtr conn);

    typedef std::map<int64_t, TcpConnectionPtr> ConnectionMap;

    EventLoop *mp_loop;
    const std::string ip;
    const std::string port;
    std::unique_ptr<Acceptor> m_acceptor;
    std::shared_ptr<EventLoopThreadPool> m_thread_pool;

    ConnectionCallback m_conn_cb;
    MessageCallback m_msg_cb;
    WriteCompleteCallback m_write_cb;
    CloseCallback m_close_cb;

    Atomic64 m_counts;
    ConnectionMap mc_connections;
    ThreadInitCallback m_thread_cb;
    static Atomic64 m_tcp_index;
};
}
}

#endif //MOE_TCPSERVER_H