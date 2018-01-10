#ifndef MOE_SOCKOPS_H
#define MOE_SOCKOPS_H

#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include <fcntl.h>
#include <stdio.h>
#include <moe_net/base/String.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>


namespace moe
{
namespace net
{
namespace sockops
{

inline const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr)
{
    return static_cast<const struct sockaddr *>(reinterpret_cast<const void *>(addr));
}
inline struct sockaddr *sockaddr_cast(struct sockaddr_in *addr)
{
    return static_cast<struct sockaddr *>(reinterpret_cast<void *>(addr));
}

inline struct sockaddr_in *sockaddr_in_cast(struct sockaddr *addr)
{
    return static_cast<struct sockaddr_in *>(reinterpret_cast<void *>(addr));
}

inline ssize_t read(int sockfd, void *buf, size_t count)
{
    return ::read(sockfd, buf, count);
}
// ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt)

inline ssize_t write(int sockfd, const void *buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

inline int nonblock_fd(sa_family_t type)
{
    int ret = ::socket(type, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (ret < 0)
    {
        // log
    }
    return ret;
}

inline int connect(int fd, const struct sockaddr *addr)
{
    return ::connect(fd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}
inline int bind(int fd, const struct sockaddr *addr)
{
    int ret = ::bind(fd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if (ret < 0)
    {
        //  log
    }
}
inline void listen(int fd)
{
    int ret = ::listen(fd,SOMAXCONN);
    if(ret<0)
    {
        // log
    }
}

inline int accept(int fd, struct sockaddr_in *addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    // accept4 比accept多了一个flag位，表示对返回的文件描述符，设置标志
    int connfd = ::accept4(fd, sockaddr_cast(addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd < 0)
    {
        // 大工程的判断
        // 以后写
    }

    return connfd;
}

inline void close(int fd)
{
    if (::close(fd) < 0)
    {
        // log
    }
}

inline void shutdown(int fd)
{
    if (::shutdown(fd, SHUT_WR) < 0)
    {
        //   log
    }
}

inline void fill_addr(char *ip,uint16_t port, struct sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htonl(port);
    if (::inet_pton(AF_INET,ip, &addr->sin_addr) <= 0)
    {
        // log
    }
}

inline int sock_error(int fd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }else{
        return optval;
    }
}

inline struct sockaddr_in peer_addr(int fd)
{
    sockaddr_in tmp;
    bzero(&tmp,sizeof(tmp));
    return tmp;
}

inline struct sockaddr_in local_addr(int fd)
{
    sockaddr_in tmp;
    bzero(&tmp,sizeof(tmp));
    
    return tmp;
}


}
}
}

#endif // MOE_SOCKOPS_H