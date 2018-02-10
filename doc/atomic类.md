# Atomic类

> 原子操作

## 1 原子操作

所谓的原子操作，就是该操作绝不会在执行完毕前被任何其他任务或事件打断，

也就是说，它的最小的执行单位，不能有比它更小的执行单元

原子操作需要硬件的支持，因此是架构相关的

## 2 gcc支持

gcc从4.1.2提供了__sync_*系列的built-in函数

### 用于数值运算的

```c
// 第一组返回更新前的值
type __sync_fetch_and_add (type *ptr, type value, ...);
type __sync_fetch_and_sub (type *ptr, type value, ...);
type __sync_fetch_and_or (type *ptr, type value, ...);
type __sync_fetch_and_and (type *ptr, type value, ...);
type __sync_fetch_and_xor (type *ptr, type value, ...);
type __sync_fetch_and_nand (type *ptr, type value, ...);

// 第二组返回更新后的值
type __sync_add_and_fetch (type *ptr, type value, ...);
type __sync_sub_and_fetch (type *ptr, type value, ...);
type __sync_or_and_fetch (type *ptr, type value, ...);
type __sync_and_and_fetch (type *ptr, type value, ...);
type __sync_xor_and_fetch (type *ptr, type value, ...);
type __sync_nand_and_fetch (type *ptr, type value, ...);
```

type可以是1,2,4或8字节长度的int类型

```c
int8_t / uint8_t
int16_t / uint16_t
int32_t / uint32_t
int64_t / uint64_t
```

后面的可扩展参数`...`用来指出哪些变量需要memory barrier,因为目前gcc实现的是full barrier（类似于linux kernel 中的mb(),表示这个操作之前的所有内存操作不会被重排序到这个操作之后）,所以可以略掉这个参数。

### 用于比较交换的

```c
bool __sync_bool_compare_and_swap (type *ptr, type oldval ,type newval, ...);
type __sync_val_compare_and_swap (type *ptr, type oldval, type newval, ...);
```

这两个函数提供原子的比较和交换，如果`*ptr == oldval`,就将`newval`写入`*ptr`,
第一个函数在相等并写入的情况下返回true.
第二个函数在返回操作之前的值。



```c
// 发出一个full barrier.
__sync_synchronize (...)
```

```c
// 将*ptr设为value并返回*ptr操作之前的值。
type __sync_lock_test_and_set (type *ptr, type value, ...)

// 将*ptr置0
void __sync_lock_release (type *ptr, ...)
```



## 3 Memory barrier

> 这是对cpu的

有单独的一篇文章来讲解。

## 4 c++11支持

使用模板:

```c++
#include <atomic>

template <typename Type>
std::atomic<Type>
```

并重载了各种运算符，可以简单的使用。





## 5 本库

只是简单的封装了`gcc`提供的接口。

