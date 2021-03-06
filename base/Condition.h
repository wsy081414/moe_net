/*

Condition 类封装了 条件变量的简单操作
注意的是，传入 Condition 构造函数的是 Mutex 的引用。



在 wait 中：
线程在对 mutex 加锁的情况下检查资源，如果不符合条件，线程应该投入睡眠
此时的 pthread_cond_wait 做的事情是，解锁 mutex 以便变得县城能够使用该资源（一般是添加），然后阻塞线程
要求这个解锁+阻塞是一个原子操作，防止出现，解锁以后，在没阻塞之前别的线程获取锁添加资源以后，线程再阻塞，那么可能永远不会返回
而当别的线程，唤醒 wait 的线程时， pthread_cond_wait 会对 mutex 加锁，以便操作资源。
CondHolder 就是这个东西，在构造的时候解锁，在析构的时候加锁

*/

#ifndef MOE_CONDITION_H
#define MOE_CONDITION_H

#include <moe_net/base/Mutex.h>
#include <moe_net/base/Noncopyable.h>
#include <pthread.h>

namespace moe
{

class Condition : aux::Noncopyable
{
  private:
    Mutex &mr_mutex;
    pthread_cond_t m_cond;

  public:
    explicit Condition(Mutex &mutex)
        : mr_mutex(mutex)
    {
        if (pthread_cond_init(&m_cond, NULL))
        {
            FATAlLOG << "Condition init error";
        }
    }

    ~Condition()
    {
        if (pthread_cond_destroy(&m_cond))
        {
            FATAlLOG << "Condition destroy error";
        }
    }

    void wait()
    {

        Mutex::CondHolder set_holder(mr_mutex);
        if (pthread_cond_wait(&m_cond, mr_mutex.mutex_ptr()))
        {
            FATAlLOG << "Condition wait error";
        }
    }

    void notify()
    {
        if (pthread_cond_signal(&m_cond))
        {
            FATAlLOG << "Condition notify error";
        }
    }
    void notify_all()
    {
        if (pthread_cond_broadcast(&m_cond))
        {
            FATAlLOG << "Condition notify_all error";
        }
    }
};
}

#endif //MOE_CONDITION_H