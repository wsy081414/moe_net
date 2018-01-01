#include <moe_net/base/LogStream.h>
#include <algorithm>
#include <stdio.h>

namespace moe 
{
namespace aux 
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
const char digitsHex[] = "0123456789ABCDEF";

template <typename T>
size_t num10_to_str(char *buf,T v)
{
    T i = v;
    char *p=buf;
    do
    {
        int tail =static_cast<int>(i%10);
        i/=10;
        *p=zero[tail];
        ++p;
    }
    while(i!=0);

    if(v<0)
    {
        *p ='-';
        p++;
    }
    *p='\0';
    std::reverse(buf,p);
    return p-buf;
}

template <typename T>
size_t num16_to_str(char *buf,T v)
{
    T i = v;
    char *p=buf;
    do
    {
        int tail =static_cast<int>(i%16);
        i/=16;
        *p=digitsHex[tail];
        ++p;
    }
    while(i!=0);

    if(v<0)
    {
        *p ='-';
        p++;
    }
    *p='\0';
    std::reverse(buf,p);
    return p-buf;
}


}
}
using namespace moe;


const int LogStream::max_num_size=32;

template <typename T>
void LogStream::append_num(T v)
{
    if(m_buffer.avail()>max_num_size)
    {
    size_t len=aux::num10_to_str(m_buffer.current(),v);
        m_buffer.add(len);
    }
}


LogStream &LogStream::operator<<(short v)
{
    append_num(v);
    return *this;
}
LogStream &LogStream::operator<<(int v)
{
    append_num(v);
    return *this;
}
LogStream &LogStream::operator<<(unsigned int v)
{
    append_num(v);
    return *this;
}
LogStream &LogStream::operator<<(long v)
{
    append_num(v);
    return *this;
}
LogStream &LogStream::operator<<(unsigned long v)
{
    append_num(v);
    return *this;
}
LogStream &LogStream::operator<<(long long v)
{
    append_num(v);
    return *this;
}
LogStream &LogStream::operator<<(unsigned long long v)  
{
    append_num(v);
    return *this;
}

LogStream &LogStream::operator<<(void *p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if(m_buffer.avail() > max_num_size)
    {
        m_buffer.append("0x",2);
        size_t len = aux::num16_to_str(m_buffer.current(),v);
        m_buffer.add(len);
    }
      return *this;
}

LogStream& LogStream::operator<<(double v)
{
    if(m_buffer.avail() > max_num_size)
    {
        int len = snprintf(m_buffer.current(),max_num_size,"%.12g",v);
        m_buffer.add(len);
    }    
}
