#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP

#include <memory>
#include <mutex>

#include "Channel.h"
#include "Epoll.h"
#include "Utils.h"

class EventLoop {
public:
    EventLoop() : poller_(SP_Epoll(new Epoll())){};
    
    void AddToPoller(SP_Channel channel);
    void ModToPoller(SP_Channel channel);
    void DelToPoller(SP_Channel channel);
    void Loop();
private:
    SP_Event poller_;
};

using SP_EventLoop = std::shared_ptr<EventLoop>;
#endif