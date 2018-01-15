#include <moe_net/net/TcpConnection.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Channel.h>
#include <moe_net/net/SockAddr.h>
#include <moe_net/net/Socket.h>
#include <moe_net/base/String.h>



#include <functional>

using namespace moe;
using namespace moe::net;



// TcpConnection 在初始化的时候已经有了 完整的连接,所以 loacl_addr 和 peer_addr 还有fd 
// 都作为参数
TcpConnection::TcpConnection(EventLoop *loop,const int64_t& index,int fd,
                            const SockAddr local_addr,const SockAddr peer_addr)
    :mp_loop(loop),m_index(index),
    m_local_addr(local_addr),m_peer_addr(peer_addr),
    m_socket(new Socket(fd)),m_channel(new Channel(loop,fd)),
    m_hight_water(64*1024*1024),
    m_status(e_connecting)
{
    m_channel->set_read_cb(
        std::bind(&TcpConnection::handle_read,this,std::placeholders::_1)
    );
    m_channel->set_write_cb(
        std::bind(&TcpConnection::handle_write,this)
    );
    m_channel->set_error_cb(
        std::bind(&TcpConnection::handle_error,this)
    );
    m_channel->set_close_cb(
        std::bind(&TcpConnection::handle_close,this)
    );

    // log 

    m_socket->set_keepalive(true);
}

TcpConnection::~TcpConnection()
{
    assert(m_status == e_disconnected);
}

void TcpConnection::send(const char* buf,size_t len)
{
    TRACELOG<<"TcpConnection::send : "<<(m_status == e_connected);
    // 当连接成功有才能发送
    if(m_status == e_connected)
    {
        mp_loop->add_task(
        std::bind(&TcpConnection::send_run_in_loop,this,String(buf,len))
                );
    }
    TRACELOG<<"TcpConnection::send end";

}

void TcpConnection::send_run_in_loop(const String& str)
{
    TRACELOG<<"TcpConnection::send_run_in_loop:\n "<<str.c_str();
    
    assert(mp_loop->is_in_loop_thread());
    size_t len = str.size();
    
    ssize_t nwrote =0;
    ssize_t remain = len;
    char *msg = const_cast<char *>(str.c_str());

    bool has_fault = false;
    if(m_status == e_disconnected)
    {
        // log
        return ;
    }
    
        TRACELOG<<"TcpConnection::send_run_in_loop  send direct :"<<
                (!m_channel->is_writing() && mc_output.read_size() ==0);
        TRACELOG<<"TcpConnection::send_run_in_loop  m_fd : "<<m_fd<<" channel fd: "<<m_channel->fd();
        
    // 判断 m_channel 也就是 m_fd 可写
    // 可写,并且没有等待写出的数据,那么直接写出
    // 如果 is_writing 也就是在监听写,那么表示有待写的没写出去,因此不会直接写.
    if(!m_channel->is_writing() && mc_output.read_size() ==0)
    {
        
        nwrote = sockops::write(m_channel->fd(),msg,len);
        TRACELOG<<"TcpConnection::send_run_in_loop  write n : "<<nwrote;
        
        if(nwrote >=0)
        {
            remain = len-nwrote;
            if(remain == 0 && m_write_cb)
            {
                // 写完以后,就在loop中调用,问题是,为什么又在loop中?
                mp_loop->add_task(
                    std::bind(m_write_cb,shared_from_this())
                );
            }
        }else{
            // 发生了错误
            nwrote =0;
            // log
            has_fault=true;
        }
    }

        TRACELOG<<"TcpConnection::send_run_in_loop  save send :"<<
                (!has_fault && remain > 0)<<" remain :"<<remain;

    // 没有发生错误,而且上一次写数据没有写完
    // 就将这些数据保存起来
    if(!has_fault && remain > 0)
    {
        size_t hasnt_out=mc_output.read_size();
        // 留存的数据已经达到了预订的大小,那么执行回调函数,并且只执行一次.
        if(hasnt_out + remain >= m_hight_water && 
            hasnt_out < m_hight_water && m_hight_water_cb)
        {
            // m_hight_water_cb();
        }
        mc_output.append(msg+nwrote,remain);

        if(m_channel->is_writing())
        {
            // 使得 sockfd 监听可写.
            // 在epoll中,
            // 其实底层的是,这是个时候发送速度过快,发送缓冲区满了.
            // 所以等待能内核发送数据,当套接字缓冲区可写,也就是可以发送的时候
            // 那么epoll 被唤醒.进行 handle_write
            m_channel->enable_write();
        }
    } 
}

