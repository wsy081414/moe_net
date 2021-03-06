/*
CountDown 类则是封装了计数器，用于协调多线程间工作
linux 中对应的是api 屏障

m_cond.wait() 放在一个 while 中，是因为，可能存在虚假唤醒
可能是信号？

*/

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