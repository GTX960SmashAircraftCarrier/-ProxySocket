#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "public_connect.h"
#include "lib/Channel.h"
#include "lib/Thread_EventLoop_pool.h"
#include "lib/Thread_Event_loop.h"
#include "lib/ProxyConnect.h"
#include "lib/Utils.h"

class Control;

class Tunnel {
public:
    typedef std::shared_ptr<Control> SP_Control;
    Tunnel(std::string tunnel_id, SP_ThreadEventloop threadloop, SP_ThreadEventLoopPoll thread_poll, SP_Control ctl)
    :   tun_id_(tunnel_id),
        listen_fd_(sockt_bind_listen(7777)),
        listen_thread_(threadloop),
        poll_(thread_poll),
        ctl_(ctl),
        public_fd_mutex_(),
        free_proxy_mutex_(){
            //客户端信息
            struct sockaddr_in listenAddr;
            socklen_t listenAddrLen = sizeof(listenAddr);
            getsockname(listen_fd_, (struct sockaddr*)&listenAddr, &listenAddrLen);
            listen_port_ = ntohs(listenAddr.sin_port);
            listen_addr_ = inet_ntoa(listenAddr.sin_addr);
            //建立监听通道，设置事件，注册handler，添加至事件循环
            acceptor_ = SP_Channel(new Channel(listen_fd_));
            acceptor_->SetEvents(EPOLLIN | EPOLLET | EPOLLRDHUP);
            acceptor_->SetReadHandler(std::bind(&Tunnel::newPublicConnHandler, this));
            listen_thread_->AddChannel(acceptor_);
            std::cout<<"Tunnel init success\n";
        }

    int getListenPort() { return listen_port_; }
    std::string getListenAddr() { return listen_addr_; }
    
    int getAndPopUndealPublicFd();
    
    void freeProxyConn(std::string proxy_id);

    void shutdownPublicConn(SP_ProxyConnect proxy_connect);
    void shutdownFromPublic(std::string proxy_id, u_int32_t tran_count);

    void reqStartProxy(int public_fd, SP_ProxyConnect proxy_conn);

    void bindPublicFdToProxyConn(int public_fd, SP_ProxyConnect proxy_conn);
    
    void claimProxyConn(SP_ProxyConnect proxyconnect);
    
    SP_ProxyConnect popFreeProxyConn(bool& isempty);
    
    SP_ProxyConnect getProxyConn(std::string proxy_id, bool& isExist);
private:
    void newPublicConnHandler();
    void addCtlPendingFunctor(std::function<void()>&&);
    void handleStartProxyConnRsp(void* msg, SP_ProxyConnect proxy_connect);
    void handlePeerProxyConnClose(SP_ProxyConnect proxy_connect);
private:
    int public_fd_in_;
    int public_fd_finish_;
    std::string tun_id_;
    int listen_fd_;
    std::string listen_addr_;
    int listen_port_;
    std::mutex public_fd_mutex_;
    std::mutex free_proxy_mutex_;

    SP_ThreadEventloop listen_thread_;
    SP_ThreadEventLoopPoll poll_;
    SP_Channel acceptor_;
    SP_Control ctl_;

    threadsafe_unordered_map<std::string, SP_ProxyConnect> proxy_connect_map_;
    threadsafe_unordered_map<std::string, SP_ProxyConnect> wait_proxy_connect_map_;
    std::vector<SP_ProxyConnect> free_proxy_connects_;
    std::vector<int> unhandle_fds_;
};

using SP_Tunnel = std::shared_ptr<Tunnel>;