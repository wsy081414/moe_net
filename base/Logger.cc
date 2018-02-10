

#include <moe_net/base/Logger.h>
#include <moe_net/base/Everythread.h>
#include <moe_net/base/String.h>
#include <stdio.h>

namespace moe 
{
namespace aux 
{
const String level_name[6]={
                            String("TRACE "),
                            String("DEBUG "),
                            String("INFO  "),
                            String("WARN  "),
                            String("ERROR "),
                            String("FATAL ")
                            };
}

void print_to_term(String str)
{
    size_t n = fwrite(str.c_str(), 1, str.size(), STDOUT_FIFENO);
}
}
using namespace moe;

Logger::OutFunc m_out_func=aux::print_to_term;

Logger::LogLevel Logger::s_level=Logger::TRACE;

Logger::Logger(const char *file_name,int line,const char *func_name,LogLevel level,bool to_abort)
    :m_time(),m_log(),mp_file(file_name),mp_func(func_name),m_line(line),m_level(level),mb_abort(to_abort)
{
    prepend_head();
}

void Logger::prepend_head()
{
    m_log<<m_time.format_string().c_str()<<" | ";
    pid_t tid = everythread::tid();
    m_log<<aux::level_name[level].c_str();
    m_log<<": "<<tid<<" "<<everythread::t_name<<" | ";
}
void Logger::append_tail()
{
    m_log<<" | "<<mp_file<<"-"<<m_line<<"-"<<mp_func;
}


Logger::~Logger()
{
    if(mb_abort)
    {
        append_tail();
    }

    m_log<<'\n';
    m_out_func(m_log.to_sting());
}
