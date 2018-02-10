#ifndef MOE_NONCOPYABLE
#define MOE_NONCOPYABLE

namespace moe 
{
namespace aux 
{

class Noncopyable 
{
    // protected 只有子类可以访问，避免了 noncopyable 的实例化
protected:
    Noncopyable() {}
    ~Noncopyable() {}

    // private 在拷贝派生类的时候，因为函数只有声明没有定义，链接期会报错
private:
    Noncopyable(const Noncopyable &);
    const Noncopyable &operator=(const Noncopyable);

};
}
}


#endif //MOE_NONCOPYABLE