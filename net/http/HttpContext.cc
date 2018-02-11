#include <moe_net/net/http/HttpContext.h>
#include <moe_net/base/String.h>

#include <moe_net/base/Logger.h>
#include <algorithm>

using namespace moe;
using namespace moe::net;

// 解析一个 请求报文 的函数
bool HttpContext::parse_req(RingBuffer *buf, Timestamp time)
{
    
    bool ok = true;
    bool has_more = true;

    while (has_more)
    {
        // 首先是去解析 请求行 ,主要包括了请求方法,url和协议版本
        // 请求行
        if (m_status == e_parse_line)
        {
            const char *crlf = buf->find_crlf();
            
            if (crlf)
            {
                // 这里具体的去解析请求行.
                ok = parse_line(buf->read_ptr(), crlf);
                if (ok)
                {
                    buf->read_skip_until(crlf+2);
                    // 请求行解析成功,表示收到了一个有用的请求报文

                    // 然后去解析 请求头
                    m_status = e_parse_header;
                }
                else
                {
                    has_more = false;
                }
            }
            else
            {
                has_more = false;
            }
        }
        else if (m_status == e_parse_header)
        {
            // 请求行解析完成后,去解析 请求头
            const char *crlf = buf->find_crlf();
            if (crlf)
            {
                const char *separator = std::find(static_cast<const char*>(buf->read_ptr()),crlf,':');
                
                if (separator != crlf)
                {
                    // 具体的讲请求头和请求行进行解析的是 下面这个函数
                    m_req.header(buf->read_ptr(), separator, crlf);
                }
                else
                {
                m_status = e_parse_over;
                    
                    has_more = false;
                }
                buf->read_skip_until(crlf+2);
            }
            else
            {
                m_status = e_parse_over;
                
                has_more = false;
            }
        }
        else if (m_status == e_parse_body)
        {
        }
    }
    buf->read_skip_all();
    return ok;
}

bool HttpContext::parse_line(const char *start, const char *end)
{
    bool ok = false;
    const char *tmp_start = start;
    const char *space = std::find(tmp_start, end, ' ');

    
    if (space != end && m_req.method(tmp_start, space))
    {
        tmp_start = space + 1;
        space = std::find(tmp_start, end, ' ');
        if (space != end)
        {

            const char *url_separator = std::find(tmp_start, space, '?');

            if (url_separator != space)
            {
                m_req.path(tmp_start, url_separator);
                m_req.query(url_separator, space);
            }
            else
            {
                m_req.path(tmp_start, space);
            }

            tmp_start = space + 1;

            ok = ((end - tmp_start) == 8) && std::equal(tmp_start, end - 1, "HTTP/1.");

            if (*(end - 1) == '1')
            {
                m_req.version(HttpRequest::e_http11);
            }
            else if (*(end - 1) == '0')
            {
                m_req.version(HttpRequest::e_http10);
            }
            else
            {
                ok = false;
            }
        }
    }
    return ok;
}
