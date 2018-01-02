#ifndef MOE_SOCKOPS_H
#define MOE_SOCKOPS_H

#include <unistd.h>

namespace moe
{
namespace net
{
namespace sockops 
{
inline ssize_t read(int sockfd, void *buf, size_t count)
{
    return ::read(sockfd,buf,count);
}
// ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt)

inline ssize_t write(int sockfd, const void *buf, size_t count)
{
    return ::write(sockfd,buf,count);
}

}



}
}

#endif // MOE_SOCKOPS_H