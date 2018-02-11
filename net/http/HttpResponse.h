#ifndef MOE_HTTPRESPONSE_H
#define MOE_HTTPRESPONSE_H

#include <string>
#include <map>
#include <moe_net/net/RingBuffer.h>

namespace moe
{
namespace net
{

// class RingBuffer;

class HttpResponse
{
  public:
    enum HttpStatus
    {
        e_unknow,
        e_ok = 200,
        e_permanntly_move = 301,
        e_bad_requset = 400,
        e_not_fount = 404
    };

    explicit HttpResponse(bool close)
        : m_status(e_unknow), mb_close(close)
    {
    }

    void status(HttpStatus v) { m_status = v; }
    void set_close(bool on) { mb_close = on; }

    void header(const std::string &key, const std::string &value)
    {
        mc_header[key] = value;
    }

    void set_content_type(const std::string &content)
    {
        header("Content-Type", content);
    }

    

    void set_body(const std::string &body)
    {
        m_body = body;
    }
    void set_status_msg(const std::string &msg)
    {
        m_msg = msg;
    }
    void build_res(RingBuffer *);
    bool is_need_close() {return mb_close;}

  private:
    HttpStatus m_status;
    bool mb_close;

    std::map<std::string, std::string> mc_header;
    std::string m_body;
    std::string m_msg;
};
}
}

#endif //MOE_HTTPRESPONSE_H