#ifndef MOE_SOCKET_H
#define MOE_SOCKET_H

#include <moe_net/base/Noncopyable.h>


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
    int accept(SockAddr &);
    void shutdown();
private:
    const int m_fd;

};

}
}




#endif //MOE_SOCKET_H