#ifndef CONNECT_HPP
#define CONNECT_HPP
#include <memory>

#include "Channel.h"
#include "Event_loop.h"
//  SP_EventLoop 收集事件
//  SP_Channel 响应事件
class Connect : public ChannelMaster {
public:
    Connect(int fd, SP_EventLoop loop) : fd_(fd), loop_(loop), channel_(new Channel(fd)){}
    ~Connect() {
        loop_->DelToPoller(channel_);
    }

    SP_Channel getChannel() { return channel_;}

    int getFd() { return fd_;}
protected:
    int fd_;
    SP_EventLoop loop_;
    SP_Channel channel_;
    bool in_buffer_empty_;
};

using SP_Connect = std::shared_ptr<Connect>;

#endif