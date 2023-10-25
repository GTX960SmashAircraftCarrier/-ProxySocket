#include <assert.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <functional>
#include <iostream>
#include <exception>
#include "Client.h"
#include "Client_connect.h"
#include "Tunnel.h"
#include "lib/Channel.h"
#include "lib/CtlConnect.h"
#include "lib/Event_loop.h"
#include "lib/Thread_EventLoop_pool.h"
#include "lib/ProxyConnect.h"
#include "lib/Utils.h"
#include "spdlog/spdlog.h"

void Client::Run() {
    thead_event_pool_->Run();
    initConnect();
    NewCtlReq();
    loop_->Loop();
}

void Client::initConnect() {
    //TCP连接
    int connect_fd = tcp_connect(proxy_server_host_.c_str(), proxy_server_port_);
    assert(connect_fd > 0);
    SP_CtlConnect connect(new CtlConnect(connect_fd, loop_));
    ctlconnect_ = connect;
    ctlconnect_->SetNewCtlRspHandler(std::bind(&Client::handleNewCtlRsp, this, std::placeholders::_1, std::placeholders::_2));
    ctlconnect_->SetCloseHandler(std::bind(&Client::handleCtlConnClose, this,  std::placeholders::_1));
    ctlconnect_->SetNewTunnelRspHandler(std::bind(&Client::handleNewTunnelRsp, this, std::placeholders::_1, std::placeholders::_2));
    ctlconnect_->SetNotifyClientNeedProxyHandler(std::bind(&Client::handleProxyNotify, this, std::placeholders::_1, std::placeholders::_2));
    ctlconnect_->SetNotifyProxyShutdownPeerConnHandler(std::bind(&Client::handleShutdownLocalConn, this, std::placeholders::_1, std::placeholders::_2));
    loop_->AddToPoller(ctlconnect_->getChannel());
}

void Client::NewCtlReq() {
    NewRequestMsg req_msg = NewRequestMsg{};
    Msg msg = MakeMsg(MsgType::NewRequest, (char *)(&req_msg), sizeof(req_msg));
    ctlconnect_->SendMsg(msg);
}

void Client::NewTunnelReq() {
    NewTunnelReqMsg req_msg;
    req_msg.server_port = server_port_;
    strcpy(req_msg.server_host, server_host_.c_str());
    Msg msg = MakeMsg( MsgType::NewTunnelReq, (char *)(&req_msg), sizeof(req_msg));
    ctlconnect_->SendMsg(msg);
}

void Client::handleNewCtlRsp(void* msg, SP_CtlConnect conn) {
    NewResponseMsg *new_rsp_msg = (NewResponseMsg *)msg;
    conn->SetCtlId(std::string(new_rsp_msg->id));
    client_id_ = std::string(new_rsp_msg->id);
    NewTunnelReq();
}

void Client::handleNewTunnelRsp(void* msg, SP_CtlConnect conn) {
    NewTunnelRspMsg *rsp_msg = (NewTunnelRspMsg *)msg;
    std::string tun_id = std::string(rsp_msg-> tunnel_id);
    std::string local_server_host = std::string(rsp_msg->server_host);
    std::string proxy_server_host = std::string(rsp_msg->proxy_server_host);
    SP_Tunnel tun(new Tunnel{tun_id, local_server_host, rsp_msg->server_port, proxy_server_host,
                            rsp_msg->proxy_server_port,  thead_event_pool_, this});
    SPDLOG_INFO("tunnel addr:{}:{}", rsp_msg->proxy_server_host, rsp_msg->proxy_server_port);
    tunnel_map_.emplace(rsp_msg-> tunnel_id, tun);
}

void Client::handleProxyNotify(void*msg, SP_CtlConnect) {
    NotifyClientNeedProxyMsg *req_msg = (NotifyClientNeedProxyMsg*)msg;
    std::string tunnel_id = req_msg->tunnel_id;
    if(tunnel_map_.find(tunnel_id) == tunnel_map_.end()) {
        SPDLOG_CRITICAL("tun_id {} not exist", tunnel_id);
        return;
    }
    SP_Tunnel tunnel = tunnel_map_[tunnel_id];
    SP_ProxyConnect proxyconnect = tunnel->createProxyConn(req_msg->proxy_server_port);
    
    (tunnel->proxy_connect_map).add(proxyconnect->getProxyID(), proxyconnect);

    SP_ClientConnect clientconnect = tunnel->createLocalConn(proxyconnect);
    //链接信息
    ProxyMetaSetMsg meat_set_msg = ProxyMetaSetMsg{};
    strcpy(meat_set_msg.ctl_id, client_id_.c_str());
    strcpy(meat_set_msg.tun_id, tunnel_id.c_str());
    strcpy(meat_set_msg.proxy_id, (proxyconnect->getProxyID()).c_str());
    ProxyMsg proxy_msg  = makeProxyMsg( ProxyMsgType::ProxyMetaSet, (char*)& meat_set_msg, sizeof(meat_set_msg));
    if(proxyconnect->SendMsgDirct(proxy_msg) == -1){
        SPDLOG_CRITICAL("proxyConn {} send ProxyMetaSetMsg failed", proxyconnect->getProxyID());
        return;
    }
    // proxyconnect->SendMsg(proxy_msg);
    proxyconnect->Run(clientconnect);
}

void Client::handleShutdownLocalConn(void* msg, SP_CtlConnect) {
    ShutdownPeerConnMsg *req_msg = (ShutdownPeerConnMsg*)msg;
    std::string tun_id = req_msg->tunnel_id;
    std::string proxy_id = req_msg->proxy_id;
    u_int32_t TotalRecvCount = ntohl(req_msg->tran_count);
    if (tunnel_map_.find(tun_id) == tunnel_map_.end()) {
        SPDLOG_CRITICAL("tun_id {} not exist", tun_id);
        return;
    }

    SP_Tunnel tun = tunnel_map_[tun_id];
    bool isExit;
    SP_ProxyConnect proxyconnect = (tun->proxy_connect_map).get(proxy_id, isExit);
    if (!isExit) {
        SPDLOG_CRITICAL("proxy_id: {} not exist", proxy_id);
        return;
    }
    proxyconnect->AddTotalCount(TotalRecvCount);
    tun->shutdonwLocalConn(proxyconnect);
}


void Client::shutdownLocal(std::string tun_id, std::string proxy_id, uint32_t trans_count){
    ShutdownPeerConnMsg req_msg;
    req_msg.tran_count = htonl(trans_count);
    strcpy(req_msg.tunnel_id, tun_id.c_str());
    strcpy(req_msg.proxy_id, proxy_id.c_str());
    Msg msg = MakeMsg(MsgType::ShutdownPeerConn, (char*)&req_msg, sizeof(ShutdownPeerConnMsg));
    ctlconnect_->SendMsg(msg);
}