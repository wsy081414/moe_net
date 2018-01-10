#ifndef MOE_SOCKET_H
#define MOE_SOCKET_H

#include <moe_net/base/Noncopyable.h>

/*
这个类单纯的分装了 sockfd ,添加了能够设置 sockfd 的函数.

还是获取一个 sockfd 信息,转为字符串的函数.

*/
namespace moe 
{
namespace net {

class SockAddr;

class Socket : aux::Noncopyable
{
public:
    Socket(int fd)
        :m_fd(fd)
    {}
    ~Socket();

    int fd() const  {return m_fd;}

    void bind(const SockAddr &);
    
    void listen();

    // 里面有 accept ,传入的参数会被填充.
    int accept(SockAddr &);

    void shutdown();

    void set_no_delay(bool no);
    void set_reuse_addr(bool no);
    void set_reuse_port(bool no);
    void set_keepalive(bool no);
    
private:
    const int m_fd;

};

}
}




#endif //MOE_SOCKET_H