#include <sys/eventfd.h>
#include <vector>
#include <vector>

#include "Event_loop.h"
#include "Channel.h"
#include "Epoll.h"
#include "Utils.h"

void EventLoop::AddToPoller(SP_Channel channel){
    poller_->PollAdd(channel);
}
void EventLoop::ModToPoller(SP_Channel channel){
    poller_->PollMod(channel);
}

void EventLoop::DelToPoller(SP_Channel channel){
    poller_->PollDel(channel);
}
void EventLoop::Loop(){
    std::vector<SP_Channel> ready_channels;
    while(true) {
        ready_channels.clear();
        ready_channels = poller_->WaitChannels();
        for(auto channel : ready_channels) {
            channel->HandleEvents();
        }
    }
}