// 我就不明白了,为什么要在 loop 中
void TcpConnection::shutdown()
{
    if(m_status == e_connected)
    {
        status(e_disconnecting);
        mp_loop->add_task(
            std::bind(&TcpConnection::shutdown_run_in_loop,this)
        );
    }
}
void TcpConnection::shutdown_run_in_loop()
{
    assert(mp_loop->is_in_loop_thread());
    if(!m_channel->is_writing())
    {
        m_socket->shutdown();
    }    
}

void TcpConnection::force_close()
{
    if(m_status == e_connecting || m_status == e_connected)
    {
        mp_loop->add_task(
            std::bind(&TcpConnection::force_close_run_in_loop,this)
        );
    }
}

void TcpConnection::force_close_run_in_loop()
{
    assert(mp_loop->is_in_loop_thread());
    if(m_status == e_connecting || m_status == e_connected)
    {
        handle_close();
    }    
}

// 只是监听 可读
void TcpConnection::start_read()
{
    mp_loop->add_task(
        std::bind(&TcpConnection::start_read_run_in_loop,this)
    );
}
void TcpConnection::start_read_run_in_loop()
{
    assert(mp_loop->is_in_loop_thread());
    if(!mb_reading || m_channel->is_reading())
    {
        m_channel->enable_read();
        mb_reading=true;
    }
}

// 不监听 可读
void TcpConnection::stop_read()
{
    mp_loop->add_task(
        std::bind(&TcpConnection::stop_read_run_in_loop,this)
    );
}
void TcpConnection::stop_read_run_in_loop()
{
    assert(mp_loop->is_in_loop_thread());
    if(mb_reading || m_channel->is_reading())
    {
        m_channel->disable_read();
        mb_reading=false;
    }
}


void TcpConnection::handle_read(Timestamp receive_time)
{
    TRACELOG<<"TcpConnection::handle_read";
    assert(mp_loop->is_in_loop_thread());
    int save_errno;

    // 读进 mc_input 中
    int n = mc_input.read_from_fd(m_channel->fd(),&save_errno);
    TRACELOG<<"TcpConnection::handle_read from "<<m_channel->fd()<<" receive : "<<n<<" errno: "<<strerror(errno);
    if(n > 0)
    {
        m_msg_cb(shared_from_this(),&mc_input,receive_time);
    }
    else if(n == 0)
    {
        handle_close();
    }else{
        errno =save_errno;
        // log
        handle_error();
    }
}

// 缓冲区可写的时候
void TcpConnection::handle_write()
{
    assert(mp_loop->is_in_loop_thread());

    if(m_channel->is_writing())
    {
        ssize_t n = sockops::write(m_fd,mc_output.read_ptr(),mc_output.read_size());
        if(n>0)
        {
            mc_output.read_skip(n);
            if(mc_output.read_size() == 0)
            {
                // 写完以后那么,确实不用再监听内核的fd缓冲区可写了
                m_channel->disable_write();

                if(m_write_cb)
                {
                    mp_loop->add_task(
                        std::bind(&TcpConnection::m_write_cb,this)
                    );
                }
                // 表示写完了,就断开连接
                if(m_status == e_disconnecting)
                {
                    shutdown_run_in_loop();
                }
            }
        }else{
            // log
        }
    }else{
        // log
    }
}

void TcpConnection::handle_close()
{
    TRACELOG<<"TcpConnection::handle_close()";
    assert(mp_loop->is_in_loop_thread());

    // log

    assert(m_status == e_connecting || m_status  ==  e_disconnected);
    status(e_disconnected);
    m_channel->disable_all();
    // m_channel->remove();

    TcpConnectionPtr guard(shared_from_this());
    //  muduo 还调用了 m_conn_cb ,为啥?
    m_close_cb(guard);
}

void TcpConnection::handle_error()
{
    // log一下
}

// connect_establised 只是监听读
void TcpConnection::connect_establised()
{
    status(e_connected);
    assert(mp_loop->is_in_loop_thread());
    m_channel->enable_read();    
}

// 只是 不在监听读,从 epoll 移除
void TcpConnection::connect_destroy()
{
    // TRACELOG<<"TcpConnection::connect_destroy()";
    assert(mp_loop->is_in_loop_thread());
    if(m_status == e_connected)
    {
        status(e_disconnected);
        m_channel->disable_all();
        m_conn_cb(shared_from_this());
    }
    m_channel->remove();
}


void TcpConnection::send(RingBuffer *buf)
{
    if(m_status ==e_connected)
    {
        mp_loop->add_task(
            std::bind(&TcpConnection::send_run_in_loop,this,buf->readall_as_string())
        );
    }
}
