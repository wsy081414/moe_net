#include <moe_net/net/Socket.h>
#include <moe_net/net/SockOps.h>
#include <moe_net/net/SockAddr.h>

#include <netinet/in.h>

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
