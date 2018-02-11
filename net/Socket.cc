#include <moe_net/net/Socket.h>
#include <moe_net/net/SockOps.h>
#include <moe_net/net/SockAddr.h>

#include <netinet/in.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <errno.h>

using namespace moe;
using namespace moe::net;


Socket::~Socket()
{
    sockops::close(m_fd);
}


void Socket::bind(const SockAddr &addr)
{
    sockops::bind(m_fd,addr.sockaddr());
}

int Socket::accept(SockAddr &addr)
{
    // TRACELOG<<"Socket::accept";
    struct sockaddr_in addr_in;
    bzero(&addr_in,sizeof(addr_in));
    int connfd = sockops::accept(m_fd,&addr_in);
    if(connfd >0)
    {   
        addr.sockaddr(addr_in);
    }
    return connfd;
}

void Socket::listen()
{
    sockops::listen(m_fd);
}
void Socket::shutdown()
{
    sockops::shutdown(m_fd);
}



void Socket::set_no_delay(bool on)
{
    int opt=on?1:0;
    ::setsockopt(m_fd,IPPROTO_TCP,TCP_NODELAY,&opt,static_cast<socklen_t>(sizeof(opt)));
}
void Socket::set_reuse_addr(bool on)
{
    int opt=on?1:0;
    int ret = ::setsockopt(m_fd,SOL_SOCKET,SO_REUSEADDR,&opt,static_cast<socklen_t>(sizeof(opt)));
    TRACELOG<<"set_reuse_addr : "<<ret;
}
void Socket::set_reuse_port(bool on)
{
    int opt=on?1:0;
    int ret = ::setsockopt(m_fd,SOL_SOCKET,SO_REUSEPORT,&opt,static_cast<socklen_t>(sizeof(opt)));
    // 可能失败,
    if(ret < 0)
    {
        TRACELOG<<" Socket::set_reuse_port error :"<<strerror(errno);
    }
}
void Socket::set_keepalive(bool on)
{
    int opt=on?1:0;
    ::setsockopt(m_fd,SOL_SOCKET,SO_KEEPALIVE,&opt,static_cast<socklen_t>(sizeof(opt)));
}