#include "Server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <exception>
#include <functional>
#include <iostream>

#include "lib/CtlConnect.h"
#include "lib/Thread_Event_loop.h"
#include "lib/Thread_EventLoop_pool.h"
#include "lib/Message.h"
#include "lib/ProxyConnect.h"
#include "lib/Utils.h"
    
UnclaimedProxyMap* Server::getUnclaimedProxyMapByFd(int fd) {
    int idx = fd % UnclaimedProxyMapLen;
    return unclaimed_proxy_map_[idx];
}

void Server::Run() try{
    eventLoopThreadPool_->Run();
    // 等待外部事件
    publicListenThread_->StartRun();
    loop_->Loop();
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    abort();
}

void Server::newCtlConnHandler() try{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while ((accept_fd = accept(ctlListenfd_, (struct sockaddr *)&client_addr, &client_addr_len)) >0) {
        SPDLOG_INFO("ctl_accept_fd: {}; from:{}; port:{}", accept_fd, inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port));
        std::string ctl_id = random_str(5);
        while (control_map_.find(ctl_id) != control_map_.end()) {
            ctl_id = random_str(5);
        }
        SP_Control ctl(new Control(accept_fd, ctl_id, loop_, this));
        control_map_.emplace(ctl_id, ctl);
    }
} catch (std::exception& e){
    std::cout << "new ctl conn handler err: " << e.what() << std::endl;
} 

void Server::newProxyConnHandler() try {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    memset(&client_addr, 0, client_addr_len);
    int accept_fd = 0;
    while ((accept_fd = accept(proxyListenfd_, (struct sockaddr *)&client_addr, &client_addr_len)) >0) {
        SPDLOG_INFO("proxy_accept_fd: {}; from:{}; port:{}", accept_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        // 封装ProxyConn,并选择一个工作线程处理
        SP_ThreadEventloop threadPicked = eventLoopThreadPool_->PickRandThread();
        SP_ProxyConnect proxyConn(new ProxyConnect(accept_fd, threadPicked));
        proxyConn->setProxyMetaSetHandler(std::bind(&Server::claimProxyConn, this, std::placeholders::_1, std::placeholders::_2));

        // 选择一个未认领proxy的map
        UnclaimedProxyMap *unclaimedProxyMap = getUnclaimedProxyMapByFd(accept_fd);
        {
            std::unique_lock<std::mutex> lock(unclaimedProxyMap->mutex);
            (unclaimedProxyMap->conns).emplace(accept_fd, proxyConn);
        }
        threadPicked->AddConnect(proxyConn);
    }
} catch (std::exception& e) {
    std::cout << "new proxy conn handler err: " << e.what() << std::endl;
}

void Server::everyHandler() {
    ctl_acceptor_->SetEvents(EPOLLIN | EPOLLET | EPOLLRDHUP);
    loop_->ModToPoller(ctl_acceptor_);
}

void Server::claimProxyConn(void* msg, SP_ProxyConnect conn) {
    ProxyMetaSetMsg *proxy_meta_set_msg = (ProxyMetaSetMsg *)msg;
    std::string ctl_id = std::string(proxy_meta_set_msg->ctl_id);
    std::string tun_id = std::string(proxy_meta_set_msg->tun_id);
    std::string proxy_id = std::string(proxy_meta_set_msg->proxy_id);

    conn->setProxyID(proxy_id);
    if (control_map_.find(ctl_id) == control_map_.end()) {
        SPDLOG_CRITICAL("control {} is not exist", ctl_id);
        return;
    }
    SP_Control ctl = control_map_[ctl_id];
    if (!(ctl->tunnel_map_).isExist(tun_id)) {
        SPDLOG_CRITICAL("tun {} is not exist in ctl {}", tun_id, ctl_id);
        return;
    }
    SP_Tunnel tun = (ctl->tunnel_map_).get(tun_id);
    tun->claimProxyConn(conn);
    // proxyConn已被认领，需从unclaimedProxyMaps中去掉
    int proxy_conn_fd = conn->getFd();
    UnclaimedProxyMap *unclaimedProxyMap = getUnclaimedProxyMapByFd(proxy_conn_fd);
    {
        std::unique_lock<std::mutex> lock(unclaimedProxyMap->mutex);
        (unclaimedProxyMap->conns).erase(proxy_conn_fd);
    }
}