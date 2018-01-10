#ifndef MOE_SOCKADDR_H
#define MOE_SOCKADDR_H

#include <moe_net/base/String.h>
#include <moe_net/net/SockOps.h>
#include <netinet/in.h>



namespace moe 
{
namespace net 
{


class SockAddr 
{
public:
    SockAddr(uint16_t port=0,bool lo=false  ,bool ipv6 = false);
    SockAddr(String ip,uint16_t port,bool ipv6=false);
    explicit SockAddr(const sockaddr_in& addr)
        :m_addr(addr)
    {}

    sa_family_t family() const {return m_addr.sin_family;}
    String to_ip();
    String to_ip_port();

    const struct sockaddr *sockaddr() const   {return sockops::sockaddr_cast(&m_addr);}
    void sockaddr(struct sockaddr_in &addr ){ m_addr = addr;}
private:
    union{
    struct sockaddr_in m_addr;
    struct sockaddr_in6 m_addr6;
    };
};
}
}


#endif //MOE_SOCKOPS_H