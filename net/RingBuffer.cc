#include <moe_net/base/RingBuffer.h>

#include <sys/uio.h>

using namespace moe;


int RingBuffer::read_from_fd(int fd,int *err)
{

char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = write_size();
  vec[0].iov_base = begin()+m_write_start;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = readv(fd, vec, iovcnt);
  if (n < 0)
  {
    *err = errno;
  }
  else if (static_cast<size_t>(n) <= writable)
  {
    m_write_start += n;
  }
  else
  {
    m_write_start = mc_buffer.size();
    append(extrabuf, n - writable);
  }
  return n;
} 


const char *RingBuffer::s_crlf="\r\n";
const char *RingBuffer::s_separator=":";


