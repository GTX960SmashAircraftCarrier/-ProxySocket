#include "Thread_Event_loop.h"
#include <assert.h>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

#include "Connect.h"
void ThreadEventLoop::WaitRun() try {
    if(loop_)
        throw "loop_ is already exit";
    loop_ = SP_EventLoop(new EventLoop());
    {
        std::unique_lock<std::mutex> lock(mutex_);
        start_ = true;
        cv2_.notify_all();
    }
    loop_->Loop();
} catch (std::exception& e){
    SPDLOG_CRITICAL("EventLoopThread::ThreadFunc exception: {}", e.what());
    abort();
}

void ThreadEventLoop::StartRun() {
    std::unique_lock<std::mutex> lock(mutex_);
    while(!start_) cv2_.wait(lock);
}

void ThreadEventLoop::AddChannel(SP_Channel channel) {
    loop_->AddToPoller(channel);
}

void ThreadEventLoop::AddConnect(SP_Connect connect) {
    loop_->AddToPoller(connect->getChannel());
}
