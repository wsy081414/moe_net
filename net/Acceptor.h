#ifndef MOE_ACCEPTOR_H
#define MOE_ACCEPTOR_H


#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/Channel.h>
#include <moe_net/net/Socket.h>
#include <moe_net/net/SockAddr.h>


#include <functional>

namespace moe 
{
namespace net 
{

class EventLoop;
class SockAddr;

class Acceptor :aux::Noncopyable
{
public:
    typedef std::function<void (int,const SockAddr&)> NewConnCallBack;
    Acceptor(EventLoop *loop,const SockAddr &listen_addr,bool reuseport);
    ~Acceptor();

    void set_net_conn_cb(const NewConnCallBack& cb) {m_new_conn_cb=cb;}

    bool listening() {return mb_listening;}
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