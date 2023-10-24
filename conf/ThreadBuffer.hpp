#ifndef _Thread_BUFFER_HPP_
#define _Thread_BUFFER_HPP_
#include <cstring>
#include <sys/socket.h>
#include "Header.hpp"
class ThreadBuffer{
public:
    ThreadBuffer(int size = 4096){
        _size = size;
        _Buff = new char[size];
    }

    ~ThreadBuffer(){
        if(_Buff){
            delete[] _Buff;
            _Buff = nullptr;
        }
    }

    char* Data(){
        return _Buff;
    }

    bool push(const char* data, int len){
        if(_last + len <= _size){
            memcpy(_Buff + _last, data, len);
            _last += len;
            if(_last == _size)
                ++_full;

            return true;
        }
        else{
            ++_full;
        }
        return false;
    }

    void pop(int len){
        int n = _last - len;
        if(n > 0){
            memcpy(_Buff, _Buff + len, n);
        }
        _last = n;
        if(_full > 0)
            --_full;   
    }

    int write2socket(int sockt){
        int ret = 0;
        if(_last > 0 && sockt > 0){
            ret = send(sockt, _Buff, _last, 0);
            _last = 0;
            _full = 0;
        }
        return ret;
    }

    int read4socket(int socket){
        if(_size - _last > 0){
            char* rv = _Buff + _last;
            int len = (int)recv(socket, rv, _size - _last, 0);
            if(len <=0 )
                return len;
             _last += len;
            return len;
        }
        return 0;
    }

    bool empty(){
        if(_last >= sizeof(DataHeader)){
            DataHeader* header = (DataHeader*) _Buff;
            return _last < header->len;
        }
        return true;
    }
private:
    char* _Buff = nullptr;
    int _last = 0;
    int _size = 0;
    int _full = 0;
};

#endif