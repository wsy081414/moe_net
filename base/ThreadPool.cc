#include <moe_net/base/ThreadPool.h>
#include <moe_net/base/Logger.h>

// #include <shared_ptr>
#include <algorithm>
using namespace moe;

ThreadPool::ThreadPool(const String &name)
    : m_mutex(), m_can_take(m_mutex), m_can_add(m_mutex),
      m_name(name), m_max_queue(0), mb_started(false), m_init_cb(nullptr)
{
}

ThreadPool::~ThreadPool()
{
    if (mb_started)
    {
        stop();
    }
}

void ThreadPool::start(int thread_counts)
{
    assert(!mb_started);
    m_max_queue = thread_counts*100;
    mb_started = true;

    mc_threads.reserve(thread_counts);

    for (int i = 0; i < thread_counts; ++i)
    {
        char id[32];
        snprintf(id, sizeof(id), "%s : %d", m_name.c_str(), i + 1);

        mc_threads.push_back(new Thread(
            std::bind(&ThreadPool::seed_into_thread, this),
            id));
        mc_threads[i]->start();
    }
    INFOLOG<<"ThreadPool start";
    if (m_init_cb != nullptr)
    {
        m_init_cb();
    }
}


void ThreadPool::stop()
{
    {
        MutexLock lock(m_mutex);
        mb_started = false;
        m_can_take.notify_all();
    }
    for(auto it=mc_threads.begin();it!=mc_threads.end();++it)
    {
        // 阻塞等待
        (*it)->join();
    }
}

size_t ThreadPool::queue_size()
{
    MutexLock lock(m_mutex);
    return mc_tasks.size();
}

void ThreadPool::add_task(const Task &t)
{
    
    if(mc_threads.empty())
    {
        t();
    }else{
        MutexLock lock(m_mutex);
        while(is_full())
        {
            m_can_add.wait();
        }
        assert(!is_full());
        mc_tasks.push_back(t);
        m_can_take.notify();
    }
}

ThreadPool::Task ThreadPool::take()
{
    Task t;
    MutexLock lock(m_mutex);
    
    while(mc_tasks.empty() && mb_started)
    {
        m_can_take.wait();
    }
    
    if(!mc_tasks.empty())
    {
        t = mc_tasks.front();
        mc_tasks.pop_front();

        if(mc_tasks.size() < m_max_queue)
        {
            m_can_add.notify();
        }
    }
    return t;
}

void ThreadPool::seed_into_thread()
{
    if(m_init_cb != nullptr)
    {
        m_init_cb();
    }

    while(mb_started)
    {
        Task t(take());
        if(t)
        {
            t();
        }
    }
}

bool ThreadPool::is_full() const 
{
    return mc_tasks.size() >= m_max_queue;
}
