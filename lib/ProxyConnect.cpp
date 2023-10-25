#include "ProxyConnect.h"
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <sys/epoll.h>

#include "ProxyMessage.h"
#include "spdlog/spdlog.h"
#include "Utils.h"

bool ProxyConnect::shutdownFromRemote(){
    std::unique_lock<std::mutex> lock{close_mutex_};
    shutdown(conn2fd_, SHUT_WR);
    if(half_close_){
        resetConn();
        return true;
    }
    else{
        half_close_ = true;
    }
    return false;
}
bool ProxyConnect::shutdownFromLocal(){
    std::unique_lock<std::mutex> lock{close_mutex_};
    if(half_close_){
        resetConn();
        return true;
    }
    else{
        half_close_ = true;
    }
    return false;
}
void ProxyConnect::resetConn(){
    setConn2fd(0);
    resetRecvCount();
    resetTranCount();
    resettotalCount();
    start_ = false;
    half_close_ = false;
}

void ProxyConnect::SendMsg(ProxyMsg& msg){
    u_int32_t msg_len = msg.len;
    msg.len = htonl(msg.len);
    size_t write_ret = buf_->WriteToBuffer((char*)&msg, msg_len);
    if (write_ret != msg_len) {
        SPDLOG_CRITICAL("send msg err");
        return;
    }
    channel_->Addevents(EPOLLOUT);
    loop_->ModToPoller(channel_);
}
int ProxyConnect::SendMsgDirct(ProxyMsg& msg){
    std::cout<<"发送 proxymessagsse direct success\n";
    u_int32_t msg_len = msg.len;
    msg.len = htonl(msg.len);
    size_t write_ret = write(fd_, (char*)&msg, msg_len);
    if (write_ret != msg_len) {
        SPDLOG_CRITICAL("send msg err");
        return -1;
    }
    return 0;
}

void ProxyConnect::Run(SP_TransConnect connect){
    start_ = true;
    conn_ = connect;
    setConn2fd(connect->getFd());
    connect->setConn2fd(fd_);
    //传给对应线程
    (connect->getThread())->AddConnect(connect);
}

void ProxyConnect::handleRead(){
    if(start_) {
        // splice 两个描述符传输，零拷贝
        int bs = splice(fd_, NULL, pipe_fd_[1], NULL, 2048, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
        if (bs < 0) {
            SPDLOG_CRITICAL("proxy_id: {} -> pipe_fd: {} splice err: {}", proxy_id_, pipe_fd_[1],strerror(errno));
            return;
        }
        if (bs == 0) {
            in_buffer_empty_ = true;
            return;
        }
        bs = splice(pipe_fd_[0], NULL, conn2fd_, NULL, bs, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
        if (bs < 0) {
            SPDLOG_CRITICAL("proxy_id {} pipe {} -> peer_conn_fd {} splice err: {}", proxy_id_,pipe_fd_[0], conn2fd_, strerror(errno));
            return;
        }
        AddRecvCount(bs);

        if(getRecvCount() == getTotalCount()) {
            closeLocalPeerConnHandler_(shared_from_this());
        }
        return;
    }
    
    ProxyMsg msg;
    size_t max_len = sizeof(ProxyMsg);
    char *buf_ptr = (char*)&msg;
    //读消息头
    size_t read_ret = readn(fd_, buf_ptr, sizeof(u_int32_t), in_buffer_empty_);
    if (read_ret == 0 && in_buffer_empty_) {
        return;
    }
    if (read_ret != sizeof(u_int32_t)) {
        SPDLOG_CRITICAL("proxy_id {} read msg len err", proxy_id_);
        return;
    }
    buf_ptr += read_ret;
    msg.len = ntohl(msg.len);

    if(msg.len > max_len) {
        SPDLOG_CRITICAL("msg len too long");
        return;
    }
    //消息体
    size_t body_len = msg.len - sizeof(msg.len);
    read_ret = readn(fd_, buf_ptr, body_len, in_buffer_empty_);
    if (read_ret == 0 && in_buffer_empty_) {
        return;
    }
    if (read_ret != body_len) {
        SPDLOG_CRITICAL("read msg body err readNum: {}; body_len: {}", read_ret, body_len);
        return;
    }

    switch ((msg.type))
    {
    case ProxyMsgType::ProxyMetaSet : {
        ProxyMetaSetMsg proxy_meta_set_msg;
        memcpy(&proxy_meta_set_msg, msg.data, getProxyMsgBodySize(msg));
        proxyMetaSetHandler_((void*)&proxy_meta_set_msg, shared_from_this());
        break;
    }
    case ProxyMsgType::ProxyConnectReq : {
        ProxyConnectReqMsg proxy_connect_req_msg;
        memcpy(&proxy_connect_req_msg, msg.data, getProxyMsgBodySize(msg));
        startProxyConnReqHandler_((void*)&proxy_connect_req_msg, shared_from_this());
        break;
    }
    case ProxyMsgType::ProxyConnectRsp : {
        ProxyConnectRspMsg proxy_connect_rsq_msg;
        memcpy(&proxy_connect_rsq_msg, msg.data, getProxyMsgBodySize(msg));
        startProxyConnRspHandler_((void*)&proxy_connect_rsq_msg, shared_from_this());
        break;
    } 

    }
}
void ProxyConnect::handleWrite(){
    if(buf_->getunused() <= 0) {
        return;
    }
    size_t sent_ret = buf_->WriteToSocket(fd_);
    if(sent_ret <= 0) SPDLOG_CRITICAL("sent to fd err");
    if(buf_->getunused() > 0) channel_->Addevents(EPOLLOUT);
}

void ProxyConnect::everyHandle(){
    if (in_buffer_empty_ && channel_->IsClosed()) {
        close_handler_(shared_from_this());
        return;
    }
    channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLRDHUP);
    //未完全发送
    if(buf_->getunused() > 0) {
        channel_->Addevents(EPOLLOUT);
    }
    loop_->ModToPoller(channel_);
}