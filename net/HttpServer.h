#ifndef MOE_HTTPSERVE_H
#define MOE_HTTPSERVE_H

#include <functional>
#include <string>
#include <moe_net/net/TcpServer.h>

namespace moe
{
namespace net
{

class HttpResponse;
class HttpRequest;

class HttpServer
{
  public:
    typedef std::function<void(const HttpRequest &,HttpResponse *)>  HttpCallback;

    HttpServer(EventLoop *loop,const std::string& name,const SockAddr& listen,int option);
    
    ~HttpServer();

    EventLoop *loop();

    void set_http_cb(const HttpCallback& cb) {m_http_cb =cb;}
    void set_threads(int i) {m_server.set_threads(i);}

    void start();

private:
    void conn_cb(const TcpConnectionPtr& conn);
    void msg_cb(const TcpConnectionPtr& conn,RingBuffer *buf,Timestamp time);
    void request_cb(const TcpConnectionPtr& conn, const HttpRequest& req);

    TcpServer m_server;
    HttpCallback m_http_cb;
};
}
}

#endif //MOE_HTTPSERVE_H