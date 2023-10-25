#include "CtlConnect.h"
#include <netdb.h>
#include <string.h>
#include <exception>
#include <functional>
#include <sys/epoll.h>

#include "Buffer.h"
#include "Message.h"
#include "Utils.h"
#include "spdlog/spdlog.h"

void CtlConnect::SendMsg(Msg& msg){
    std::unique_lock<std::mutex> lock(mutex_);
    u_int32_t msg_len = msg.len;
    msg.len = htonl(msg.len);
    size_t write_num = out_buf_->WriteToBuffer((char *)&msg, msg_len);
    if(write_num != msg_len){
        SPDLOG_CRITICAL("send msg err");
        return;
    }
    channel_->Addevents(EPOLLOUT);
    loop_->ModToPoller(channel_);
}

void  CtlConnect::HandlerRead() try{
    Msg msg;
    size_t max_len = sizeof(Msg);
    char *buf_ptr = (char*)&msg;
    size_t read_num = readn(fd_, buf_ptr, sizeof(u_int32_t), in_buffer_empty_);
    if (read_num == 0 && in_buffer_empty_) {
        return;
    }
    if (read_num != sizeof(u_int32_t)) {
        SPDLOG_CRITICAL("read msg len err");
        return;
    }
    buf_ptr += read_num;
    msg.len = ntohl(msg.len);
    if (msg.len > max_len) {
        SPDLOG_CRITICAL("msg len too long: %d", msg.len);
        return;
    }
    size_t body_size = msg.len - sizeof(msg.len);
    read_num = readn(fd_, buf_ptr, body_size, in_buffer_empty_);
    if (read_num == 0 && in_buffer_empty_) {
        return;
    }
    if (read_num != body_size) {
        // printf("read msg body err, read_num: %ld; body_len: %ld", read_num, body_size);
        SPDLOG_CRITICAL("read msg body err, read_num: %ld; body_len: %ld", read_num, body_size);
        return;
    }
    
    switch (msg.type)
    {
    case MsgType::NewRequest: {
        NewRequestMsg new_ctl_req_msg;
        memcpy(&new_ctl_req_msg, msg.data, getMsgBodySize(msg));
        new_ctl_req_handler_((void*)&new_ctl_req_msg, shared_from_this());
        
        break;
    }
    //不定长处理
    case MsgType::NewResponse:{
        size_t msg_size = getMsgBodySize(msg);
        NewResponseMsg* new_ctl_rsp_mag = (NewResponseMsg*)malloc(sizeof(NewResponseMsg) + msg_size);
        memset(new_ctl_rsp_mag->id, 0, msg_size);
        memcpy(new_ctl_rsp_mag, msg.data, msg_size);
        new_ctl_rsp_handler_((void*)new_ctl_rsp_mag, shared_from_this());
        free(new_ctl_rsp_mag);
        break;
    }
        
    case MsgType::NewTunnelReq:{
        NewTunnelReqMsg new_tunnel_req_msg;
        memcpy(&new_tunnel_req_msg, msg.data, getMsgBodySize(msg));
        new_tunnel_req_handler_((void*)&new_tunnel_req_msg, shared_from_this());
        break;
    }
    case MsgType::NewTunnelRsp:{
        NewTunnelRspMsg new_tunnel_rsp_msg;
        memcpy(&new_tunnel_rsp_msg, msg.data, getMsgBodySize(msg));
        new_tunnel_rsp_handler_((void *)&new_tunnel_rsp_msg, shared_from_this());
        break;
    }
    case MsgType::NotifyClient:{
        NotifyClientNeedProxyMsg Notify_client_msg;
        memcpy(&Notify_client_msg, msg.data, getMsgBodySize(msg));
        notify_client_need_proxy_handler_((void*)&Notify_client_msg, shared_from_this());
        break;
    }
        
    case MsgType::ShutdownPeerConn:{
        ShutdownPeerConnMsg req_msg;
        memcpy(&req_msg, msg.data, getMsgBodySize(msg));
        notify_proxy_shutdown_peer_conn_handler_((void *)&req_msg, shared_from_this());
        break;
    }
    case MsgType::FreeConnReq:{
        FreeProxyConnReqMsg req_msg;
        memcpy(&req_msg, msg.data, getMsgBodySize(msg));
        free_proxy_conn_req_handler_((void *)&req_msg, shared_from_this());
        break;
    }
    default:{
        break;
    }
        
    }

} catch (const std::exception &e) {
    std::cout << "read ctl_conn except: " << e.what() << std::endl;
    abort();
}


void CtlConnect::HandlerWrite(){
    if( out_buf_ ->getunused() <= 0) return;
    size_t sent_num = out_buf_->WriteToSocket(fd_);
    if(sent_num <= 0)
        SPDLOG_CRITICAL("sent to fd err");
    //未读完
    if(out_buf_->getunused() > 0)
        channel_->Addevents(EPOLLOUT);
}

void CtlConnect::HandlerEvery(){
    if(in_buffer_empty_ && channel_->IsClosed()){
        close_handler_(shared_from_this());
        return;
    }
    
    channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLRDHUP);
    if(out_buf_->getunused() > 0)
        channel_->Addevents(EPOLLOUT);

    loop_->ModToPoller(channel_);
}