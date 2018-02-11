/*

Mutex 类对 互斥量的初始化、加锁、解锁、销毁进行了简单的封装。
同时 Mutex 不允许被复制。

MutexLock 则是使用 RAII 的思想，在初始化的时候加锁，在析构的时候释放锁
MutexLock 保存的是锁的引用


Mutex 内有一变量 m_holder 保存了改锁目前被那个线程所持有。在加锁解锁的时候回记录，该线程。
然而，在条件变量中，在条件变量 wait 的时候回原子的解锁，在wait 返回的时候会加锁，
因此设计了一个 CondHolder 用来在 条件变量 wait 函数一开始就改变 m_holder =0 
表示在 wait 的线程，释放了该锁。 而在wait函数结束的时候，CondHolder 析构，设置 m_holder = tid()
表示该线程有进行了加锁。

*/

#ifndef MOE_MUTEX_H
#define MOE_MUTEX_H

#include <moe_net/base/Noncopyable.h>
#include <pthread.h>
#include <moe_net/base/Everythread.h>
#include <moe_net/base/Logger.h>

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
        : m_holder(0)
    {
        if (pthread_mutex_init(&m_mutex, NULL))
        {
            FATAlLOG << "Mutex init error";
        }
    }

    ~Mutex()
    {
        assert(m_holder == 0);
        if (pthread_mutex_destroy(&m_mutex))
        {
            FATAlLOG << "Mutex destroy error";
        }
    }

    void lock()
    {
        if (pthread_mutex_lock(&m_mutex))
        {
            FATAlLOG << "Mutex lock error";
        }
        after_lock();
    }
    void unlock()
    {
        before_unlock();
        if (pthread_mutex_unlock(&m_mutex))
        {
            FATAlLOG << "Mutex unlock error";
        }
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
            : mr_mutex(mutex)
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
        : mr_mutex(mutex)
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