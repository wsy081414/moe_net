/*

Logger 类，则是封装了 LogStream ，将信息打印在终端上。

重载了 << 运算符，以和标准库一致。

Logger 类在构造的时候，传入出错的代码的行数，函数，文件等信息，直接保存在 m_log 中
同时还 << 了时间戳，目前的线程的信息
然后在  Logger 析构的时候，将日志打印出去。
其中打印的函数 m_out_func ,是一个静态变量，可以设置，因此可以修改该变量，来修改日志打印的方向

也就是说每次输出都会构造一个 Logger 实例，类内还有一个 LogStream 的实例


日志级别：

TRACE level 随便
DEBUG level 用于在开发是观察各类变量，再出问题的是后方便解决
INFO level 用于记录信息，打印一些状态信息，例如 EventLoop 创建了一个实例
WARN level 表明会出现潜在错误的情形。
ERROR level 指出虽然发生错误事件，但仍然不影响系统的继续运行。
FATAL level 指出每个严重的错误事件将会导致应用程序的退出。

*/
#ifndef MOE_LOGGER_H
#define MOE_LOGGER_H

#include <moe_net/base/LogStream.h>
#include <moe_net/base/Timestamp.h>
#include <functional>

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
    typedef std::function<void (String)> OutFunc;
  private:
    LogStream m_log;
    Timestamp m_time;
    const char *mp_file;
    const char *mp_func;
    int m_line;
    static LogLevel s_level;
    LogLevel m_level;
    
    bool mb_abort;
    static OutFunc m_out_func;
  public:
    Logger(const char *file_name, int line, const char *func_name, LogLevel level, bool to_abord = false);
    ~Logger();
    void prepend_head();
    void append_tail();
    LogStream &log() {return m_log;}
    static LogLevel level() {return s_level;}
    static void level(LogLevel v) {s_level=v;}
    static void out_func(OutFunc f) {m_out_func = f;}
};
#define TRACELOG if( Logger::level() <= Logger::TRACE) \
                  Logger(__FILE__, __LINE__, __func__, Logger::TRACE).log()
#define DEBUGLOG if( Logger::level() <= Logger::TRACE) \
                  Logger(__FILE__, __LINE__, __func__, Logger::DEBUG).log()
#define INFOLOG if( Logger::level() <= Logger::TRACE) \
                  Logger(__FILE__, __LINE__, __func__, Logger::INFO).log() 
#define WARNLOG  Logger(__FILE__, __LINE__, __func__, Logger::WARN,true).log()
                                   
#define ERRORLOG  Logger(__FILE__, __LINE__, __func__, Logger::ERROR,true).log()
#define FATAlLOG  Logger(__FILE__, __LINE__, __func__, Logger::FATAl,true).log()                          
}

#endif //MOE_LOGGER_H