#ifndef MOE_THREAD_POOL_H
#define MOE_THREAD_POOL_H

#include <moe_net/base/Thread.h>
#include <moe_net/base/Noncopyable.h>
#include <moe_net/base/String.h>
#include <moe_net/base/Mutex.h>
#include <moe_net/base/Condition.h>

#include <functional>
#include <vector>
#include <queue>

namespace moe 
{


class ThreadPool : aux::Noncopyable
{
public:
    typedef std::function<void ()> Task;
    explicit ThreadPool(const String & name = String("thread poll"));
    ~ThreadPool();

    void max_queue(int max_size) {m_max_queue = max_size;}
    void set_init_callback(Task &t) {m_init_cb = t;}

    void start(int thread_count);
    void stop();

    void add_task(const Task &);

    size_t queue_size();
private:
    bool is_full() const;
    void seed_into_thread();
    Task take();

    mutable Mutex m_mutex;
    Condition m_can_take;
    Condition m_can_add;

    String m_name;

    Task m_init_cb;
    
    std::vector<moe::Thread*> mc_threads;
    std::deque<Task> mc_tasks;

    size_t m_max_queue;
    
    bool mb_started;

};

}



#endif //MOE_THREAD_POOL_H