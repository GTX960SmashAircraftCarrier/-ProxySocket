
#include <sys/epoll.h>
#include <iostream>

#include "Channel.h"
#include "Epoll.h"
#include "spdlog/spdlog.h"


void Epoll::PollAdd(SP_Channel channel) {
    int fd = channel->getFd();
    epoll_event event;
    event.data.fd = fd;
    event.events = channel->GetEvents();
    if (epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event) < 0){
        SPDLOG_CRITICAL("epoll_ctl fd: {} err: {}", fd, strerror(errno));
    }
    else{
        fd_channel_[fd] = channel;
    }
}

void Epoll::PollMod(SP_Channel channel){
    int fd = channel->getFd();
    epoll_event event;
    event.data.fd = fd;
    event.events = channel->GetEvents();
    if (epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event) < 0){
        perror("epoll_mod error");
        fd_channel_[fd].reset();
    }
}

void Epoll::PollDel(SP_Channel channel){
    int fd = channel->getFd();
    epoll_event event;
    event.data.fd = fd;
    event.events = channel->GetEvents();
    if (epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &event) < 0){
        perror("epoll_del error");
        fd_channel_[fd].reset();
    }
}

std::vector<SP_Channel> Epoll::getReadyChannels(int event_count){
    std::vector<SP_Channel> ret;
    for(int i = 0; i < event_count; i++){
        epoll_event cur_event = epoll_events_[i];
        int fd = cur_event.data.fd;
        SP_Channel cur_channel = fd_channel_[fd];
        if(cur_channel) {
            cur_channel->SetRevents(cur_event.events);
            ret.emplace_back(cur_channel);
        }
        else {
            std::cout<< "fd" <<fd<< "not exitx in fd2chan_" <<std::endl;
        }
    }
    return ret;
}

std::vector<SP_Channel> Epoll::WaitChannels() {
    while(true) {
        int event_count = epoll_wait(fd_, &*epoll_events_.begin(), epoll_events_.size(), EPOLLWAIT_TIME);
        if(event_count < 0){
            perror("epoll wait error");
            continue;
        }

        std::vector<SP_Channel> readyChannels = getReadyChannels(event_count);
        if (readyChannels.size() > 0){
            return readyChannels;
        }
            
    }
}