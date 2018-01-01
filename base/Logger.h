#ifndef MOE_LOGGER_H
#define MOE_LOGGER_H

#include <moe_net/base/LogStream.h>
#include <moe_net/base/Timestamp.h>

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
    static LogLevel s_level;
    LogLevel m_level;
    
    bool mb_abort;
  public:


    Logger(const char *file_name, int line, const char *func_name, LogLevel level, bool to_abord = false);
    ~Logger();
    void prepend_time();
    void append();
    LogStream &log() {return m_log;}
    static LogLevel level() {return s_level;}
    static void level(LogLevel v) {s_level=v;}
    
};
#define TRACELOG if( Logger::level() <= Logger::TRACE) \
                  Logger(__FILE__, __LINE__, __func__, Logger::TRACE).log()
#define DEBUGLOG if( Logger::level() <= Logger::TRACE) \
                  Logger(__FILE__, __LINE__, __func__, Logger::DEBUG).log()
#define INFOLOG if( Logger::level() <= Logger::TRACE) \
                  Logger(__FILE__, __LINE__, __func__, Logger::INFO).log()                  
#define ERRORLOG  Logger(__FILE__, __LINE__, __func__, Logger::ERROR).log()
#define FATAlLOG  Logger(__FILE__, __LINE__, __func__, Logger::FATAl).log()                          
}

#endif //MOE_LOGGER_H