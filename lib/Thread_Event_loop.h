#ifndef THREAD_EVENT_LOOP_HPP
#define THREAD_EVENT_LOOP_HPP

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <iostream>
#include "Connect.h"
#include "Event_loop.h"
// 线程事件处理
class ThreadEventLoop {
public:
    ThreadEventLoop() : thread_(std::bind(&ThreadEventLoop::WaitRun, this)), mutex_(), cv2_() {
        std::cout<<"ThreadEventloop init success\n";
    }
    ~ThreadEventLoop() {
        if(start_)
            thread_.join();
    }

    void WaitRun();
    void StartRun();
    void AddChannel(SP_Channel channel);
    void AddConnect(SP_Connect connect);

    SP_EventLoop GetLoop() { return loop_;}
private:
    bool start_;
    SP_EventLoop loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv2_;
};

using SP_ThreadEventloop = std::shared_ptr<ThreadEventLoop>;

#endif