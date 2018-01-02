#ifndef MOE_ATOMIC_H
#define MOE_ATOMIC_H
#include <moe_net/base/Noncopyable.h>
#include <stdint.h>  
namespace moe 
{
namespace aux 
{
template <typename T>
class Atomic :aux::Noncopyable
{
private:
    T m_value;
public:
    Atomic()
        :m_value(0)
    {}

    T get()
    {
        return __sync_val_compare_and_swap(m_value);
    }
    T get_add(T v)
    {
        return __sync_fetch_and_add(&m_value,v);
    }
    T add_get(T v)
    {
        return get_add(v)+v;
    }
    T inc_get()
    {
        return add_get(1);
    }
    T dec_get()
    {
        return add_get(-1);
    }
    T get_set(T new_value)
    {
        return __sync_lock_test_and_set(&m_value,new_value);
    }

};
}

typedef aux::Atomic<int32_t> Atomic32;
typedef aux::Atomic<int64_t> Atomic64;

}



#endif //MOE_ATOMIC_H