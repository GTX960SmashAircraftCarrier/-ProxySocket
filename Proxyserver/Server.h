#pragma once
#include <sys/epoll.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include "Control.h"
#include "Tunnel.h"
#include "lib/CtlConnect.h"
#include "lib/Event_loop.h"
#include "lib/Thread_Event_loop.h"
#include "lib/Thread_EventLoop_pool.h"
#include "lib/ProxyConnect.h"
#include "lib/Utils.h"

const int UnclaimedProxyMapLen = 4;
struct UnclaimedProxyMap {
  std::unordered_map<int, SP_ProxyConnect> conns;
  std::mutex mutex;
};

class Server : public std::enable_shared_from_this<Server> {
public:
    Server(int PoolNum, int ctlPort, int ProxyPort)
    :   ctlPort_(ctlPort),
        proxyPort_(ProxyPort),
        ctlListenfd_(sockt_bind_listen(ctlPort)),
        proxyListenfd_(sockt_bind_listen(ProxyPort)),
        loop_(new EventLoop()) {
        //ctl
        std::cout<<"ctlListenfd_"<<ctlListenfd_<<std::endl;
        std::cout<<"proxyListenfd_"<<proxyListenfd_<<std::endl;
        if (ctlListenfd_ < 0) {
            perror("listen socket fail");
            abort();
        }
        ignoreSigPipe();
        if (setNoblock(ctlListenfd_) == -1) {
            perror("set non block fail");
            abort();
        }
        //ctl连接事件
        ctl_acceptor_ = SP_Channel(new Channel(ctlListenfd_));
        ctl_acceptor_->SetEvents(EPOLLIN | EPOLLET | EPOLLRDHUP);
        ctl_acceptor_->SetReadHandler(std::bind(&Server::newCtlConnHandler, this));
        ctl_acceptor_->SetEveryHandler(std::bind(&Server::everyHandler, this));
        loop_->AddToPoller(ctl_acceptor_);
        // proxy初始化
        if (proxyListenfd_ < 0) {
            perror("listen socket fail");
            abort();
        }
        if (setNoblock(proxyListenfd_) == -1) {
            perror("set non block fail");
            abort();
        }
        // 监听proxy客户端的proxy事件
        proxy_acceptor_ = SP_Channel(new Channel(proxyListenfd_));
        proxy_acceptor_->SetEvents(EPOLLIN | EPOLLET | EPOLLRDHUP);
        proxy_acceptor_->SetReadHandler(std::bind(&Server::newProxyConnHandler, this));
        proxy_acceptor_->SetEveryHandler(std::bind(&Server::everyHandler, this));
        loop_->AddToPoller(proxy_acceptor_);

        eventLoopThreadPool_ = SP_ThreadEventLoopPoll(new ThreadEventLoopPoll(PoolNum));
        // 用于监听外部客户端事件
        publicListenThread_ = SP_ThreadEventloop(new ThreadEventLoop());
    
        for (int i = 0; i < UnclaimedProxyMapLen; i++) {
            unclaimed_proxy_map_[i] = new UnclaimedProxyMap{};
        }
        std::cout<<"Server init success\n";
    }

    int getProxyPort() { return proxyPort_; };
    
    UnclaimedProxyMap* getUnclaimedProxyMapByFd(int fd);

    void Run();

public:
    std::unordered_map<std::string, SP_Control> control_map_;
    SP_ThreadEventLoopPoll eventLoopThreadPool_;
    SP_ThreadEventloop publicListenThread_;

private:
    void newCtlConnHandler();
    void newProxyConnHandler();
    void everyHandler();
    void claimProxyConn(void*, SP_ProxyConnect);
private:
    int ctlPort_;
    int proxyPort_;
    int ctlListenfd_;
    int proxyListenfd_;
    SP_EventLoop loop_;
    SP_Channel ctl_acceptor_;
    SP_Channel proxy_acceptor_;
    UnclaimedProxyMap* unclaimed_proxy_map_[UnclaimedProxyMapLen];
};