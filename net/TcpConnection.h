#ifndef MOE_TCPCONNECTION_H
#define MOE_TCPCONNECTION_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/CallBack.h>
#include <moe_net/base/String.h>
#include <moe_net/net/SockAddr.h>
#include <moe_net/base/RingBuffer.h>
#include <moe_net/net/HttpContext.h>
#include <memory>


/*
想一下,这个类,干了什么
这个类分装了具体的 读写 操作

send 和 read 操作
断开连接

这个类实在 connector 的基础上获得了 peer_addr 以后才创建的.
*/
namespace moe
{
namespace net
{

class EventLoop;
class Channel;
class Socket;

class TcpConnection : aux::Noncopyable, public std::enable_shared_from_this<TcpConnection>
{
  public:
    TcpConnection(EventLoop *loop, const int64_t &index, int fd,
                  const SockAddr local_addr, const SockAddr peer_addr);
    ~TcpConnection();

    EventLoop *loop() { return mp_loop; }
    const int64_t index() { return m_index; }
    const SockAddr &local_addr() { return m_local_addr; }
    const SockAddr &peer_addr() { return m_peer_addr; }

    bool is_connected() const { return m_status == e_connected; }
    bool isnt_connected() const { return m_status == e_disconnected; }

    void send(const void *msg, size_t len);
    void send(const char *buf, size_t len);

    void send(const String &msg) { send(msg.c_str(), msg.size()); }
    void send(RingBuffer *buf);
    

    void shutdown();

    void force_close();

    void start_read();
    void start_write();
    void stop_read();

    void set_conn_cb(const ConnectionCallback &cb) { m_conn_cb = cb; }
    void set_msg_cb(const MessageCallback &cb) { m_msg_cb = cb; }
    void set_write_cb(const WriteCompleteCallback &cb) { m_write_cb = cb; }
    void set_hight_water_cb(const HighWaterMarkCallback &cb) { m_hight_water_cb = cb; }
    void set_close_cb(const CloseCallback &cb) { m_close_cb = cb; }

    void connect_establised();
    void connect_destroy();

    void context(const HttpContext& v) {m_context =v;}
    HttpContext* context() {return &m_context;}
    
  private:
    enum Status
    {
        e_disconnected,
        e_connected,
        e_connecting,
        e_disconnecting
    };

    void handle_read(Timestamp receive_time);
    void handle_write();
    void handle_error();
    void handle_close();

    void send_run_in_loop(const String&);
    void shutdown_run_in_loop();
    void force_close_run_in_loop();
    void start_read_run_in_loop();
    void stop_read_run_in_loop();

    void start_write_run_in_loop();

    void status(Status v) {m_status = v;};

    EventLoop *mp_loop;
    const int64_t m_index;
    Status m_status;

    bool mb_reading;
    int m_fd;
    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;

    const SockAddr m_local_addr;
    const SockAddr m_peer_addr;

    size_t m_hight_water;

    ConnectionCallback m_conn_cb;
    MessageCallback m_msg_cb;
    WriteCompleteCallback m_write_cb;
    HighWaterMarkCallback m_hight_water_cb;
    CloseCallback m_close_cb;

    RingBuffer mc_output;
    RingBuffer mc_input;

    HttpContext m_context;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
}
}

#endif //MOE_TCPCONNECTION_H