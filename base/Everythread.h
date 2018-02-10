/*

改文件中定义了，每个线程都应该有的。
__thread是GCC内置的线程局部存储设施，存取效率可以和全局变量相比。

__thread变量每一个线程有一份独立实体，各个线程的值互不干扰
__thread使用规则：只能修饰POD类型(类似整型指针的标量，不带自定义的构造、拷贝、赋值、析构的类型）

同时是用 syscall 获取线程的 tid
*/

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