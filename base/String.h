#ifndef MOE_STRING_H
#define MOE_STRING_H


#include <string>
#include <string.h>
#include <utility>
namespace moe 
{


class String 
{

private:
    char *mp_data;
public:
    String()
        :mp_data(new char[1])
    {
        *mp_data='\0';
    }
    String(const char* str)
        :mp_data(new char[strlen(str)+1])
    {
        strcpy(mp_data,str);
    }
    String(const char* str,size_t len)
        :mp_data(new char[len+1])
    {
        strncpy(mp_data,str,len);
        mp_data[len]='\0';
    }
    String(const String &str)
        :mp_data(new char[str.size()+1])
    {
        strcpy(mp_data,str.c_str());
    }
    String(const std::string &str)
        :mp_data(new char[str.size()+1])
    {
        strcpy(mp_data,str.c_str());
    }
    ~String()
    {
        delete[] mp_data;
    }

    // 使用直接传值的方式
    String &operator=(String rhs)
    {
        swap(rhs);
        return *this;
    }
    void swap(String &str)
    {
        std::swap(mp_data,str.mp_data);
    }
    const size_t size() const
    {
        return strlen(mp_data);
    }
    const char* c_str() const
    {
        return mp_data;
    }
         char* c_str() 
    {
        return mp_data;
    }
    bool empty() const 
    {
        return size()==0;
    }

    bool operator==(const String &rhs)
    {
        return ::memcpy(c_str(),rhs.c_str(),size());
    }
    bool operator!=(const String &rhs)
    {
        return !this->operator==(rhs);
    }
};
}


#endif //MOE_STRING_H