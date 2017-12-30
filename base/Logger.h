#ifndef MOE_LOGGER_H
#define MOE_LOGGER_H

#include <LogStream.h>
#include <Timestamp.h>

namespace moe
{

class Logger
{
  public:
    enum LogLevel
    {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAl = 5
    };

  private:
    LogStream m_log;
    Timestamp m_time;
    const char *mp_file;
    const char *mp_func;
    int m_line;
    LogLevel m_level;
    bool mb_abort;

  public:


    Logger(const char *file_name, int line, const char *func_name, LogLevel level, bool to_abord = false);
    ~Logger();
    void prepend_time();
    void append();
    LogStream &log() {return m_log;}
};
#define TRACELOG Logger(__FILE__, __LINE__, __func__, Logger::TRACE).log()
}

#endif //MOE_LOGGER_H