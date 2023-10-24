#include <sys/types.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <random>
#include <iostream>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>

#include "Utils.h"

size_t readn(int fd, char* buf, size_t size, bool& bufferEmpty) {
    char* buf_ptr = buf;
    int len_left = size;
    while(len_left > 0) {
        int readNum = read(fd, buf_ptr, len_left);
        if(readNum < 0) {
            if(errno == EINTR || errno == EAGAIN)
                continue;
            else
                return -1;
        }
        else if (readNum == 0){
            bufferEmpty =  true;
            break;
        }
        len_left -= readNum;
        buf_ptr += readNum;
    }
    return size - len_left;
};

std::string random_str(int len) {
    char temp;
    std::string res;
    std::random_device rd;
    std::default_random_engine random(rd());

    for (int i = 0; i < len; i++) {
      //字母与数字
      temp = random() % 36;
      if (temp < 10) temp += '0';
      else temp = temp - 10 + 'A';
      res += temp;
    }
    return res;
}

int setNoblock(int fd) {
  int flag = fcntl(fd, F_GETFL, 0);
  if( flag == -1) return -1;
  flag |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flag) == -1) return -1;
  return 0;
}

int tcp_connect(const char* host, u_int32_t port){
    std::cout<<host<<" is conenct\t";
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    in_addr_t addr = inet_addr(host);
    //by域名
    if (addr == INADDR_NONE) {
        hostent*  entry = gethostbyname(host);
        if (!entry) return -1;
        memcpy(&addr, entry->h_addr, entry->h_length);
    }
    server_addr.sin_addr.s_addr = addr;
    server_addr.sin_port = htons(port);
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) return sock_fd;

    if(connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock_fd);
        return -1;
    }
    
    if (setNoblock(sock_fd)) {
      SPDLOG_CRITICAL("设置非阻塞失败: {}", strerror(errno));
    }
    return sock_fd;
}

// / 忽略sigpipe信号，当收到rst包后，继续往关闭的socket中写数据，会触发SIGPIPE信号, 该信号默认结束进程
void ignoreSigPipe() {
  struct sigaction sig;
  sigemptyset(&sig.sa_mask);
  sig.sa_handler = SIG_IGN;
  sig.sa_flags = 0;
  sigaction(SIGPIPE, &sig, NULL);
}

//192.169.0.1:8080 -> ip, port
void parse_host_port(char *addr, std::string &host, u_int32_t &port) {
  char addr_arr[100];
  strcpy(addr_arr, addr);
  char *splitPtr = strtok(addr_arr, ":");
  char *split_ret[2];
  for (int i = 0; i < 2; i++) {
    split_ret[i] = splitPtr;
    splitPtr = strtok(NULL, ":");
    if (splitPtr == nullptr) {
      break;
    }
  }
  host = split_ret[0];
  port = atoi(split_ret[1]);
};

//socker bind listen 
int sockt_bind_listen(int port) {
  if (port < 0 || port > 65535) {
    return -1;
  }
  int fd = 0;
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if ( fd < 0) {
    close(fd);
    return -1;
  }

  struct sockaddr_in server_addr;
  bzero((char*)&server_addr, sizeof(server_addr));
  // 只绑定port，ip监听后
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    close(fd);
    return -1;
  }

  if (listen(fd, 2048) < 0) {
    close(fd);
    return -1;
  }
  return fd;
}
