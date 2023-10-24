#include <unistd.h>
#include <string.h>
#include <stdexcept>
#include <exception>
#include <iostream>

#include "spdlog/spdlog.h"
#include "Buffer.h"

const int DOUBLE_BORDER = 1024;

size_t Buffer::read(char* data, int len){
    if(len <= 0) return 0;
    if(len > getunused()) return -1;
    int cur_pos = read_pos_;
    if(write_pos_ > read_pos_){
        memcpy(data, data_ + read_pos_, len);
        read_pos_ += len;
    }
    else{
        int last = datasize_ - read_pos_;
        if(last > len){
            memcpy(data, data_ + read_pos_, len);
            read_pos_ += len;
        }
        else{
            memcpy(data, data_ + read_pos_, last);
            memcpy(data + last, data_, len - last);
            read_pos_ = len - last;
        }
    }
    return len;
}

size_t Buffer::WriteToBuffer(char* data, int len) try{
    if(len <= 0) return 0;
    MustInsert(len);
    int cur_pos = write_pos_;

    for (int i = 0; i < len; i++) {
        cur_pos = (write_pos_ + i) % datasize_;
        memcpy(data_ + cur_pos, data + i, 1);
    }

    write_pos_ = (cur_pos + 1) % datasize_;
    return len;
} catch (std::exception &e) {
    SPDLOG_CRITICAL("write to buffer except: {}", e.what());
    return 0;
}

size_t Buffer::WriteToSocket(int fd){
    int total_sent{0};
    while(getunused() > 0){
        int last = write_pos_ - read_pos_;
        if(read_pos_ > write_pos_){
            last = datasize_ - read_pos_;
        }
        size_t write_ret = write(fd, data_ + read_pos_, last);
        if (write_ret == 0){
            if (errno == EINTR) continue;
            else break;
        }
        read_pos_ = (read_pos_ + write_pos_) % datasize_;
        total_sent += write_ret;
    }
    return total_sent;
}

int Buffer::getunused(){
    if(write_pos_ > read_pos_){
        return write_pos_ - read_pos_;
    }
    // |<---->|------|<------>|
    // |------w------r--------|
    return datasize_ - read_pos_ + write_pos_;
}

int Buffer::GetFreeSize(){
    return capacity_ - getunused();
}

void Buffer::resize(int capacity){
    int unused_size = getunused();
    // 不缩小
    if(unused_size > capacity || capacity == capacity_) return;

    char *temp = (char*)malloc(capacity + 1);
    if(write_pos_ > read_pos_)
        memcpy(temp, data_ + read_pos_, write_pos_ - read_pos_);
    else{
        int last = datasize_ - read_pos_;
        memcpy(temp, data_ + read_pos_, last);
        memcpy(temp + last, data_, write_pos_);
    }
    /* 
    for(int i = 0; i < unused_size; i++){
        int cur_index = (read_pos_ + i) % datasize_;
        memcpy(temp + i, data_ + cur_index + i, 1);
    }
    */
    free(data_);
    data_ = temp;
    datasize_ = capacity + 1;
    capacity_ = capacity;
    read_pos_ = 0;
    write_pos_ = unused_size;
}

void Buffer::MustInsert(int len){
    if(getunused() >= len) return;
    int min_span = capacity_ + len;
    if(min_span > maxcapacity_){
        throw std::runtime_error("Buffer out of capicity\n");
    }
    int new_span = capacity_;
    int double_span = capacity_ * 2;
    if(min_span > double_span){
        new_span = min_span;
    }
    else{
        if(capacity_ < DOUBLE_BORDER){
            new_span = double_span <= maxcapacity_ ? double_span : maxcapacity_;
        }
        else{
            while (new_span < min_span){
                new_span += new_span / 4;
                if(new_span > maxcapacity_){
                    new_span = maxcapacity_;
                    break;
                }
            }
        }
    }
    resize(new_span);
    return;
}