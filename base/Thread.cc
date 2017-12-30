#include <Thread.h>

#include <sys/prctl.h>


using namespace moe;

namespace moe
{


namespace aux 
{

struct ThreadData 
{
    typedef moe::Thread::ThreadFunc ThreadFunc;
    ThreadFunc m_func;

    String m_name;
    pid_t *m_tid;
    CountDown *mp_sem;

    ThreadData(const ThreadFunc &func,const String &name, pid_t *pid,CountDown *sem)
        :m_func(func),m_name(name),mp_sem(sem),m_tid(pid)
    {}

    void run_in_thread()
    {
        *m_tid = moe::everythread::tid();
        mp_sem->count_down();
        // moe::everythread::t_name = m_name.empty()?"moe thread":m_name.c_str();
        ::prctl(PR_SET_NAME,m_name);
        
        
        m_func();

    }

};

void *send_into_pthread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    data->run_in_thread();
    delete data;
    return NULL;
}

}
}


using namespace moe;

Thread::Thread(const ThreadFunc& func, const String &name)
    :m_started(false),m_joined(false),
    m_tid(0),m_pthread(0),
    m_func(func),m_name(name),
    m_sem(1)
{}

Thread::~Thread()
{
    if(m_started && !m_joined)
    {
        pthread_detach(m_pthread); 
    }
}

void Thread::start()
{
    assert(m_started==false);
    m_started = true;

    aux::ThreadData *data=new aux::ThreadData(m_func,m_name,&m_tid,&m_sem);
    if(pthread_create(&m_pthread,NULL,&aux::send_into_pthread,data) != 0)
    {
        m_started = false;
        delete data;
        // log
    }else{
        m_sem.wait();
        assert(m_tid>0);
    }
}


void Thread::join()
{
    if(m_started && !m_joined)
    {
        m_joined = true;
        pthread_join(m_pthread,NULL);
    }
}
