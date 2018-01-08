#include <moe_net/net/TimerQueue.h>
#include <moe_net/net/EventLoop.h>

#include <sys/timerfd.h>
namespace moe
{
namespace aux
{
    int create_time_fd()
    {
        // timerfd_create（）函数创建一个定时器对象，同时返回一个与之关联的文件描述符。
        // clockid：clockid标识指定的时钟计数器，可选值（CLOCK_REALTIME、CLOCK_MONOTONIC。。。）
        // CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时,
        // 中间时刻如果系统时间被用户改成其他,则对应的时间相应改变
        // CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
        // flags：参数flags（TFD_NONBLOCK(非阻塞模式)/TFD_CLOEXEC（表示当程序执行exec函数时本fd将被系统自动关闭,表示不传递）
        int ret = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
        if(ret<0)
        {
            // log
        }
        return ret;
    }

    struct timespec cal_new_expiration(Timestamp& new_expiration)
    {
        Timestamp now;
        int64_t remain_micro = new_expiration.micro()-now.micro();
        if(remain_micro <100 )
        {
         remain_micro=100;   
        }

        struct timespec ts;
        ts.tv_sec=static_cast<time_t>(remain_micro /Timestamp::kk);
        ts.tv_nsec=static_cast<long>((remain_micro %Timestamp::kk)*1000);
        return ts;
        
    }

    void read_time_fd(int fd,Timestamp time)
    {
        uint64_t one;
        ssize_t ret = ::read(fd,&one,sizeof(one));
        if(ret != sizeof(one))
        {
            // log
        }
        TRACELOG<<"timefd wake at:"<<time.format_string();
    }

    void reset_timefd_expiration(int fd,Timestamp expiration)
    {
        struct itimerspec new_expiration;
        struct itimerspec old_expiration;
        bzero(&new_expiration,sizeof(new_expiration));
        bzero(&old_expiration,sizeof(old_expiration));
        new_expiration.it_value=cal_new_expiration(expiration);

        int ret = ::timerfd_settime(fd,0,&new_expiration,&old_expiration);
        if(ret <0)
        {
            // log
        }
        TRACELOG<<"reset timerfd ,will wakeup :"<<expiration.format_string();
    }

}
}


using namespace moe;
using namespace moe::net;


TimerQueue::TimerQueue(EventLoop *loop)
    :mp_loop(loop),m_timer_fd(aux::create_time_fd()),m_timer_channel(mp_loop,m_timer_fd),
    mc_timers()
{
    m_timer_channel.set_read_cb(
                                std::bind(&TimerQueue::handle_expiration,this)
                                );
    m_timer_channel.enable_read();
}

TimerQueue::~TimerQueue()
{
    m_timer_channel.disable_all();
    m_timer_channel.remove();
    // 
}

// TimerQueue::handle_expiration(const Timer& timer)
// {
//     mp_loop->add_queue(
//                         std::bind()
//                         );
// }


int64_t TimerQueue::add_timer(const TimerCallBack& cb,int64_t when,bool repeat)
{
    Timestamp now;
    now.add(when);
    Timer* timer = new Timer(cb,now,repeat?when:0);
    return readd(timer);
}

void TimerQueue::cancel_timer(int64_t index)
{
    assert(mc_timers_for_del.size() == mc_timers.size());
    TimerMapRunDel::iterator it=mc_timers_for_del.find(index);

    if(it != mc_timers_for_del.end())
    {   
        // 如果还在map中，表示还没有超时，那么直接删除
        {
            delete it->second->second;
            mc_timers.erase(it->second);
            mc_timers_for_del.erase(it);
        }
   
    }else{
        if(mb_handling)
        {
            // 如果没有找到，因为在执行超时函数的时候，会讲timer从map中移除，因此需要讲 定期的timer，执行完超时函数，不要在添加进队列
            mc_timers_will_del.insert(index);
        }
    }
}

void TimerQueue::handle_expiration()
{
    mb_handling=true;
    Timestamp now;
    aux::read_time_fd(m_timer_fd,now);

    std::vector<Timer*> expired_timers = get_expired(now);

    for(timer : expired_timers)
    {
        timer->run();
    }
    mb_handling=false;

    handle_interval(expired_timers,now);
}


std::vector<Timer*> TimerQueue::get_expired(Timestamp now)
{
    // TRACELOG<<"size : "<<mc_timers.size()<<" "<<mc_timers_for_del.size();
    assert(mc_timers_for_del.size() == mc_timers.size());
    std::vector<Timer*> ret;
    TimerMapRun::iterator expired_last=mc_timers.lower_bound(now);
    for(TimerMapRun::iterator it=mc_timers.begin();it != expired_last;it++)
    {
        ret.push_back(it->second);
    }
    mc_timers.erase(mc_timers.begin(), expired_last);

    for(timer : ret)
    {
        size_t n = mc_timers_for_del.erase(timer->index());
        if(n <0)
        {
            // log
        }
    }
    assert(mc_timers_for_del.size() == mc_timers.size());
    return ret;
}   


void TimerQueue::handle_interval(const std::vector<Timer*>& expired_timers,Timestamp now)
{
    bool add=false;
    for(timer : expired_timers)
    {
        if(timer->repeat() == true && 
            (mc_timers_will_del.find(timer->index()) == mc_timers_will_del.end())
            )
        {
            timer->restart();
            readd(timer);
            add =true;
        }else{
            TRACELOG<<"Timer delete : "<<timer->index();
            delete timer;
        }
    }
    if(add == false)
    {
        if(!mc_timers.empty())
        {
            aux::reset_timefd_expiration(m_timer_fd,mc_timers.begin()->first);
        }else{
            TRACELOG<<"no timer waiting run";
        }
    }
}

int64_t TimerQueue::readd(Timer *timer)
{
    bool new_run = false;
    bool first_run =false;
    
    TimerMapRun::iterator it = mc_timers.begin();

    if(it == mc_timers.end())
    {
        new_run = true;
    }else{ 

        // 添加符号运算
        if(timer->expiration() < it->first)
        {
            new_run = true;
        }else{
            first_run = true;
        }
    }

    std::pair<TimerMapRun::iterator, bool> tmp = mc_timers.insert( std::pair<Timestamp,Timer*>(timer->expiration(),timer));
    mc_timers_for_del.insert(std::pair<int64_t,TimerMapRun::iterator>(timer->index(),tmp.first));
    
    // TRACELOG<<"add timer:"<<timer->index()<<" expiration :"<< timer->expiration().format_string()
    //         <<"need reset timefd: "<<new_run<<" size: "<<mc_timers.size()<<" "<<mc_timers_for_del.size();
    
    if(new_run == true)
    {
        aux::reset_timefd_expiration(m_timer_fd,timer->expiration());
    }else if(first_run ==true)
    {
        aux::reset_timefd_expiration(m_timer_fd,it->first);
        
    }

    return timer->index();
}
