#include <moe_net/base/Timestamp.h>
#include <sys/time.h>
#include <stdio.h>
using namespace moe;
int64_t Timestamp::kk = 1000 * 1000;

int64_t Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * kk + tv.tv_usec;
}

String Timestamp::to_sting()
{
    char buf[32];
    int secs = m_micro / kk;
    int nsecs = m_micro % kk;
    snprintf(buf, sizeof(buf) - 1, "%d . %06d", secs, nsecs);
    return buf;
}

String Timestamp::format_string(bool show_nsec)
{
    char buf[32];
    time_t secs = static_cast<time_t>(m_micro / kk);
    struct tm tm_time;
    gmtime_r(&secs, &tm_time);

    if (show_nsec)
    {
        int nsecs = static_cast<int>(m_micro % kk);

        snprintf(buf, sizeof(buf) - 1, "%04d-%02d-%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour+8, tm_time.tm_min, tm_time.tm_sec,
                 nsecs);
    }
    else
    {
        snprintf(buf, sizeof(buf) - 1, "%04d-%02d-%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour+8, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}