#ifndef MOE_TIMERQUEUE_H
#define MOE_TIMERQUEUE_H

#include <moe_net/net/Timer.h>
#include <moe_net/base/Noncopyable.h>

#include <moe_net/base/Mutex.h>
#include <moe_net/base/Condition.h>
#include <moe_net/net/Channel.h>

#include <map>
#include <set>

#include <vector>


namespace moe
{
namespace net
{


class TimerQueue : aux::Noncopyable
{
public:
    typedef Timer::TimerCallBack TimerCallBack;
    explicit TimerQueue(EventLoop*);
    ~TimerQueue();
private:
    EventLoop *mp_loop;

    const int m_timer_fd;
    Channel m_timer_channel;

    typedef std::map<Timestamp,Timer*> TimerMapRun;
    TimerMapRun mc_timers;

    typedef std::map<int64_t,TimerMapRun::iterator> TimerMapRunDel;
    TimerMapRunDel mc_timers_for_del;
    std::set<int64_t> mc_timers_will_del;
    
    // Mutex m_mutex;
    // Condition m_cond;
    bool mb_handling;

    void handle_interval(const std::vector<Timer*>& expired_timers,Timestamp now);
    std::vector<Timer*> get_expired(Timestamp now);
    void handle_expiration();

    int64_t readd(Timer *);


public:
    // int64_t add_timer(const Timer& );
    // void add_timer(const Timer::TimerCallBack& );
    
    void cancel_timer(int64_t index);
    int64_t add_timer(const TimerCallBack& cb,int64_t when,bool repeat);

    // int64_t add_timer(Timer &timer);




};


}
}


#endif //MOE_TIMERQUEUE_H