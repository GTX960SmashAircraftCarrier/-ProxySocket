#ifndef ThreadClient_HPP
#define ThreadClient_HPP
#include <cstring>
#include "TimeStamp.hpp"
#include "ThreadBuffer.hpp"
#ifndef MaxBufSize
#define MaxBufSize  81920
#endif
//客户端不操作断开限制
#define ExpireTime 50000
//服务器最大发送延时,  Nagle 算法
#define SendDeadTime 200
class ThreadClient{
public:
    ThreadClient():SendBuf(MaxBufSize), RecvBuf(MaxBufSize){
        socket = -1;
        _deadtime = 0;
        _senddeadtime = 0;
    }

    ThreadClient(int clientsocket):SendBuf(MaxBufSize), RecvBuf(MaxBufSize){
        socket = clientsocket;
        _deadtime = 0;
        _senddeadtime = 0;
    }

    ~ThreadClient(){
        std::cout<<"ThreadClient close\n";
    }
    
    void SetSocket(int clientsocket){
        socket = clientsocket;
    }
    
    int GetSocketId(){
        return socket;
    }
    
    int RecvData(){
        return RecvBuf.read4socket(socket);
    }

    bool empty(){
        return RecvBuf.empty();
    }

    DataHeader* front(){
        return (DataHeader*) RecvBuf.Data();
    }

    void pop_front(){
        if(!empty()) return RecvBuf.pop(front()->len);
    }

    int SendDataReal(){
        Resetsendtime();
        return SendBuf.write2socket(socket);
    }

    int SendData(const std::shared_ptr<DataHeader>& header){
    // int SendData(DataHeader* header){
        if(SendBuf.push((const char*)header.get(), header->len)){
            return header->len;
        }
        return 0;
        // while(true){
        //     // 数据聚集的 Nagle 算法
        //     if(LastSendPos + nSendLen >= MaxBufSize){
        //         int Copylen = MaxBufSize - LastSendPos;
        //         memcpy(SendBuf + LastSendPos, pSendData, Copylen);
        //         pSendData += Copylen;
        //         nSendLen -= Copylen;
        //         int ret = send(socket, SendBuf, MaxBufSize, 0);
        //         LastSendPos = 0;
        //         Resetsendtime();
        //         if(ret < 0){
        //             return -1;
        //         }
        //     }
        //     else{
        //         memcpy(SendBuf + LastSendPos, pSendData, nSendLen);
        //         LastSendPos += nSendLen;
        //         break;
        //     }
        // }
    }

    void Resettime(){
        _deadtime = 0;
    }

    bool checkexpire(time_t time){
        _deadtime += time;
        if(_deadtime >= ExpireTime){
            // Resettime();
            return true;
        }
        return false;
    }

    void Resetsendtime(){
        _senddeadtime = 0;
    }

    bool checksendtime(time_t time){
        _senddeadtime += time;
        if(_senddeadtime >= SendDeadTime){
            SendDataReal();
            Resetsendtime();
            return true;
        }
        return false;
    }
private:
    int socket;
    ThreadBuffer RecvBuf;
    ThreadBuffer SendBuf;
    //expire time
    time_t _deadtime;
    //send afert time
    time_t _senddeadtime;

};
#endif