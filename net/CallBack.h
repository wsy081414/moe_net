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
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterMarkCallback;
typedef std::function<void(const TcpConnectionPtr &,
                           RingBuffer *,
                           Timestamp)>
    MessageCallback;
}
}