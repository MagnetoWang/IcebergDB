#include <thread>
#include <iostream>
#include <pthread.h>
#include <atomic>

#include "gtest/gtest.h"
class ThreadTest {};

void PrintHello() {
    std::cout << "hello concurrent" << std::endl;
}

//create thread
TEST(ThreadTest, PrintHello) {
    std::thread t(PrintHello);
    t.join();
}

TEST(ThreadTest, CreatePthread) {
    pthread_t ntid;
    // int err = pthread_create(&ntid, NULL, (void *)PrintHello, NULL); 
}


void do_something(int& i)
{
    ++i;
}

struct func
{
    int& i;
    func(int& i_):i(i_){}
    void operator()()
    {
        for(unsigned j=0;j<1000000;++j)
        {
            do_something(i);
        }
    }
};


void oops()
{
    int some_local_state=0;
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    my_thread.detach();
}

// 会出现线程竞争
// thread race conditions
TEST(ThreadTest, ThreadDetach) {
    //oops();
}

class thread_guard
{
    std::thread& t;
public:
    explicit thread_guard(std::thread& t_):
        t(t_)
    {}
    ~thread_guard()
    {
        if(t.joinable())
        {
            t.join();
        }
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

void TestThreadGuard() {
    int some_local_state;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
}

TEST(ThreadTest, ThreadJoin) {
    TestThreadGuard();
}

std::atomic<bool> x,y;
std::atomic<int> z;

void write_x()
{
    x.store(true,std::memory_order_seq_cst);
}

void write_y()
{
    y.store(true,std::memory_order_seq_cst);
}

void read_x_then_y()
{
    while(!x.load(std::memory_order_seq_cst));
    if(y.load(std::memory_order_seq_cst))
        ++z;
}

void read_y_then_x()
{
    while(!y.load(std::memory_order_seq_cst));
    if(x.load(std::memory_order_seq_cst))
        ++z;
}

// 内存模型强制顺序一致性
// verify memory_order_seq_cst feature
TEST(TheadTest, TestSeqConsistent) {
    x=false;
    y=false;
    z=0;
    std::thread a(write_x);
    std::thread b(write_y);
    std::thread c(read_x_then_y);
    std::thread d(read_y_then_x);
    a.join();
    b.join();
    c.join();
    d.join();
    std::cout << z.load() << std::endl;
    assert(z.load()!=0); // 可能等于1或者2
    // assert(z.load()==2);
}

void relax_write_x_then_y()
{
    x.store(true,std::memory_order_relaxed);
    y.store(true,std::memory_order_relaxed);
}

void relax_read_y_then_x()
{
    while(!y.load(std::memory_order_relaxed));
    if(x.load(std::memory_order_relaxed))
        ++z;
}

// 内存模型松散一致性
// verify memory_order_relaxed feature 
TEST(TheadTest, TestRelax) {
    x=false;
    y=false;
    z=0;
    std::thread a(relax_write_x_then_y);
    std::thread b(relax_read_y_then_x);
    a.join();
    b.join();
    assert(z.load() != 0);
    if (z.load() == 1) {
        std::cout << z.load() << std::endl;
    } else {
        std::cout << "wrong" << std::endl;
    }
}

void release_write_x()
{
    x.store(true,std::memory_order_release);
}

void release_write_y()
{
    y.store(true,std::memory_order_release);
}

void acquire_read_x_then_y()
{
    while(!x.load(std::memory_order_acquire));
    if(y.load(std::memory_order_acquire))
        ++z;
}

void acquire_read_y_then_x()
{
    while(!y.load(std::memory_order_acquire));
    if(x.load(std::memory_order_acquire))
        ++z;
}

TEST(TheadTest, TestReleaseAcquire) {
    x=false;
    y=false;
    z=0;
    std::thread a(release_write_x);
    std::thread b(release_write_y);
    std::thread c(acquire_read_x_then_y);
    std::thread d(acquire_read_y_then_x);
    a.join();
    b.join();
    c.join();
    d.join();
    assert(z.load()!=0);
    std::cout << z.load() << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
