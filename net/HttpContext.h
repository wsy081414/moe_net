#ifndef MOE_HTTPCONTEXT_H
#define MOE_HTTPCONTEXT_H


#include <moe_net/base/Timestamp.h>
#include <moe_net/base/RingBuffer.h>
#include <moe_net/net/HttpRequest.h>
#include <moe_net/base/Logger.h>

#include <utility>
namespace moe 
{
namespace net 
{



class HttpContext 
{
public:
    enum ParseState 
    {
        e_parse_line,
        e_parse_header,
        e_parse_body,
        e_parse_over
    };

    bool parse_req(RingBuffer *req,Timestamp time);
    bool parse_line(const char *,const char *);
    bool is_parse_finish() 
    {
        TRACELOG<<"is_parse_finish : "<<(m_status==e_parse_over);
        return m_status==e_parse_over;
    }
    HttpRequest request() {return m_req;}
    void reset()
    {
        m_status=e_parse_line;
        HttpRequest tmp;
        m_req.swap(tmp);
    }
private:
    ParseState m_status;
    HttpRequest m_req;
};
}
}




#endif //MOE_HTTPCONTEXT_H
