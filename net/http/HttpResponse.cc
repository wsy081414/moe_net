#include <moe_net/net/http/HttpResponse.h>


#include <stdio.h>
using namespace moe;
using namespace moe::net;

void HttpResponse::build_res(RingBuffer *res_buf)
{

    m_status=e_ok;
    m_msg ="OK";


    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", m_status);
    res_buf->append(buf);

    res_buf->append(m_msg);
    res_buf->append("\r\n");

    if (mb_close)
    {
        res_buf->append("Connection: close\r\n");
    }
    else
    {
        m_body="<html>hello world\n";
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", m_body.size());
        res_buf->append(buf);
        res_buf->append("Connection: Keep-Alive\r\n");
    }

    for (std::map<std::string, std::string>::const_iterator it = mc_header.begin();
         it != mc_header.end();
         ++it)
    {
        res_buf->append(it->first);
        res_buf->append(": ");
        res_buf->append(it->second);
        res_buf->append("\r\n");
    }

    res_buf->append("\r\n");
    res_buf->append(m_body);
}
