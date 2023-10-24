#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <unistd.h>
#include <functional>
#include <memory>

class ChannelMaster {};
using SP_ChannelMaster = std::shared_ptr<ChannelMaster>;
// 文件描述符事件
class Channel {
public:
    typedef std::function<void()> EventHandler;
    Channel(int fd) : fd_(fd), close_and_delete_(true){};
    ~Channel() {
        if (close_and_delete_) {
            close(fd_);
        }
    }

    void setFd(int fd){ fd_ = fd;}
    int getFd(){ return fd_;}
    //设置监视事件
    void SetEvents(__uint32_t events) { events_ = events; };
    void SetRevents(__uint32_t events) { revents_ = events; };
    //添加事件
    void Addevents(__uint32_t events) { events_ |= events; }
    __uint32_t GetEvents() { return events_; }
    __uint32_t GetRevents() { return revents_; }
    //设置 读 写 错误 循环事件
    void SetReadHandler(EventHandler handler) { read_handler_ = handler; };
    void SetWriteHandler(EventHandler handler) { write_handler_ = handler; };
    void SetErrorHandler(EventHandler handler) { error_handler_ = handler; }
    void SetEveryHandler(EventHandler handler) { Every_handler_ = handler; }
    
    void SetChannelOwner(SP_ChannelMaster master) { master_ = master; }
    bool IsClosed() { return closed_; }
    void SetNeedCloseWhenDelete(bool needClose) { close_and_delete_ = needClose; }
    //处理接受事件
    void HandleEvents();
private:
    int fd_;
    bool close_and_delete_;
    __uint32_t events_;
    __uint32_t revents_;
    EventHandler read_handler_ = [](){};
    EventHandler write_handler_ = [](){};
    EventHandler error_handler_ = [](){};
    EventHandler Every_handler_ = [](){};
    SP_ChannelMaster master_;
    bool closed_;
};

using SP_Channel =  std::shared_ptr<Channel>;

#endif