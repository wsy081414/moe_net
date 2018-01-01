#ifndef MOE_EVERYTHREAD_H
#define MOE_EVERYTHREAD_H

#include <sys/syscall.h> //SYS_xxx
#include <unistd.h> //syscall()


namespace moe
{
namespace everythread
{

extern __thread int t_tid;
extern __thread char t_name[32];
extern __thread int t_name_len;

inline pid_t tid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

}
}






#endif //MOE_EVERYTHREAD_H