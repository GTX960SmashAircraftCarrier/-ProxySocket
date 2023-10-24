#pragma once
#include <sys/epoll.h>
#include <memory>

#include "lib/Channel.h"
#include "lib/Event_loop.h"
#include "lib/Thread_Event_loop.h"
#include "lib/TransConnect.h"

class Tunnel;

class PublicConnect : public TransConnect, public std::enable_shared_from_this<PublicConnect> {
public:
    PublicConnect(int fd, SP_ThreadEventloop thread, Tunnel* tunnel, std::string proxy_id)
    :   TransConnect(fd, thread), tunnel_(tunnel), proxy_id_(proxy_id), closed_(false) {
        // 注册事件
        channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLRDHUP);
        channel_->SetReadHandler(std::bind(&PublicConnect::handleRead, this));
        channel_->SetEveryHandler(std::bind(&PublicConnect::everyHandle, this));
        channel_->SetNeedCloseWhenDelete(false);
        std::cout<<"Publickconnect  init success\n";
    }
    ~PublicConnect() { printf("publicConn killing\n"); }
private:
    void handleRead();
    void everyHandle();
private:
    bool closed_;
    Tunnel* tunnel_;
    std::string proxy_id_;
};

using SP_PublicConnect = std::shared_ptr<PublicConnect>;