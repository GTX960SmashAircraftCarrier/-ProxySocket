#include <netinet/in.h>
#include <string.h>

#include "Client.h"
#include "Tunnel.h"
#include "lib/ProxyConnect.h"
#include "lib/Utils.h"
#include "spdlog/spdlog.h"

std::string Tunnel::getValidProxyID() {
    std::string proxy_id = random_str(5);
        while (proxy_connect_map.isExist(proxy_id)) {
            proxy_id = random_str(5);
        }
    return proxy_id;
}

void Tunnel::shutdownFromClient(std::string proxy_id, u_int32_t tran_count) {
    std::cout<<"本地发送完毕，准备关闭本地连接\n";
    bool proxyExit;
    SP_ProxyConnect proxyconnect = proxy_connect_map.get(proxy_id, proxyExit);
    if(!proxyExit) {
        SPDLOG_CRITICAL("proxy_id: {} not exist", proxy_id);
        return;
    }
    proxyconnect->shutdownFromLocal();
    client_->shutdownLocal(tunnel_id_, proxy_id, tran_count);
}
void Tunnel::shutdonwLocalConn(SP_ProxyConnect proxyconnect) {
    std::cout<<"关闭本地连接中\n";
    if (proxyconnect->getTotalCount() != proxyconnect->getRecvCount()) {
            std::cout<<"存在未发送完数据\n";
            return;
        }
        bool isFree = proxyconnect->shutdownFromRemote();
        if (isFree) {
            FreeProxyConnReqMsg msg;
            strcpy(msg.tun_id, tunnel_id_.c_str());
            strcpy(msg.proxy_id, (proxyconnect->getProxyID()).c_str());
            Msg sendmsg = MakeMsg(MsgType::FreeConnReq, (char*)&msg, sizeof(FreeProxyConnReqMsg));
            (client_->getConnect())->SendMsg(sendmsg);
    }
}

SP_ProxyConnect Tunnel::createProxyConn(u_int32_t proxy_port) {
    // std::cout<<"proxyhost: "<<client_->getProxyServerHost()<<" port: "<<proxy_port<<std::endl;
    int proxy_connect_fd = tcp_connect(client_->getProxyServerHost().c_str(), proxy_port);
    if (proxy_connect_fd <= 0) {
        SPDLOG_CRITICAL("connect proxy port error");
        return SP_ProxyConnect{};
    }
    //线程池取出
    SP_ThreadEventloop thread = work_pool_->PickRandThread();
    SP_ProxyConnect proxyConnect(new ProxyConnect(proxy_connect_fd, thread));
    proxyConnect->setStartProxyConnReqHandler_(std::bind(&Tunnel::HandleProxyConnectReq, this, std::placeholders::_1, std::placeholders::_2));
    proxyConnect->setCloseLocalPeerConnHandler_(std::bind(&Tunnel::shutdonwLocalConn, this, std::placeholders::_1));

    thread->AddConnect(proxyConnect);
    //生成随机ID
    std::string proxyid = getValidProxyID();
    proxyConnect->setProxyID(proxyid);
    return proxyConnect;
}
SP_ClientConnect Tunnel::createLocalConn(SP_ProxyConnect ProxyConnect) {
    int local_fd = tcp_connect(server_host_.c_str(), server_port_);
    if (local_fd <= 0) {
        SPDLOG_CRITICAL("connect proxy port error");
        return SP_ClientConnect{};
    }
    SP_ClientConnect clientconn(new ClientConnect(local_fd, ProxyConnect->getThread(), this, ProxyConnect->getProxyID()));
    return clientconn;
}

void Tunnel::HandleProxyConnectReq(void* msg, SP_ProxyConnect proxy_connect) {
    ProxyConnectReqMsg* proxy_connect_req_msg = (ProxyConnectReqMsg*)msg;
    std::string proxy_id = proxy_connect_req_msg->proxy_id;
    u_int32_t fd = ntohl(proxy_connect_req_msg->fd);
    bool isExit;
    SP_ProxyConnect proxyconnect = proxy_connect_map.get(proxy_id, isExit);
    if(!proxy_connect) {
        SPDLOG_CRITICAL("proxyConn {} not exist", proxy_id);
        return;
    }
    if(proxy_connect->is_start()) {
        SPDLOG_CRITICAL("proxyConn {} is starting, cannot start again", proxy_id);
        return;
    }
    SP_ClientConnect ClientConnect = createLocalConn(proxyconnect);
    
    ProxyConnectRspMsg proxy_connect_rsp_msg;
    strcpy(proxy_connect_rsp_msg.proxy_id, proxy_id.c_str());
    proxy_connect_rsp_msg.fd = htonl(fd);

    ProxyMsg ctl_msg = makeProxyMsg(ProxyMsgType::ProxyConnectRsp, (char*)&proxy_connect_rsp_msg, sizeof(ProxyConnectRspMsg));
    if(proxyconnect->SendMsgDirct(ctl_msg) == -1){
        SPDLOG_CRITICAL("proxyConn {} send StartProxyConnRspMsg fail", proxy_id);
        return;
    }
    proxyconnect->Run(ClientConnect);
}