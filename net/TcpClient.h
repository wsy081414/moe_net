#ifndef MOE_TCPCLIENT_H
#define MOE_TCPCLIENT_H


#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/SockAddr.h>
#include <moe_net/net/CallBack.h>

#include <moe_net/base/String.h>
#include <moe_net/base/Mutex.h>

#include <moe_net/net/Connector.h>
#include <moe_net/net/TcpConnection.h>

/*
这个类,也只是封装了 connector 和 TcpConnection 这两个类.
TcpClient 顺序上,先是使用 connector 建立连接.
连接建立以后, connector 的回调函数 new_conn ,讲连接移交给 TcpConnection 
具体的读写操作是 TcpConnection 来操作的.
而 TcpClient 对象的 connection() 可以返回 TcpConnection 对象.




*/
namespace moe 
{
namespace net
{

class EventLoop;


class TcpClient 
{

public:
typedef std::shared_ptr<Connector> ConnectorPtr;

    TcpClient(EventLoop *loop,const SockAddr& sevaddr,const String& name);
    ~TcpClient();


    void connect();
    void disconnect();
    
    void stop();

    EventLoop *loop() {return mp_loop;}
    const String name() {return m_name;}

    TcpConnectionPtr connection()
    {
        MutexLock lock(m_mutex);
        return m_tcp_conn;
    }

    void set_conn_cb(const ConnectionCallback& cb) {m_conn_cb = cb;}
    void set_msg_cb(const MessageCallback& cb) {m_msg_cb = cb;}
    void set_write_cb(const WriteCompleteCallback& cb) {m_write_cb = cb;}

private:
    void new_conn(int fd);
    void remove_conn(const TcpConnectionPtr& conn);

    EventLoop *mp_loop;

    ConnectionCallback m_conn_cb;
    MessageCallback m_msg_cb;
    WriteCompleteCallback m_write_cb;

    const String m_name;
    ConnectorPtr m_conn;
    TcpConnectionPtr m_tcp_conn;    

    bool mb_connect;
    Mutex m_mutex;
};
}
}

#endif //MOE_TCPCLIENT_H