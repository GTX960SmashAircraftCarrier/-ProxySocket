#ifndef PROXCONNECT_HPP
#define PROXCONNECT_HPP

#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <iostream>
#include "ProxyMessage.h"
#include "Buffer.h"
#include "Connect.h"
#include "Event_loop.h"
#include "TransConnect.h"


class ProxyConnect : public TransConnect, public std::enable_shared_from_this<ProxyConnect> {
public:
    typedef std::shared_ptr<ProxyConnect> SP_ProxyConnect;
    typedef std::function<void(void*, SP_ProxyConnect)> MsgHandler;
    ProxyConnect(int fd, SP_ThreadEventloop thread) : TransConnect{fd, thread}, close_mutex_(), buf_(new Buffer(1024, 40960)), start_(false), half_close_(false){
        channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLOUT | EPOLLRDHUP);
        channel_->SetReadHandler(std::bind(&ProxyConnect::handleRead, this));
        channel_->SetWriteHandler(std::bind(&ProxyConnect::handleWrite, this));
        channel_->SetEveryHandler(std::bind(&ProxyConnect::everyHandle, this));
        std::cout<<"ProxyConnect init success\n";
    }
    ~ProxyConnect(){ std::cout<<"proxy connect is clean\n"; }

    int getPeerConnFd() { return conn_->getFd(); }
    void setProxyID(std::string proxy_id) { proxy_id_ = proxy_id; }
    std::string getProxyID() { return proxy_id_; }
    bool is_start() { return start_; }
    
    bool shutdownFromRemote();
    bool shutdownFromLocal();
    void resetConn();

    void setProxyMetaSetHandler(MsgHandler handler) { proxyMetaSetHandler_ = handler; }
    void setStartProxyConnReqHandler_(MsgHandler handler) { startProxyConnReqHandler_ = handler; }
    void setStartProxyConnRspHandler_(MsgHandler handler) { startProxyConnRspHandler_ = handler; }
    void SetCloseHandler(std::function<void(SP_ProxyConnect connect)> handler) { close_handler_ = handler; }
    void setCloseLocalPeerConnHandler_(std::function<void(SP_ProxyConnect connect)> handler) { closeLocalPeerConnHandler_ = handler; }

    void SendMsg(ProxyMsg& msg);
    int SendMsgDirct(ProxyMsg& msg);
    void Run(SP_TransConnect connect);
private:
    void handleRead();
    void handleWrite();
    void everyHandle();
    MsgHandler proxyMetaSetHandler_ = [](void*, SP_ProxyConnect) {};
    MsgHandler startProxyConnReqHandler_ = [](void*, SP_ProxyConnect) {};
    MsgHandler startProxyConnRspHandler_ = [](void*, SP_ProxyConnect) {};
    std::function<void(SP_ProxyConnect conn)> close_handler_ = [](SP_ProxyConnect) {};
    std::function<void(SP_ProxyConnect conn)> closeLocalPeerConnHandler_ = [](SP_ProxyConnect) {};
private:
    std::mutex close_mutex_;
    bool start_;
    bool half_close_;
    SP_Buffer buf_;
    SP_Connect conn_;
    std::string proxy_id_;
};

using SP_ProxyConnect = std::shared_ptr<ProxyConnect>;

#endif