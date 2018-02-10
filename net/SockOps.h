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
#include <moe_net/base/Logger.h>
#include <assert.h>

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

inline const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr)
{
    return static_cast<const struct sockaddr_in *>(reinterpret_cast<const void *>(addr));
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
        FATAlLOG << "sockets::nonblock_fd create error ";
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
        FATAlLOG << "sockets::bind fd: " << fd << " error: " << strerror(errno);
    }
}
inline void listen(int fd)
{
    int ret = ::listen(fd, SOMAXCONN);
    if (ret < 0)
    {
        FATAlLOG << "sockets::listen fd: " << fd << " error: " << strerror(errno);
    }
}

/*
EAGAIN:
客户通过Socket提供的send函数发送大的数据包时，就可能返回一个EAGAIN的错误,
错误产生的原因是由于send 函数中的size变量大小超过了tcp_sendspace的值:
1.  调大tcp_sendspace，使之大于send中的size参数 
2.  在调用send前，在setsockopt函数中为SNDBUF设置更大的值 
3.  使用write替代send，因为write没有设置O_NDELAY或者O_NONBLOCK

在非阻塞模式下调用了阻塞操作，在该操作没有完成就返回这个错误，
这个错误不会破坏socket的同步，不用管它，下次循环接着recv就可以


EINTR:
指操作被中断唤醒，需要重新读/写
read返回-1

*/

inline int accept(int fd, struct sockaddr_in *addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    // accept4 比accept多了一个flag位，表示对返回的文件描述符，设置标志
    int connfd = ::accept4(fd, sockaddr_cast(addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd < 0)
    {
        int errno_save = errno;
        switch (errno_save)
        {
            // 这一部分不需要处理，直接重新阻塞就行
        case EAGAIN:       // 对非阻塞套接字进行读取，但是没有数据，因此会返回这个错误
        case ECONNABORTED: //ECONNABORTED通常发生在重传（一定次数）失败后，强制关闭套接字
        case EINTR:        //指操作被中断唤醒，需要重新读/写
        case EPROTO:       // 协议错误。协议错误怎么传过来的？
        case EPERM:        // 防火墙规则不允许
        case EMFILE:       // 预处理？哈？达到最大文件描述符大小
            errno = errno_save;
            break;
        case EBADF:  //sockfd 不是一个文件描述符
        case EFAULT: // addr 不可写
        case EINVAL: //sockfd 不是一个监听套接字，或是addrlen 不可写
        case ENFILE: //系统宽度限制的文件描述符大小，达到最大文件描述符大小

        case ENOBUFS:    //内存上限，不能再为新的连接分配缓冲区
        case ENOMEM:     //同上
        case ENOTSOCK:   // sockfd 不是一个socket套接字
        case EOPNOTSUPP: //sockfd 不是 SOCK_STREAM
            FATAlLOG << "sockets::accept  error: " << strerror(errno_save);
            break;
        default:
            FATAlLOG << "sockets::accept error:" << strerror(errno_save);
            break;
        }
    }

    return connfd;
}

inline void close(int fd)
{
    if (::close(fd) < 0)
    {
        ERRORLOG<<"sockets::close error";
    }
}

inline void shutdown(int fd)
{
    if (::shutdown(fd, SHUT_WR) < 0)
    {
        ERRORLOG<<"sockets::shutdown error";
    }
}

inline void fill_addr(char *ip, uint16_t port, struct sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        ERRORLOG << "sockets::inet_pton error";
    }
    char buf[64];
}

inline int sock_error(int fd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

inline struct sockaddr_in peer_addr(int sockfd)
{
    sockaddr_in peer_addr;
    bzero(&peer_addr, sizeof(peer_addr));

    socklen_t addrlen = static_cast<socklen_t>(sizeof peer_addr);
    if (::getpeername(sockfd, sockaddr_cast(&peer_addr), &addrlen) < 0)
    {
        ERRORLOG << "sockets::local_addr error";
    }

    return peer_addr;
}

inline struct sockaddr_in local_addr(int sockfd)
{
    sockaddr_in local_addr;
    bzero(&local_addr, sizeof(local_addr));

    socklen_t addrlen = static_cast<socklen_t>(sizeof local_addr);
    if (::getpeername(sockfd, sockaddr_cast(&local_addr), &addrlen) < 0)
    {
        ERRORLOG << "sockets::local_addr error";
    }

    return local_addr;
}

inline void to_ip(char *buf, size_t size, const struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET)
    {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
}
inline void to_ip_port(char *buf, size_t size, const struct sockaddr *addr)
{
    to_ip(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    uint16_t port = ntohs(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}
}
}
}

#endif // MOE_SOCKOPS_H