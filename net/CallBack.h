// #include <moe_net/net/CallBack.h>

#include <memory>
#include <moe_net/base/Timestamp.h>
#include <moe_net/base/RingBuffer.h>

namespace moe
{
namespace net
{

class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;

// 在连接建立 和连接断开的时候调用的回调函数
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;

// 连接关闭的时候调用的回调函数，主要是在 TcpServer 中移除一个 TcpConnectionPtr
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;

// 当所有需要写的数据写完以后调用的回调函数
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;

// 待发送的数据过多的时候，执行的回调函数
typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterMarkCallback;

// 读取到新数据的时候执行的回调函数
typedef std::function<void(const TcpConnectionPtr &,
                           RingBuffer *,
                           Timestamp)>
    MessageCallback;
}
}