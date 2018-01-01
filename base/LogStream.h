#ifndef MOE_LOGSTREAM_H
#define MOE_LOGSTREAM_H

#include <moe_net/base/Timestamp.h>
#include <moe_net/base/Noncopyable.h>
#include <string.h>
#include <stdio.h>
namespace moe
{
namespace aux
{

const int small_buffer_size = 4000;
const int big_buffer_size = 4000 * 1000;

template <int Size>
class Buffer : aux::Noncopyable
{
  private:
    char m_data[Size];
    char *mp_cur;

  public:
    Buffer()
        : mp_cur(m_data)
    {
    }
    ~Buffer()
    {
    }

    void append(const char *buf, size_t len)
    {
        if (avail() > len)
        {
            memcpy(mp_cur, buf, len);
            mp_cur += len;
        }
    }
    void append(char v)
    {
        memcpy(mp_cur,&v,1);
        mp_cur+=1;
    }

    const char *data() { return m_data; }
    size_t size() { return static_cast<size_t>(mp_cur - m_data); }
    size_t avail() { return static_cast<size_t>(end() - mp_cur); }
    const char *end() { return (m_data + sizeof(m_data)); }
    char *current() {return mp_cur;}
    void reset() { mp_cur = m_data; }
    void bzero() { ::bzero(m_data, sizeof(m_data)); }
    String to_sting() { return String(&m_data[0], size()); }
    // 只为添加数字而用的。
    void add(size_t len) {mp_cur +=len;}
    void print() {printf("%s",to_sting().c_str());}
};
}

class LogStream : aux::Noncopyable
{
  private:
    aux::Buffer<aux::small_buffer_size> m_buffer;

    template<typename T>
    void append_num(T);

    static const int max_num_size;
  public:
    LogStream &operator<<(bool v)
    {
        m_buffer.append(v ? '1' : '0');
        return *this;
    }
    LogStream &operator<<(const char v)
    {
        m_buffer.append(&v, 1);
        return *this;
    }
    LogStream &operator<<(const char *str)
    {
        if (str != nullptr)
        {
            m_buffer.append(str, strlen(str));
        }
        else
        {
            m_buffer.append("(null)", 6);
        }
        return *this;
    }
    LogStream &operator<<(const String str)
    {
        m_buffer.append(str.c_str(),str.size());
        return *this;
    }


    LogStream &operator<<(short);
    LogStream &operator<<(int);
    LogStream &operator<<(unsigned int);
    LogStream &operator<<(long);
    LogStream &operator<<(unsigned long);
    LogStream &operator<<(long long);
    LogStream &operator<<(unsigned long long);
    LogStream &operator<<(void *);
    LogStream &operator<<(double );



    void append(const char *str,size_t len)
    {
        m_buffer.append(str,len);
    }
    void reset(){m_buffer.reset();}
    void print(){m_buffer.print();}

};
}

#endif //MOE_LOGSTREAM_H