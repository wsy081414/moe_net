#ifndef MOE_CHANNEL_H
#define MOE_CHANNEL_H


#include <moe_net/base/Timestamp.h>
#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/String.h>

#include <functional>


namespace moe 
{
namespace net 
{
class EventLoop;

class Channel : aux::Noncopyable
{
public:
    typedef std::function<void ()> EventCallBack;
    typedef std::function<void (Timestamp)> ReadEventCallBack;
private:
    ReadEventCallBack m_read_cb;
    EventCallBack m_write_cb;
    EventCallBack m_close_cb;
    EventCallBack m_error_cb;

    int m_fd;
    int m_status;

    int m_events;
    int m_revents;

    EventLoop *mp_loop;

    static int s_read_event;
    static int s_write_event;
    static int s_none_event;
    
    bool mb_is_handling;
    bool mb_is_in_loop;
    
    String to_string(int);
    
public:
    Channel(EventLoop *,int fd);
    ~Channel();

    void handle_event(Timestamp );

    void set_read_cb(const ReadEventCallBack &cb) {m_read_cb = cb;}
    void set_write_cb(const EventCallBack &cb) {m_write_cb = cb;}
    void set_cloce_cb(const EventCallBack &cb) {m_close_cb = cb;}
    void set_error_cb(const EventCallBack &cb) {m_error_cb =cb;}

    int fd() {return m_fd;}
    int events() {return m_events;}
    void revents(int v) {m_revents=v;} 
    
    bool is_no_event() {return m_events == s_none_event;}
    bool is_reading() {return m_events & s_read_event;}
    bool is_writing() {return m_events & s_write_event;}

    void enable_read() {m_events |= s_read_event; update();}
    void enable_write() {m_events |= s_write_event; update();}
    
    void disable_read() {m_events &= ~s_read_event; update();}
    void disable_write() {m_events &= ~s_write_event; update();}
    void disable_all() {m_events = 0; update();}
    

    void status(int v) {m_status = v;}
    int status() {return m_status;}

    EventLoop *loop() {return mp_loop;}

    void remove();
    void update();
    
    String events_to_string();
    String revents_to_string();
    
 
};
}

}






#endif //MOE_CHANNEL_H