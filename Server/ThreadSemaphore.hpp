#ifndef _THREAD_SEMAPHORE_HPP
#define _THREAD_SEMAPHORE_HPP

#include<thread>
#include<condition_variable>
#include<chrono>
class ThreadSemaphore{
public:
    ThreadSemaphore(): _wait(0), _wakeup(0){}
    void wait(){
        std::unique_lock<std::mutex> lk(_mutex);
        if(--_wait < 0){
            _cv.wait(lk, [this]() -> bool {return _wakeup > 0;});
            --_wakeup;
        }
    }

    void wakeup(){
        std::unique_lock<std::mutex> lk(_mutex);
        if(++_wait <= 0){
            ++_wakeup;
            _cv.notify_one();
        }
    }
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    int _wait;
    int _wakeup;
};

#endif