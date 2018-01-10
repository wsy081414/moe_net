#ifndef MOE_CONNECTOR_H
#define MOE_CONNECTOR_H



#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/SockAddr.h>

#include <sys/socket.h>

#include <functional>
#include <memory>


/*
这个类,怎么说
就是简单的封装了 客户端 发起连接的过程.

当连接成功以后,就去调用,连接以后应该调用的回调函数.

首先,在发起连接的时候,可能 "成功",那么就直接建立 m_channel 

如果连接的时候发生错误,可以恢复的那种,就在 m_retry_ms 时间后,重新 发起连接,这里使用的的是 eventloop 的timer
这里 m_retry_ms 使用了,退步的算法. m_retry_ms 一次比一次 长,但是不会超过 s_max_retry_delay 
但是 retry 没有设置 最大重试次数.

在m_channel 发生错误和sockfd 发生错误的时候,都会调用 retry.

这个类,只负责建立连接,并不负责读写数据
*/
namespace moe 
{
namespace net 
{


class Channel;
class EventLoop;

class Connector : aux::Noncopyable,public std::enable_shared_from_this<Connector>
{
public:
    typedef std::function<void (int)> NewConnCallBack;

    Connector(EventLoop *,const SockAddr& );
    ~Connector();
    void set_new_conn_cb(const NewConnCallBack &cb) {m_new_conn_cb = cb;}

    void start();
    void restart();
    void stop();

    const SockAddr &sockaddr() {return m_addr;}
private:
    enum Status 
    {
        e_disconnected,
        e_connecting,
        e_connected
    };

    static const int s_max_retry_delay ;
    static const int s_init_retry_delay ;

    void status(Status e) {m_status = e;}
    void start_in_loop();
    void stop_in_loop(); 

    void connect();
    void connecting(int fd);

    void handle_write();
    void handle_error();
    void retry(int fd);
    int remove();
    void reset();


    EventLoop *mp_loop;
    SockAddr m_addr;
    bool mb_connect;
    Status m_status;

    std::unique_ptr<Channel> m_channel;
    NewConnCallBack m_new_conn_cb;
    int m_retry_ms;
};

}
}






#endif //MOE_CONNECTOR_H