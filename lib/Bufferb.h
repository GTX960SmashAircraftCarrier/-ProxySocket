#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <iostream>
#include <memory>


// 循环缓存
// capacity  初始化缓存大小
// maxcapacity 最大缓存大小
class Buffer {
public:
    // datasize = capacity + 1, 便于判断write尾部
    Buffer(int capacity = 8, int maxcapacity = 1024)
        : capacity_(capacity), datasize_(capacity + 1), maxcapacity_(maxcapacity > capacity ? maxcapacity : capacity) , data_((char*)malloc(datasize_)){
            std::cout<<"Buffer init success\n";
        }
    ~Buffer(){ free(data_); }

    //-1 剩余空间不够，0其他错误
    size_t read(char* data, int len);
    
    size_t WriteToBuffer(char* data, int len);
    
    size_t WriteToSocket(int fd);
    
    int getunused();

    int GetFreeSize();
private:
     // 扩容参考golang切片扩容机制
    void resize(int capacity);
   
    void MustInsert(int len);

private:
    int capacity_;
    int datasize_;
    int maxcapacity_;
    char *data_;
    int read_pos_{0};
    int write_pos_{0};
};

using SP_Buffer = std::shared_ptr<Buffer>;

#endif