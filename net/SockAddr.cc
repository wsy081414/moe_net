#include <moe_net/net/SockAddr.h>
#include <string.h>
#include <netdb.h>





namespace moe 
{
    namespace aux 
    {
        const int s_ip_any = INADDR_ANY;
        const int s_ip_lo = INADDR_LOOPBACK;
    }
}

using namespace moe;
using namespace moe::net;


SockAddr::SockAddr(uint16_t port,bool lo,bool ipv6)
{
    if(ipv6 ==true)
    {

    }else{
        bzero(&m_addr,sizeof(struct sockaddr_in));
        in_addr_t ip=lo?aux::s_ip_lo:aux::s_ip_any;
        m_addr.sin_addr.s_addr=htonl(ip);
        m_addr.sin_port=htonl(port);
    }
}

SockAddr::SockAddr(String ip,uint16_t port,bool ipv6)
{
    if(ipv6==true)
    {
        
    }else{
        bzero(&m_addr,sizeof(struct sockaddr_in));
        sockops::fill_addr(ip.c_str(),port,&m_addr);
    }
}


String SockAddr::to_ip()
{
    char buf[64] = "";
    sockops::to_ip(buf,sizeof(buf),sockaddr());
    return buf;
}
String SockAddr::to_ip_port()
{
    char buf[64] = "";
    sockops::to_ip_port(buf,sizeof(buf),sockaddr());
    return buf;
}