#include "Control.h"
#include <string.h>
#include "Server.h"
#include "lib/Utils.h"
#include "spdlog/spdlog.h"


void Control::notifyClientNeedProxy(std::string tun_id) {
    NotifyClientNeedProxyMsg req_msg;
    strcpy(req_msg.tunnel_id, tun_id.c_str());
    req_msg.proxy_server_port = server_->getProxyPort();
    Msg msg = MakeMsg(MsgType::NotifyClient, (char*)&req_msg, sizeof(NotifyClientNeedProxyMsg));
    connect_->SendMsg(msg);
}

void Control::shutdownFromPublic(std::string tun_id, std::string proxy_id, u_int32_t tran_count) {
    ShutdownPeerConnMsg req_msg;
    req_msg.tran_count = htonl(tran_count);
    strcpy(req_msg.tunnel_id, tun_id.c_str());
    strcpy(req_msg.proxy_id, proxy_id.c_str());
    Msg msg = MakeMsg(MsgType::ShutdownPeerConn, (char*)&req_msg, sizeof(ShutdownPeerConnMsg));
    connect_->SendMsg(msg);
}

void Control::handleNewCtlReq(void* msg, SP_CtlConnect connect) {
    std::string ctl_id = connect->GetId();
    NewResponseMsg* req_msg = (NewResponseMsg*)malloc(sizeof(NewResponseMsg) + ctl_id.size() + 1);
    memset(req_msg->id, 0, ctl_id.size() + 1);
    strcpy(req_msg->id, ctl_id.c_str());

    Msg ctl_msg = MakeMsg(MsgType::NewResponse, (char*)req_msg, sizeof(NewResponseMsg) + ctl_id.size() + 1);
    connect->SendMsg(ctl_msg);
    free(req_msg);
}

void Control::handleNewTunnelReq(void* msg, SP_CtlConnect connect) {
    NewTunnelReqMsg* req_msg = (NewTunnelReqMsg*)msg;
    std::string rand_id = random_str(5);
    SP_Tunnel tunnel(new Tunnel(rand_id, server_->publicListenThread_, server_->eventLoopThreadPool_, shared_from_this()));
    addtoTunnel(rand_id, tunnel);

    NewTunnelRspMsg rsp_msg;
    rsp_msg.server_port = req_msg->server_port;
    rsp_msg.proxy_server_port = tunnel->getListenPort();
    strcpy(rsp_msg.tunnel_id, rand_id.c_str());
    strcpy(rsp_msg.proxy_server_host, (tunnel->getListenAddr()).c_str());
    strcpy(rsp_msg.server_host, req_msg->server_host);

    Msg ctl_msg = MakeMsg(MsgType::NewTunnelRsp, (char*)&rsp_msg, sizeof(NewTunnelRspMsg));
    connect_->SendMsg(ctl_msg);
}

void Control::handleCtlConnClose(SP_CtlConnect conn) {
    server_->control_map_.erase(ctl_id_); 
}

void Control::handleShutdownPublicConn(void* msg, SP_CtlConnect conn) {
    ShutdownPeerConnMsg *req_msg = (ShutdownPeerConnMsg *)msg;
    u_int32_t theoreticalTotalRecvCount = ntohl(req_msg->tran_count);
    std::string tun_id = req_msg->tunnel_id;
    std::string proxy_id = req_msg->proxy_id;
    bool tunIsExist;
    SP_Tunnel tun = tunnel_map_.get(tun_id, tunIsExist);
    if (!tunIsExist) {
        SPDLOG_CRITICAL("tun {} not exist", tun_id);
        return;
    }

    bool proxyConnIsExist;
    SP_ProxyConnect proxyConn = tun->getProxyConn(proxy_id, proxyConnIsExist);
    if (!proxyConnIsExist) {
        SPDLOG_CRITICAL("proxy conn {} not exist", proxy_id);
        return;
    }

    proxyConn->AddTotalCount(theoreticalTotalRecvCount);
    tun->shutdownPublicConn(proxyConn);
}

void Control::handleFreeProxyConnReq(void* msg, SP_CtlConnect conn) {
    FreeProxyConnReqMsg *req_msg = (FreeProxyConnReqMsg *)msg;
    std::string tun_id = std::string(req_msg->tun_id);
    std::string proxy_id = std::string(req_msg->proxy_id);
    bool tunIsExist;
    SP_Tunnel tun = tunnel_map_.get(tun_id, tunIsExist);
    if (!tunIsExist) {
        SPDLOG_CRITICAL("tun {} not exist", tun_id);
        return;
    }
    tun->freeProxyConn(proxy_id);
}