#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>

#include "Client_connect.h"
#include "Tunnel.h"
#include "lib/Utils.h"

void ClientConnect::handleRead() {
    try{
        int zero_count = splice(fd_, NULL, pipe_fd_[1], NULL, 2048, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
        if (zero_count < 0) {
            SPDLOG_CRITICAL("proxy_id: {} local_fd: {} -> pipe_fd: {} splice err: {} ", proxy_id_, fd_,pipe_fd_[1], strerror(errno));
            return;
        }

        if (zero_count == 0 && !close_) {
            std::cout<<"?????\n";
            tunnel_->shutdownFromClient(proxy_id_,  getTranCount());
            close_ = true;
            return;
        }

        zero_count = splice(pipe_fd_[0], NULL, conn2fd_, NULL, zero_count, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
        if(zero_count < 0) {
            SPDLOG_CRITICAL("proxy_id {} local_fd: {} pipe_fd: {} -> proxy_conn_fd: {} splice err: {}", proxy_id_, fd_, pipe_fd_[0], conn2fd_, strerror(errno));
            return;
        }
        AddTranCount(zero_count);
    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("read local_conn except: {}", e.what());
    }   
}

void ClientConnect::everyHandle() {
    if (close_){
        return;
    }
    channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLRDHUP);
    loop_->ModToPoller(channel_);
}