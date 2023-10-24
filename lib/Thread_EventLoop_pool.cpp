#include "Thread_EventLoop_pool.h"
#include "Thread_Event_loop.h"

SP_ThreadEventloop ThreadEventLoopPoll::PickRandThread() {
    SP_ThreadEventloop t;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        t = threads_[next_thread_id_];
        next_thread_id_ = (next_thread_id_ + 1) % thread_num_;
    }
    return t;
}

void ThreadEventLoopPoll::Run() {
    for (int i = 0; i < thread_num_; i++) {
        SP_ThreadEventloop t(new ThreadEventLoop());
        //阻塞 等待唤醒
        t->StartRun();
        threads_.emplace_back(t);
    }
}