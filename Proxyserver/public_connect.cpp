#include "public_connect.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "Tunnel.h"
#include "spdlog/spdlog.h"

void PublicConnect::handleRead() try{
    int bs = splice(fd_, NULL, pipe_fd_[1], NULL, 2048, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
    if (bs < 0 ) {
        SPDLOG_CRITICAL("public_fd: {} -> pipe_fd: {} splice err: {}", fd_, pipe_fd_[1], strerror(errno));
        return;
    }

    if (bs == 0 && !closed_) {
        // 收到fin包,通知proxy
        tunnel_->shutdownFromPublic(proxy_id_, getTranCount());
        closed_ = true;
        return;
    }
    bs = splice(pipe_fd_[0], NULL, conn2fd_, NULL, bs, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);

    if (bs < 0 ) {
        SPDLOG_CRITICAL("proxy_id {} pipe {} - >proxy_fd: {} splice err: {}", proxy_id_, pipe_fd_[0], conn2fd_, strerror(errno));
        return;
    }

    AddTranCount(bs);

} catch (const std::exception& e) {
    SPDLOG_CRITICAL("read public except: {}", e.what());
}
void PublicConnect::everyHandle() {
    if (closed_) {
        return;
    }
    channel_->SetEvents(EPOLLET | EPOLLIN | EPOLLRDHUP);
    loop_->ModToPoller(channel_);
}