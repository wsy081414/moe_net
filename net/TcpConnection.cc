#include <moe_net/net/TcpConnection.h>
#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Channel.h>
#include <moe_net/net/SockAddr.h>
#include <moe_net/net/Socket.h>

#include <functional>

using namespace moe;
using namespace moe::net;




TcpConnection::TcpConnection(EventLoop *loop,const String& name,int fd,const SockAddr local_addr,const SockAddr peer_addr)
    :mp_loop(loop),m_name(name),m_local_addr(local_addr),m_peer_addr(peer_addr),
    m_socket(new Socket(fd)),m_channel(new Channel(loop,fd)),
    m_hight_water(64*1024*1024),m_status(e_connecting)
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

    // set socket
}

TcpConnection::~TcpConnection()
{
    assert(m_status == e_disconnected);
}

void TcpConnection::send(const char* buf,size_t len)
{
    mp_loop->add_task(
        std::bind(&TcpConnection::send_run_in_loop,this,buf,len)
    );
}

void TcpConnection::send_run_in_loop(const char* msg,size_t len)
{
    assert(mp_loop->is_in_loop_thread());
    ssize_t nwrote =0;
    ssize_t remain = len;

    bool has_fault = false;
    if(m_status == e_disconnected)
    {
        // log
        return ;
    }
    
    // 可写,并且没有等待写出的数据,那么直接写出
    if(m_channel->is_writing() && mc_output.read_size() ==0)
    {
        nwrote = sockops::write(m_fd,msg,len);
        if(nwrote >=0)
        {
            remain = len-nwrote;
            if(remain == 0 && m_write_cb)
            {
                // m_write_cb();
            }
        }else{
            // nwrote <0
            nwrote =0;
            // log
            has_fault=true;
        }
    }

    // 没有发生错误,而且上一次写数据没有写完
    // 就将这些数据保存起来
    if(!has_fault && remain > 0)
    {
        size_t hasnt_out=mc_output.read_size();
        // 留存的数据已经达到了预订的大小,那么执行回调函数,并且只执行一次.
        if(hasnt_out + remain >= m_hight_water && hasnt_out < m_hight_water && m_hight_water_cb)
        {
            // m_hight_water_cb();
        }
        mc_output.append(msg+nwrote,remain);

        if(m_channel->is_writing())
        {
            m_channel->enable_write();
        }
    }
}

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
    assert(mp_loop->is_in_loop_thread());
    int save_errno;

    size_t n = mc_input.read_from_fd(m_channel->fd(),&save_errno);
    if(n<0)
    {
        // 没写
    }else if(n==0)
    {
        handle_close();
    }else{
        errno =save_errno;
        // log
        handle_error();
    }

}

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
                m_channel->disable_write();
                if(m_write_cb)
                {
                    mp_loop->add_task(
                        std::bind(&TcpConnection::m_write_cb,this)
                    );
                }
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
    assert(mp_loop->is_in_loop_thread());

    // log


    assert(m_status == e_connecting || m_status  ==  e_disconnected);
    status(e_disconnected);
    m_channel->disable_all();
    m_channel->remove();

    // 没看懂
}

void TcpConnection::handle_error()
{
    // log
}
