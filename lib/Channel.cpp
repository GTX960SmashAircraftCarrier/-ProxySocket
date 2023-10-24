#include "Channel.h"
#include <sys/epoll.h>
#include "spdlog/spdlog.h"

void Channel::HandleEvents() {
    events_ = 0;
    if(!revents_) return;

    // 对方关闭 , 回复rst，设置channel close为true
    if(revents_ & EPOLLHUP) {
        closed_ = true;
    }
    
    if (revents_ & EPOLLERR) {
        error_handler_();
    }
    // 对端调用close时，本端会收到RDHUP，EPOLLRDHUP想要被触发，需要显式地在epoll_ctl调用时设置在events中，（此时本端可能还有数据可接收？）
    if(revents_ & EPOLLRDHUP) {
        closed_ = true;
    }

    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        read_handler_();
    }
    
    if ((revents_ & EPOLLOUT) && !closed_) {
        write_handler_();
    }
    Every_handler_();
}
