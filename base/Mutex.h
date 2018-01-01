#ifndef MOE_MUTEX_H
#define MOE_MUTEX_H

#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/Debug.h>
#include <pthread.h>
#include <moe_net/base/Everythread.h>
#include <assert.h>

namespace moe 
{



class Mutex : aux::Noncopyable
{
private:
    pthread_mutex_t m_mutex;
    pid_t m_holder;
public:
    Mutex()
        :m_holder(0)
    {
        zero_is_ok(pthread_mutex_init(&m_mutex,NULL))
    }

    ~Mutex()
    {
        assert(m_holder == 0);
        zero_is_ok(pthread_mutex_destroy(&m_mutex));
    }

    void lock()
    {
        zero_is_ok(pthread_mutex_lock(&m_mutex));
        after_lock();
    }
    void unlock()
    {
        before_unlock();
        zero_is_ok(pthread_mutex_unlock(&m_mutex));
    }

    pthread_mutex_t *mutex_ptr()
    {
        return &m_mutex;
    }
private:
    void after_lock()
    {
        m_holder = everythread::tid();
    }
    void before_unlock()
    {
        m_holder = 0;
    }

    friend class Condition;

    // Condition 专用的东西。
    class CondHolder 
    {
    private:
        Mutex &mr_mutex;        
    public:
        explicit CondHolder(Mutex &mutex)
            :mr_mutex(mutex)
        {
            mr_mutex.before_unlock();
        }

        ~CondHolder()
        {
            mr_mutex.after_lock();
        }
    };
};


class MutexLock 
{
private:
    Mutex &mr_mutex;
public:
    explicit MutexLock(Mutex &mutex)
        :mr_mutex(mutex)
    {
        mr_mutex.lock();
    }
    ~MutexLock()
    {
        mr_mutex.unlock();
    }
};
}



#endif //MOE_MUTEX_H