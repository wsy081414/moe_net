#ifndef MOE_RINGBUFFER_H
#define MOE_RINGBUFFER_H

#include <string.h>
#include <vector>
#include <assert.h>
#include <unistd.h>
#include <moe_net/base/String.h>
#include <algorithm>

namespace moe
{

class RingBuffer
{
  private:
    std::vector<char> mc_buffer;
    size_t m_read_start;
    size_t m_write_start;

    static const int s_prepend = 64;

  public:
    explicit RingBuffer(size_t size = 1024)
        : mc_buffer(size), m_read_start(s_prepend), m_write_start(s_prepend)
    {
    }

    void swap(RingBuffer &rhs)
    {
        mc_buffer.swap(rhs.mc_buffer);
        std::swap(m_read_start, rhs.m_read_start);
        std::swap(m_write_start, rhs.m_write_start);
    }

    size_t read_size()
    {
        return m_write_start - m_read_start;
    }
    size_t write_size()
    {
        return mc_buffer.size() - m_write_start;
    }
    const char *read_ptr() const
    {
        return begin() + m_read_start;
    }

    char *read_ptr()
    {
        return begin() + m_read_start;
    }

    const char *read_ptr() const
    {
        return begin() + m_read_start;
    }

    char *write_ptr()
    {
        return begin() + m_write_start;
    }

    const char *write_ptr() const
    {
        return begin() + m_write_start;
    }

    void read_skip(size_t len)
    {
        assert(len <= read_size());
        if (len < read_size())
        {
            m_read_start += len;
        }
        else
        {
            read_skip_all();
        }
    }
    void read_skip_until(const char *end)
    {
        assert(read_ptr() <= end);
        assert(write_ptr() >= end);

        read_skip(end - read_ptr());
    }
    void read_skip_all()
    {
        m_read_start = s_prepend;
        m_write_start = s_prepend;
    }

    void write_skip(size_t len)
    {
        assert(len <= write_size());
        m_write_start += len;
    }

    String to_string(size_t len)
    {
        assert(len <= read_size());
        String ret(read_ptr(), len);
        read_skip(len);
        return ret;
    }

    void append(const char *buf, size_t len)
    {
        ensure_write(len);
        std::copy(buf, buf + len, write_ptr());
        write_skip(len);
    }
    void append(const void *buf, size_t len)
    {
        append(static_cast<const char *>(buf), len);
    }
    void append(const String &str)
    {
        append(str.c_str(), str.size());
    }

    void prepend(const char *buf, size_t len)
    {
        assert(prepend_size() >= len);
        m_read_start -= len;
        std::copy(buf, buf + len, begin() + m_read_start);
    }

    void ensure_write(size_t len)
    {
        if (write_size() < len)
        {
            re_ring(len);
        }
        assert(write_size() >= len);
    }
    size_t prepend_size()
    {
        return m_read_start;
    }
    int read_from_fd(int fd, int *err);
    
    /*
    CRLF表示句尾使用回车换行两个字符(即我们常在Windows编程时使用"\r\n"换行)
    LF表示表示句尾，只使用换行.
    CR表示只使用回车.
    */
    const char *find_crlf(const char *start)
    {
        assert(read_ptr() <= start);
        assert(write_ptr() >= start);

        const char *crlf = std::search(start, static_cast<const char *>(write_ptr()), s_crlf, s_crlf + 2);

        return crlf == write_ptr() ? nullptr : crlf;
    }

    const char *find_crlf()
    {
        const char *crlf = std::search(static_cast<const char *>(read_ptr()), static_cast<const char *>(write_ptr()), s_crlf, s_crlf + 2);
        assert(crlf <= write_ptr());
        return crlf == write_ptr() ? nullptr : crlf;
    }

    // const char *find(const char *start,const char *end,const char *target,int len)
    // {
    //     const char *index = std::search(start,end,target,target+1);
    //     assert(index<end);
    //     return index==end?nullptr:index;
    // }

    String readall_as_string()
    {
        String all(read_ptr(), read_size());
        read_skip_all();
        return all;
    }

    static const char *s_separator;

  private:
    char *begin()
    {
        return &*mc_buffer.begin();
    }

    const char *begin() const
    {
        return &*mc_buffer.begin();
    }

    void re_ring(size_t len)
    {
        if (write_size() + prepend_size() - s_prepend < len)
        {
            mc_buffer.resize(m_write_start + len);
        }
        else
        {
            size_t readable = read_size();
            std::copy(begin() + m_read_start, begin() + m_write_start, begin() + s_prepend);
            m_read_start = s_prepend;
            m_write_start = s_prepend + readable;
            assert(write_size() >= len);
        }
    }

    static const char *s_crlf;
};
}

#endif // MOE_RINGBUFFER_H