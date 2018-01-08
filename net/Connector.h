#ifndef MOE_CONNECTOR_H
#define MOE_CONNECTOR_H



#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/SockAddr.h>

#include <sys/socket.h>

#include <functional>
#include <memory>
namespace moe 
{
namespace net 
{


class Channel;
class EventLoop;

class Connector : aux::Noncopyable
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