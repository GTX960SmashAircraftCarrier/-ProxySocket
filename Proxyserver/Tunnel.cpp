#include "Tunnel.h"
#include <string.h>
#include <algorithm>

#include "Control.h"
#include "public_connect.h"
#include "lib/CtlConnect.h"
#include "lib/Thread_Event_loop.h"
#include "lib/ProxyConnect.h"

int Tunnel::getAndPopUndealPublicFd() {
    int fd;
    {
        std::unique_lock<std::mutex> lock(public_fd_mutex_);
        fd = unhandle_fds_.back();
        unhandle_fds_.pop_back();
    }
    return fd;
}

void Tunnel::freeProxyConn(std::string proxy_id) {
    bool proxy_exist;
    SP_ProxyConnect proxy_connect = proxy_connect_map_.get(proxy_id, proxy_exist);
    if (!proxy_exist) {
        SPDLOG_CRITICAL("proxy_id {} not exist", proxy_id);
        return;
    }
    if (proxy_connect->is_start()) {
        SPDLOG_CRITICAL("proxy_id {} is starting", proxy_id);
        return;
    }

    proxy_connect_map_.erase(proxy_id);
    {
        std::unique_lock<std::mutex> lock(free_proxy_mutex_);
        free_proxy_connects_.emplace_back(proxy_connect);
    }
}

void Tunnel::shutdownPublicConn(SP_ProxyConnect proxy_connect) {
    int totalRecvConunt = proxy_connect->getTotalCount();
    int RecvCount = proxy_connect->getRecvCount();
    std::cout<<"TotalRecv: "<<totalRecvConunt<<"local Serer send: "<<RecvCount<<std::endl;
    if (RecvCount != totalRecvConunt){
        std::cout<<"收到和本地服务器发送数据不对, TotalRecv: "<<totalRecvConunt<<"local Serer send: "<<RecvCount<<std::endl;
        return;
    }
        
    
    bool Free = proxy_connect->shutdownFromRemote();
    if (Free) {
        freeProxyConn(proxy_connect->getProxyID());
    }
}

void Tunnel::shutdownFromPublic(std::string proxy_id, u_int32_t tran_count) {
    std::cout<<"Public Client close\n";
    bool isproxyExit;
    SP_ProxyConnect proxy_connect = proxy_connect_map_.get(proxy_id, isproxyExit);
    if (!isproxyExit)
        return;
    proxy_connect->shutdownFromLocal();
    ctl_->shutdownFromPublic(tun_id_, proxy_id, tran_count);
}

void Tunnel::reqStartProxy(int public_fd, SP_ProxyConnect proxy_conn) {
    std::string proxy_id = proxy_conn->getProxyID();
    ProxyConnectReqMsg req_msg;
    strcpy(req_msg.proxy_id, proxy_id.c_str());
    req_msg.fd = htonl(public_fd);

    ProxyMsg proxy_msg = makeProxyMsg(ProxyMsgType::ProxyConnectReq, (char*)&req_msg, sizeof(ProxyConnectReqMsg));
    proxy_conn->SendMsg(proxy_msg);

}

void Tunnel::bindPublicFdToProxyConn(int public_fd, SP_ProxyConnect proxy_conn) {
    SP_PublicConnect public_connect(new PublicConnect(public_fd, proxy_conn->getThread(), this, proxy_conn->getProxyID()));
    proxy_conn->Run(public_connect);
    proxy_connect_map_.add(proxy_conn->getProxyID(), proxy_conn);
}

void Tunnel::claimProxyConn(SP_ProxyConnect proxyconnect) {
    proxyconnect->setStartProxyConnRspHandler_(std::bind(&Tunnel::handleStartProxyConnRsp, this, std::placeholders::_1, std::placeholders::_2));
    proxyconnect->SetCloseHandler(std::bind(&Tunnel::handlePeerProxyConnClose, this, std::placeholders::_1));
    proxyconnect->setCloseLocalPeerConnHandler_(std::bind(&Tunnel::shutdownPublicConn, this, std::placeholders::_1));
    int publicfd = getAndPopUndealPublicFd();
    bindPublicFdToProxyConn(publicfd, proxyconnect);
}

SP_ProxyConnect Tunnel::popFreeProxyConn(bool& isempty) {
    std::unique_lock<std::mutex> lock(free_proxy_mutex_);
    if (free_proxy_connects_.empty()) {
        isempty = true;
        return SP_ProxyConnect{};
    }
    SP_ProxyConnect proxy_connect = free_proxy_connects_.back();
    free_proxy_connects_.pop_back();
    isempty = false;
    return proxy_connect;
}

SP_ProxyConnect Tunnel::getProxyConn(std::string proxy_id, bool& isExist) {
    return proxy_connect_map_.get(proxy_id, isExist);
}

//外部连接
void Tunnel::newPublicConnHandler() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while ((accept_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0) {
        SPDLOG_INFO("public_accept_fd: {}; from:{}; port:{}", accept_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        bool freeEmpty;
        
        SP_ProxyConnect proxy_connect = popFreeProxyConn(freeEmpty);
        if (!freeEmpty) {
            std::cout<<"复用proxy连接\n";
            reqStartProxy(accept_fd, proxy_connect);
            wait_proxy_connect_map_.add(proxy_connect->getProxyID(), proxy_connect);
            continue;
        }

        {
            std::unique_lock<std::mutex> lock(public_fd_mutex_);
            unhandle_fds_.push_back(accept_fd);
        }
        
        // 请求生成一个proxyConn
        ctl_->notifyClientNeedProxy(tun_id_);
            
    }
}

void Tunnel::addCtlPendingFunctor(std::function<void()>&&) {

}

void Tunnel::handleStartProxyConnRsp(void* msg, SP_ProxyConnect proxy_connect) {
    ProxyConnectRspMsg* rsp_msg = (ProxyConnectRspMsg*)msg;
    u_int32_t public_fd = ntohl(rsp_msg->fd);
    std::string proxy_id = proxy_connect->getProxyID();
    if (!wait_proxy_connect_map_.isExist(proxy_id)) {
        SPDLOG_CRITICAL("proxy_id {} not exist in wait_for_start_proxy_conn_map", proxy_id);
        return;
    }
    bindPublicFdToProxyConn(public_fd, proxy_connect);
    wait_proxy_connect_map_.erase(proxy_id);
}

void Tunnel::handlePeerProxyConnClose(SP_ProxyConnect proxy_connect) {
    std::string proxy_id = proxy_connect->getProxyID();
    proxy_connect_map_.erase(proxy_id);
    {
        std::unique_lock<std::mutex> lock(free_proxy_mutex_);
        std::remove(free_proxy_connects_.begin(), free_proxy_connects_.end(), proxy_connect);
    }
}