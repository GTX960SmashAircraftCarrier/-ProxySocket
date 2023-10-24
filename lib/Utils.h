#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/types.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

size_t readn(int fd, char* buf, size_t size, bool& bufferEmpty);

std::string random_str(int len);

int setNoblock(int fd);

int tcp_connect(const char* host, u_int32_t port);

// / 忽略sigpipe信号，当收到rst包后，继续往关闭的socket中写数据，会触发SIGPIPE信号, 该信号默认结束进程
void ignoreSigPipe();

//192.169.0.1:8080 -> ip, port
void parse_host_port(char *addr, std::string &host, u_int32_t &port);

//socker bind listen 
int sockt_bind_listen(int port);



template <typename keyT, typename valueT>
struct threadsafe_unordered_map {
    std::mutex mutex_;
    std::unordered_map<keyT, valueT> map;
    bool isExist(keyT key) {
      std::unique_lock<std::mutex> lock(mutex_);
      if (map.find(key) == map.end()) {
        return false;
      } else {
        return true;
      }
    }

    void erase(keyT key) {
      std::unique_lock<std::mutex> lock(mutex_);
      map.erase(key);
    }

    valueT get(keyT key, bool& isExist) {
      std::unique_lock<std::mutex> lock(mutex_);
      if (map.find(key) == map.end()) {
        isExist = false;
        return valueT{};
      } else {
        isExist = true;
      }
      return map[key];
    }

    valueT get(keyT key) {
      std::unique_lock<std::mutex> lock(mutex_);
      return map[key];
    }

    void add(keyT key, valueT val) {
      std::unique_lock<std::mutex> lock(mutex_);
      map.emplace(key, val);
    }

    bool empty() { return map.empty(); }
};

#endif