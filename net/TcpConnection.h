/*
TcpConnection 类
基本上全程使用了 只能指针来来维护生命周期。
TcpConnection 在 io_loop 中，调用完 connect_destroy 后被析构。


在 TcpServe 中的使用：
TcpServe 在使用Accept获取一个链接的 sockfd ，本端对端地址，还有一个 io_loop 以后，使用这些信息
新建一个 TcpConnection 类，并设置各种回调函数
TcpConnection 类则使用，sockfd 和 回调函数，建立了一个 channel 并且在 TcpServe 时，将channel 注册到了 loop中
由 EpollPoll 进行监听


TcpConnection 连接的状态 m_status :
由 TcpServe 创建的时候 m_status== e_connecting 也就是链接还在建立。表示 TcpConnection 还没有初始化完全
此时，虽然 TcpConnection 绑定了 EventLoop ，但是并没有在 EpollPoller 中注册。因此不会有任何的事件
当 TcpServe 设置好了 TcpConnection 的各种回调事件， 然后，在 TcpConnection 的 loop 中执行 connect_establised
该函数会在 EpollPoller 中注册，并开始监听 ——读—— 事件，也就是说该服务器，不会再链接成功后就想对端写数据。需要显式的请求

然后，当可读的时候，channel 调用传入的回调函数  TcpConnection::handle_read ,读取数据，当督导具体数据以后，
调用 m_msg_cb，这是由上层 TcpServer 传入的回调函数。

当调用 shutdown 时，会立即将 m_status == e_disconnecting 表示正在关闭，然后在 loop 中执行具体的关闭动作
但是此时可能有数据写出，shutdown 会确保数据都写出以后关闭。
因此，在 handle_write 数据写完以后，会检查 m_status 如果是 e_disconnecting 那么调用 shutdown

当需要关闭的时候，此时的 m_status 应为 e_disconnecting 也就是本端已经关闭，或是 e_connected 还在连接。
那么会调用 m_close_cb ,这是由 TcpServer 传入的回调函数，会在 TcpServer 的map 中删除 该 TcpConnection 
而使用 connect_destroy 在 EpollPoller 中移除监听。


*/
#ifndef MOE_TCPCONNECTION_H
#define MOE_TCPCONNECTION_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/net/CallBack.h>
#include <moe_net/base/String.h>
#include <moe_net/net/SockAddr.h>
#include <moe_net/base/RingBuffer.h>
#include <moe_net/net/HttpContext.h>
#include <memory>



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
    void set_write_complete_cb(const WriteCompleteCallback &cb) { m_write_complete_cb = cb; }
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
    WriteCompleteCallback m_write_complete_cb;
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