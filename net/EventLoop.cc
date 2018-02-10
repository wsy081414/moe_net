#include <moe_net/net/EventLoop.h>
#include <moe_net/net/Channel.h>
#include <moe_net/net/EpollPoller.h>
#include <moe_net/net/SockOps.h>


#include <sys/eventfd.h>

namespace moe
{
namespace everythread
{

using namespace moe::net;
__thread EventLoop *t_loop = 0;
}
namespace aux
{

// const int s_poll_time = 10000;
// eventfd
// eventfd()创建了一个"eventfd object"，能在用户态用做事件wait/notify机制，
// 通过内核取唤醒用户态的事件。这个对象保存了一个内核维护的uint64_t类型的整型counter。
// 这个counter初始值被参数initval指定，一般初值设置为0。
// 它返回了一个引用eventfd object的描述符。这个描述符可以支持以下操作：
// read：如果计数值counter的值不为0，读取成功，获得到该值。如果counter的值为0，非阻塞模式，会直接返回失败，并把errno的值指纹EINVAL。
//         如果为阻塞模式，一直会阻塞到counter为非0位置。
// write：会增加8字节的整数在计数器counter上，如果counter的值达到0xfffffffffffffffe时，就会阻塞。直到counter的值被read。
//         阻塞和非阻塞情况同上面read一样。
// close：

// EFD_SEMAPHORE参数。如果一个eventfd被指定了该参数，当eventfd的计数器不为0时，
// 对eventfd的read操作会读到一个8byte长度的整数1，然后计数器的值减1；
// 如果没有指定该参数，当eventfd的计数器不为0时，对eventfd的read操作会将计数器的值读出（8byte的整数），并将计数器置0.
int event_fd()
{
    // 第一个参数是初始化参数
    int ret = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (ret < 0)
    {
        FATAlLOG<<"EventLoop eventfd create error";
    }
    return ret;
}
}
}

using namespace moe;
using namespace moe::net;
using namespace moe::everythread;


EventLoop::EventLoop()
    : mb_looping(false), mb_quited(false), mb_handling(false), m_tid(everythread::tid()),
      mp_poll(new EpollPoller(this)),m_timer_queue(this),
      m_wakeup_fd(aux::event_fd()), mp_wakeup_channel(new Channel(this, m_wakeup_fd)),
      m_mutex(),s_poll_time(1000*8)
{
    TRACELOG << "EventLoop init: " << this << " " << m_tid;
    
    if (t_loop != nullptr)
    {
        FATAlLOG << "EventLoop thread: " << m_tid << " has EventLoop";
    }
    else
    {
        t_loop = this;
    }

    mp_wakeup_channel->set_read_cb(
        std::bind(&EventLoop::wakeup_channel_handle_read, this));
    mp_wakeup_channel->enable_read();
}

EventLoop::~EventLoop()
{
    TRACELOG << "EventLoop destory " << this << " " << m_tid;
    mp_wakeup_channel->disable_all();
    mp_wakeup_channel->remove();
    ::close(m_wakeup_fd);
    t_loop = nullptr;
}

void EventLoop::loop()
{
    assert(!mb_looping);
    assert(is_in_loop_thread());
    mb_looping = true;
    mb_quited = false;

    TRACELOG << "EventLoop loop start: " << m_tid;

    while (!mb_quited)
    {
        mc_active_channels.clear();
        m_poll_return = mp_poll->poll(s_poll_time, &mc_active_channels);
        mb_handling = true;

        for (ChannelVector::iterator it = mc_active_channels.begin();
             it != mc_active_channels.end(); ++it)
        {
            (*it)->handle_event(m_poll_return);
        }
        mb_handling = false;
        handle_tasks();
    }
    mb_looping = false;
}

void EventLoop::quit()
{
    mb_quited = true;
    if (is_in_loop_thread())
    {
        wakeup();
    }
}



void EventLoop::update(Channel *channel)
{
    assert(channel->loop() == this);
    assert(is_in_loop_thread());
    mp_poll->update(channel);
}

void EventLoop::remove(Channel *channel)
{
    assert(channel->loop() == this);
    assert(is_in_loop_thread());

    // 这里有判断

    mp_poll->remove(channel);
}

bool EventLoop::has_channel(Channel *channel)
{
    assert(channel->loop() == this);
    assert(is_in_loop_thread());

    return mp_poll->has_channel(channel);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = sockops::write(m_wakeup_fd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        ERRORLOG<<"EventLoop wakeup write,size error: "<<n;
    }
}

void EventLoop::wakeup_channel_handle_read()
{
    uint64_t one = 1;
    ssize_t n = sockops::read(m_wakeup_fd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        ERRORLOG<<"EventLoop wakeup read,size error: "<<n;
    }
}

bool EventLoop::is_in_loop_thread(bool need)
{
    // need 表示是否需要终止进程，在添加task的时候只是需要判断是否需要添加在 queue 中，
    // 因此不需要终止
    if(need && m_tid != everythread::tid())
    {
        FATAlLOG<<"create in thread: "<<m_tid <<" running thread: "<<everythread::tid();
    }
    return m_tid == everythread::tid();
}

// 添加计时器
int64_t EventLoop::add_timer(const TimerCallBack& cb,int64_t when,bool repeat)
{
 
    return m_timer_queue.add_timer(cb,when,repeat);
}   
void EventLoop::cancel_timer(int64_t id)
{
    m_timer_queue.cancel_timer(id);
}


// 添加任务
void EventLoop::add_task(const Func & task)
{
    if(is_in_loop_thread(false))
    {
        task();
    }else{
        add_in_queue(task);
    }
}


void EventLoop::add_in_queue(const Func &task)
{
    {
        MutexLock lock(m_mutex);
        mc_tasks.push_back(task);
    }

    if(!is_in_loop_thread(false) || m_handle_tasks)
    {
        wakeup();
    }
}

void EventLoop::handle_tasks()
{
    m_handle_tasks=true;
    std::vector<Func> tmp_vector;
    {
        MutexLock lock(m_mutex);
        tmp_vector.swap(mc_tasks);
    }

    for(task:tmp_vector)
    {
        task();
    }
    m_handle_tasks=false;
}