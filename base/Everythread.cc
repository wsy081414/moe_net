#include <Everythread.h>

namespace moe
{
namespace everythread
{

__thread int t_tid=0;
__thread const char t_name[32]="moe thread";
__thread int t_name_len=12;

}

}



