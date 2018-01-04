#ifndef MOE_TIMER_H
#define MOE_TIMER_H


#include <moe_net/base/Atomic.h>
#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/Timestamp.h>

#include <functional>

namespace moe 
{
namespace net 
{

class Timer : aux::Noncopyable
{
public:
    typedef std::function<void ()> TimerCallBack;
private:
    TimerCallBack m_cb;
    int m_interval;
    Timestamp m_expiration;
    const int64_t m_index;

    static Atomic64 timer_index;
public:

    Timer(const TimerCallBack& cb,Timestamp when,int interval = 0)
        :m_cb(cb),m_expiration(when),m_interval(interval),m_index(timer_index.inc_get())
    {}
    void run()
    {
        m_cb();
    }
    void restart();
    Timestamp expiration() const
    {
        return m_expiration;
    }
    bool repeat()
    {
        return m_interval!=0;
    }
    int64_t index()
    {
        return m_index;
    }
};



}
}

#endif //MOE_TIMER_H