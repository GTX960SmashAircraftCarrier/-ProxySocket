#ifndef TRANS_CONNECT_HPP
#define TRANS_CONNECT_HPP

#include <unistd.h>

#include "Connect.h"
#include "Event_loop.h"
#include "Thread_Event_loop.h"

class TransConnect : public Connect {
public:
    TransConnect(int fd, SP_ThreadEventloop thread) : Connect{fd, thread->GetLoop()}, thread_(thread), tran_count_(0), recv_count_(0), total_count_(-1) {
        pipe(pipe_fd_);
        std::cout<<"TransConnect init success\n";
    }

    ~TransConnect(){
        close(pipe_fd_[0]);
        close(pipe_fd_[1]);
    }

    void setConn2fd (int fd) { conn2fd_ = fd; }

    SP_ThreadEventloop getThread() { return thread_; }

    int getRecvCount() { return recv_count_; }
    void AddRecvCount(int count) {recv_count_ += count; }
    void resetRecvCount() { recv_count_ = 0; }

    int getTranCount() { return tran_count_; }
    void AddTranCount(int count) { tran_count_ += count; }
    void resetTranCount() { tran_count_ = 0; }

    int getTotalCount() { return total_count_; }
    void AddTotalCount(int count) { 
        if(total_count_ == -1) total_count_ = count;
        else total_count_ += count; 
    }
    void resettotalCount() { total_count_ = 0; }

public:
    int pipe_fd_[2];
    int conn2fd_;
    SP_ThreadEventloop thread_;
    std::atomic<int> tran_count_;
    std::atomic<int> recv_count_;
    std::atomic<int> total_count_;
};

using SP_TransConnect = std::shared_ptr<TransConnect>;
#endif