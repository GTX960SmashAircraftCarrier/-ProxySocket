#pragma once
#include <memory>
#include <unordered_map>
#include "Tunnel.h"
#include "lib/CtlConnect.h"
class Server;

class Control : public std::enable_shared_from_this<Control> {
public:
    Control(int fd, std::string ctl_id, SP_EventLoop loop, Server* server)
    :   connect_fd_(fd),
        ctl_id_(ctl_id),
        loop_(loop),
        server_(server){
            connect_ = SP_CtlConnect(new CtlConnect(fd, loop));
            connect_->SetCtlId(ctl_id);
            connect_->SetNewCtlReqHandler(std::bind(&Control::handleNewCtlReq, this, std::placeholders::_1, std::placeholders::_2));
            connect_->SetNewTunnelReqHandler(std::bind(&Control::handleNewTunnelReq, this, std::placeholders::_1, std::placeholders::_2));
            connect_->SetCloseHandler(std::bind(&Control::handleCtlConnClose, this, std::placeholders::_1));
            connect_->SetNotifyProxyShutdownPeerConnHandler(std::bind(&Control::handleShutdownPublicConn, this, std::placeholders::_1, std::placeholders::_2));
            connect_->SetFreeProxyConnReqHandler(std::bind(&Control::handleFreeProxyConnReq, this, std::placeholders::_1, std::placeholders::_2));
            loop_->AddToPoller(connect_->getChannel());
            std::cout<<"Server Control init success\n";
        }
    
    SP_CtlConnect getCtlConnect() { return connect_;}
    void addtoTunnel(std::string tun_id, SP_Tunnel tun) { tunnel_map_.add(tun_id, tun); };
    
    void notifyClientNeedProxy(std::string tun_id);
    void shutdownFromPublic(std::string tun_id, std::string proxy_id, u_int32_t tran_count);

private:
    void handleNewCtlReq(void* msg, SP_CtlConnect connect);
    void handleNewTunnelReq(void* msg, SP_CtlConnect connect);
    void handleCtlConnClose(SP_CtlConnect conn);
    void handleShutdownPublicConn(void* msg, SP_CtlConnect conn);
    void handleFreeProxyConnReq(void* msg, SP_CtlConnect conn);
private:
    friend class Server;
    int connect_fd_;
    std::string ctl_id_;
    SP_CtlConnect connect_;
    SP_EventLoop loop_;
    Server* server_;
    threadsafe_unordered_map<std::string, SP_Tunnel> tunnel_map_;

};

using SP_Control = std::shared_ptr<Control>;