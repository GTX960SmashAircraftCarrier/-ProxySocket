#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <sys/epoll.h>
#include <memory>
#include <vector>
#include <assert.h>
#include "Channel.h"
#include "Event.h"
#include <iostream>
// EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
// EPOLLOUT：表示对应的文件描述符可以写；
// EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
// EPOLLERR：表示对应的文件描述符发生错误；
// EPOLLHUP：表示对应的文件描述符被挂断；
// EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
// EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里

//　最多事件
const int MAXFD = 10000;

const int EVENTSNUM = 4096;
// 等待事件
const int EPOLLWAIT_TIME = 1000;

class Epoll : public Event {
public:
    Epoll(): fd_(epoll_create1(EPOLL_CLOEXEC)), epoll_events_(EVENTSNUM){
        assert(fd_ > 0);
        std::cout<<"Epoll init success\n";
    }
    virtual ~Epoll() {}
    int get_fd(){return fd_;}

    virtual void PollAdd(SP_Channel channel) override final;
    virtual void PollMod(SP_Channel channel) override final;
    virtual void PollDel(SP_Channel channel) override final;
    
    // 获取事件
    std::vector<SP_Channel> WaitChannels() override final;
private:
    std::vector<SP_Channel> getReadyChannels(int);
private:
    int fd_;
    SP_Channel fd_channel_[MAXFD];
    std::vector<epoll_event> epoll_events_;
};

using SP_Epoll = std::shared_ptr<Epoll>;

#endif