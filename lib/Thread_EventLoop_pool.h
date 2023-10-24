#ifndef THREAD_EVENT_LOOP_POOL_HPP
#define THREAD_EVENT_LOOP_POOL_HPP

#include <memory>
#include <mutex>
#include <vector>
#include <iostream>
#include "Event_loop.h"
#include "Thread_Event_loop.h"
//线程池
class ThreadEventLoopPoll {
public:
    ThreadEventLoopPoll(int thread_num) : mutex_(), thread_num_(thread_num > 0 ? thread_num : 1), next_thread_id_(0) {
        std::cout<<"Threadeventpoll init success\n";
    }
    ~ThreadEventLoopPoll(){
        std::cout<<"ThreadEventLoopPoll is close\n";
    }
    SP_ThreadEventloop PickRandThread();
    
    void Run();
private:
    std::mutex mutex_;
    int thread_num_;
    int next_thread_id_;
    std::vector<SP_ThreadEventloop> threads_;
};

using SP_ThreadEventLoopPoll = std::shared_ptr<ThreadEventLoopPoll>;
#endif