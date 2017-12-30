#include <Logger.h>
#include <Everythread.h>
#include <String.h>
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
}
using namespace moe;



Logger::Logger(const char *file_name,int line,const char *func_name,LogLevel level,bool to_abort)
    :m_time(),m_log(),mp_file(file_name),mp_func(func_name),m_line(line),m_level(level),mb_abort(to_abort)
{
    prepend_time();
    pid_t tid = everythread::tid();
    m_log<<" "<<aux::level_name[level].c_str()<<" ";
    m_log<<"in thread : "<<tid<<" "<<everythread::t_name<<" ";
    
    //这里添加出错信息 

}

void Logger::prepend_time()
{
    m_log<<m_time.format_string().c_str()<<" ";
}
void Logger::append()
{
    m_log<<" | "<<mp_file<<"-"<<m_line<<"-"<<mp_func<<'\n';
    m_log.print();
}


Logger::~Logger()
{
    append();
}
