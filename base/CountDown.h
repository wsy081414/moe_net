#ifndef MOE_COUNTDOWN_H
#define MOE_COUNTDOWN_H


#include <moe_net/base/Mutex.h>
#include <moe_net/base/Condition.h>
#include <moe_net/base/Noncopyable.h>

namespace moe 
{

class CountDown : aux::Noncopyable
{
private:
    int m_counts;
    mutable Mutex m_mutex;
    Condition m_cond;
public:
    explicit CountDown(int counts)
        :m_counts(counts),m_mutex(),m_cond(m_mutex)
    {}

    void wait()
    {
        MutexLock lock(m_mutex);
        while(m_counts>0)
        {
            m_cond.wait();
        }
    }
    void count_down()
    {
        MutexLock lock(m_mutex);
        --m_counts;
        if(m_counts == 0)
        {
            m_cond.notify_all();
        }
    }
    int counts() const
    {
        MutexLock lock(m_mutex);
        return m_counts;
    }

};
}

#endif //MOE_COUNTDOWN_H