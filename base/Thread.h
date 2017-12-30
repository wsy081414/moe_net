#ifndef MOE_THREAD_H
#define MOE_THREAD_H

#include <pthread.h>
#include <Noncopyable.h>
#include <functional>
#include <String.h>
#include <CountDown.h>

namespace moe 
{

class Thread : aux::Noncopyable
{
public:
    typedef std::function<void ()> ThreadFunc;

private:

    ThreadFunc m_func;
    String m_name;
    bool m_started;
    bool m_joined;
    pid_t m_tid;
    pthread_t m_pthread;
    CountDown m_sem;
    
public:
    
    explicit Thread(const ThreadFunc& , const String &name  = String());
    ~Thread();

    void start();
    void join();
    
    pid_t tid() {return m_tid;}
    const String& name() {return m_name;}



};
}

#endif //MOE_THREAD
