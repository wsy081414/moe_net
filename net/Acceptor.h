#ifndef MOE_ACCEPTOR_H
#define MOE_ACCEPTOR_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/Channel.h>
#include <moe_net/net/Socket.h>
#include <moe_net/net/SockAddr.h>

#include <functional>

/*
这个类,就是封装了 socket ,讲一个 socket 转为 监听描述符
同时,负责accept() 
设置了 接受一个新连接的时候应该调用的回调函数

如果没有设置这个回调函数,那么 连接建立以后,直接关闭链接.

回调函数的参数是 (fd,peer_addr) ,这样一个新连接的所有信息都有了.   

*/
namespace moe
{
namespace net
{

class EventLoop;
class SockAddr;

class Acceptor : aux::Noncopyable
{
  public:
    typedef std::function<void(int, const SockAddr &)> NewConnCallBack;

    Acceptor(EventLoop *loop, const SockAddr &listen_addr, bool reuseport);
    ~Acceptor();

    void set_net_conn_cb(const NewConnCallBack &cb) { m_new_conn_cb = cb; }

    bool listening() { return mb_listening; }
    void listen();

  private:
    void handle_read();
    EventLoop *mp_loop;
    Socket m_socket;
    Channel m_channel;
    NewConnCallBack m_new_conn_cb;
    bool mb_listening;
};
}
}
#endif //MOE_ACCEPTOR_H