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
#include <moe_net/net/EventLoopThreadPool.h>


#include <functional>
#include <memory>
#include <string>


/*
这个类封装了 Acceptor 和 TcpConnection 

获取连接以后,调用回调函数,建立一个 TcpConnection 对象,放入 mc_connection .
mc_connection 是一个map,但是 map<sting,TcpConnection> 
后面改为 atomic64 应该更好啊.

*/
namespace moe 
{
namespace net 
{

class Acceptor;
class EventLoop;
class EventLoopThread;

class TcpServer :aux::Noncopyable 
{

public:
  typedef std::function<void (EventLoop*)> ThreadInitCallback;

    TcpServer(EventLoop *loop,const std::string& name,const SockAddr& listen_addr,int option);
    ~TcpServer();

    EventLoop *loop() {return mp_loop;}
    void set_threads(int i);
    void set_thread_cb(const ThreadInitCallback& cb) {m_thread_cb=cb;}


    std::shared_ptr<EventLoopThreadPool> thread_pool() {return m_thread_pool;}

    void start();

    void set_conn_cb(const ConnectionCallback& cb) {m_conn_cb = cb;}
    void set_msg_cb(const MessageCallback& cb ) {m_msg_cb=cb;}
    void set_write_cb(const WriteCompleteCallback& cb) {m_write_cb = cb;}
private:
    void new_conn(int sockfd,const SockAddr& peer_addr);
    void remove_conn(const TcpConnectionPtr conn);
    void remove_conn_run_in_loop(const TcpConnectionPtr conn);

    typedef std::map<std::string ,TcpConnectionPtr> ConnectionMap;

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
    

};
}
}

#endif //MOE_TCPSERVER_H