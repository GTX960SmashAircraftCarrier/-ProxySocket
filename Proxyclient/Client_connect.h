#ifndef CLIENT_CONNECT_HPP
#define CLIENT_CONNECT_HPP

#include <sys/syscall.h>
#include <unistd.h>
#include <memory>
#include <sys/epoll.h>
#include <iostream>
#include "lib/Event_loop.h"
#include "lib/Thread_Event_loop.h"
#include "lib/TransConnect.h"

class Tunnel;

class ClientConnect : public TransConnect, public std::enable_shared_from_this<ClientConnect> {
public:
    ClientConnect(int fd, SP_ThreadEventloop thread, Tunnel* tunnel, std::string proxy_id) : TransConnect(fd, thread), tunnel_(tunnel), proxy_id_(proxy_id), close_(false) {
        
        channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLRDHUP);
        channel_->SetReadHandler(std::bind(&ClientConnect::handleRead, this));
        channel_->SetEveryHandler(std::bind(&ClientConnect::everyHandle, this));
        channel_->SetNeedCloseWhenDelete(false);
        std::cout<<"localServerconnect  init success\n";
    }
    ~ClientConnect(){
        std::cout<<"ClientConnect is close\n";
    }
private:
    void handleRead();
    void everyHandle();
private:
    //使用指针或引用来替代对象作为成员变量，可以编译
    Tunnel* tunnel_;
    std::string proxy_id_;
    bool close_;
};
using SP_ClientConnect = std::shared_ptr<ClientConnect>;
#endif