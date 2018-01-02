#include <moe_net/net/Timer.h>

using namespace moe;
using namespace moe::net;

Atomic64 Timer::timer_index;

    void Timer::restart()
    {
        if(repeat())
        {
            m_expiration.add(m_interval);
        }else{
            m_expiration.set(0);
        }
    }
