#include <moe_net/net/http/HttpServer.h>
#include <moe_net/net/http/HttpResponse.h>
#include <moe_net/base/Logger.h>



using namespace moe;
using namespace moe::net;

// TcpServer::TcpServer(EventLoop *loop,const std::string& name,const SockAddr& listen_addr,int option)
HttpServer::HttpServer(EventLoop *loop, const std::string &name, const SockAddr &listen, int option)
    :m_server(loop,  name,listen, option)
{
    m_server.set_conn_cb(
        std::bind(&HttpServer::conn_cb, this, std::placeholders::_1));
    m_server.set_msg_cb(
        std::bind(&HttpServer::msg_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

HttpServer::~HttpServer()
{

}

void HttpServer::start()
{
    m_server.start();
}

void HttpServer::conn_cb(const TcpConnectionPtr &conn)
{
    if (conn->is_connected())
    {
        conn->context(HttpContext());
    }
}

void HttpServer::msg_cb(const TcpConnectionPtr &conn, RingBuffer *buf, Timestamp time)
{
    HttpContext *context = conn->context();

    if (!context->parse_req(buf, time))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }else if(context->is_parse_finish())
    {
        request_cb(conn,context->request());
        context->reset();
    }
}

void HttpServer::request_cb(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const std::string connection= req.header("Connection");
    bool close = (connection == "close") || 
                            (req.version()==HttpRequest::e_http10 && connection != "Keep-Alive");
    TRACELOG<<"connection need close: "<<close;
    HttpResponse res(close);

    RingBuffer buf;
    res.build_res(&buf);

    conn->send(&buf);

    if(res.is_need_close())
    {
        conn->shutdown();
    }
}
