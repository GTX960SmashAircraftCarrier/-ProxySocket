#ifndef EVENT_HPP
#define EVENT_HPP
#include <memory>
#include <vector>
#include "Channel.h"
//event抽象类
class Event {
public:
    Event() {};
    virtual ~Event(){};
    virtual void PollAdd(SP_Channel) = 0;
    virtual void PollMod(SP_Channel) = 0;
    virtual void PollDel(SP_Channel) = 0;
    virtual std::vector<SP_Channel> WaitChannels() = 0;
};

using SP_Event = std::shared_ptr<Event>;
#endif