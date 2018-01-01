#ifndef MOE_TIMESTAMP_H
#define MOE_TIMESTAMP_H

#include <moe_net/base/String.h>

namespace moe 
{

class Timestamp
{
private:
    int64_t m_micro;
public:
    Timestamp()
        :m_micro(now())
    {}
    Timestamp(int64_t micro)
        :m_micro(micro)
    {}
    
    String to_sting();
    String format_string(bool show_nsec=true);

    static int64_t kk;
    void from_sec(time_t secs)
    {
        m_micro = secs*kk;
    }
private:
    int64_t now();
};



}


#endif //MOE_TIMESTAMP_H