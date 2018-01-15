#ifndef MOE_HTTPREQUEST_H
#define MOE_HTTPREQUEST_H

#include <map>
#include <string>
#include <moe_net/base/Timestamp.h>
#include <utility>

#include <moe_net/base/Logger.h>

namespace moe
{
namespace net
{
class HttpRequest
{
  public:
    enum Method
    {
        e_invalid,
        e_get,
        e_post,
        e_head,
        e_put,
        e_delete
    };

    enum Version
    {
        e_unknow,
        e_http10,
        e_http11
    };

    HttpRequest()
        : m_method(e_invalid), m_version(e_unknow)
    {
    }

    void version(Version v) { m_version = v; }
    Version version() const { return m_version; }
    void method(Method v) { m_method = v; }

    bool method(const char *start, const char *end)
    {
        std::string v(start, end);
        TRACELOG<<"HttpRequest::method : "<<v.c_str()<< " "<<(v == "GET") ;
        if (v == "GET")
        {
            m_method = e_get;
        }
        else if (v == "POST")
        {
            m_method = e_post;
        }
        else if (v == "HEAD")
        {
            m_method = e_head;
        }
        else if (v == "PUT")
        {
            m_method = e_put;
        }
        else if (v == "DELETE")
        {
            m_method = e_delete;
        }
        else
        {
            m_method = e_invalid;
        }

        return m_method != e_invalid;
    }

    const char *method_to_string() const
    {
        const char *result = "UNKNOWN";
        switch (m_method)
        {
        case e_get:
            result = "GET";
            break;
        case e_post:
            result = "POST";
            break;
        case e_head:
            result = "HEAD";
            break;
        case e_put:
            result = "PUT";
            break;
        case e_delete:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }

    void path(const char *start, const char *end)
    {
        m_path.assign(start, end);
    }
    void query(const char *start, const char *end)
    {
        m_query.assign(start, end);
    }

    void receiv_time(Timestamp t)
    {
        m_time = t;
    }
    Timestamp receiv_time()
    {
        return m_time;
    }

    // 传入了一行的开始,结束还有中间的 : 分隔符的位置,用来构建一个 请求头的键值对
    // 这里主要要要处理的就是 空格的问题
    void header(const char *start, const char *separator, const char *end)
    {
        
        std::string key(start,separator);
        ++separator;

        while(separator<=end && *separator==' ')
        {
            
            ++separator;
            
        }
    
        while(separator<=end && *end==' ')
        {
            
            --end;
        }
        mc_res[key] = separator==end?std::string():std::string(separator,end);
    }

    const std::string header(const std::string &key) const
    {
        std::map<std::string, std::string>::const_iterator it = mc_res.find(key);
        return it == mc_res.end() ? std::string() : it->second;
    }

    void swap(HttpRequest &rhs)
    {
        std::swap(m_method, rhs.m_method);
        std::swap(m_version, rhs.m_version);
        m_path.swap(rhs.m_path);
        m_query.swap(rhs.m_query);

        m_time.swap(rhs.m_time);
        mc_res.swap(rhs.mc_res);
    }

  private:
    Method m_method;
    Version m_version;
    std::string m_path;
    std::string m_query;
    Timestamp m_time;
    std::map<std::string, std::string> mc_res;
};
}
}

#endif //MOE_HttpRequest_H