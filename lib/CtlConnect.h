#ifndef CTLCONNECT_HPP
#define CTLCONNECT_HPP

#include <memory>
#include <mutex>


#include "Buffer.h"
#include "Connect.h"
#include "Event_loop.h"
#include "Message.h"


class CtlConnect : public Connect, public std::enable_shared_from_this<CtlConnect> {
public:
    typedef std::shared_ptr<CtlConnect> SP_CtlConnect;
    typedef std::function<void(void*, SP_CtlConnect)> MsgHandler;
    CtlConnect(int fd, SP_EventLoop loop) : Connect{fd, loop}, out_buf_(new Buffer(1024, 40960)), mutex_(){
        channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLOUT | EPOLLRDHUP);
        channel_->SetReadHandler(std::bind(&CtlConnect::HandlerRead, this));
        channel_->SetWriteHandler(std::bind(&CtlConnect::HandlerWrite, this));
        channel_->SetEveryHandler(std::bind(&CtlConnect::HandlerEvery, this));
        std::cout<<"CtlConnect init success\n";
    }

    ~CtlConnect() { std::cout<<"ctl clean\n";}

    int GetFd() { return fd_;}
    std::string GetId() { return ctl_id_;}
    void SetCtlId(std::string id) {ctl_id_ = id;}
    
    void SetNewCtlReqHandler(MsgHandler handler) { new_ctl_req_handler_ = handler; }
    void SetNewCtlRspHandler(MsgHandler handler) { new_ctl_rsp_handler_ = handler; }
    void SetNewTunnelReqHandler(MsgHandler handler) { new_tunnel_req_handler_ = handler; }
    void SetNewTunnelRspHandler(MsgHandler handler) { new_tunnel_rsp_handler_ = handler; }
    void SetCloseHandler(std::function<void(SP_CtlConnect connect)> handler) { close_handler_ = handler; }
    void SetNotifyClientNeedProxyHandler(MsgHandler handler) { notify_client_need_proxy_handler_ = handler; }
    void SetNotifyProxyShutdownPeerConnHandler(MsgHandler handler) { notify_proxy_shutdown_peer_conn_handler_ = handler; }
    void SetFreeProxyConnReqHandler(MsgHandler handler) { free_proxy_conn_req_handler_ = handler; }
    
    void SendMsg(Msg& msg);

private:
    void HandlerRead();
    void HandlerWrite();
    //检查关闭
    void HandlerEvery();

private:
    SP_Buffer out_buf_;
    std::string ctl_id_;
    std::mutex mutex_;
    std::function<void(SP_CtlConnect connect)> close_handler_ = [](SP_CtlConnect) {};
    
    MsgHandler new_tunnel_req_handler_ = [](void*, SP_CtlConnect) {};
    MsgHandler new_tunnel_rsp_handler_ = [](void*, SP_CtlConnect) {};
    MsgHandler new_ctl_req_handler_ = [](void*, SP_CtlConnect) {};
    MsgHandler new_ctl_rsp_handler_ = [](void*, SP_CtlConnect) {};
    MsgHandler notify_client_need_proxy_handler_ = [](void*, SP_CtlConnect) {};
    MsgHandler notify_proxy_shutdown_peer_conn_handler_ = [](void*, SP_CtlConnect) {};
    MsgHandler free_proxy_conn_req_handler_ = [](void*, SP_CtlConnect) {};
};

using SP_CtlConnect = std::shared_ptr<CtlConnect>;

#endif