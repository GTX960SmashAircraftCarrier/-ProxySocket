#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include "../conf/Header.hpp"
#include "../conf/ThreadClient.hpp"
#ifndef EASYCLIETN_HPP
#define  EASYCLIETN_HPP
class EasyClient{
public:
    EasyClient(){
        clientSocket = -1;
        _recvcount = 0;
        isRun = false;
    }
    virtual ~EasyClient(){
        Close();
    }
    int InitSocket(){  
        if(clientSocket < 0)
            clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(clientSocket < 0){
            std::cerr<<"Error ceating client socket"<<std::endl;
            return -1;
        }
        client.SetSocket(clientSocket);
        std::cout<<"socket has been created "<<std::endl;
        return 1;
    }
    
    int Connect(const char *ip, unsigned short port){
        if(clientSocket < 0)
            InitSocket();
        if(clientSocket < 0) std::cout<<"please InitSocket first"<<std::endl;
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr(ip);
        serverAddress.sin_port = htons(port);
        int connect_state = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
        if(connect_state < 0){
            Close();
            std::cerr<<"Error connecting server socket"<<std::endl;
            return -1;
        }else{
            std::cout<<"conect with server: "<<std::endl;
            isRun = true;
        }
        return connect_state;
    }
    
    void Close(){
        if(clientSocket > 0)
            close(clientSocket);        
        isRun = false;
        clientSocket = -1;
    }

    void Handel(){
        if(isRun){
            fd_set fdRead, fdWrite;
            FD_ZERO(&fdRead);
            FD_ZERO(&fdWrite);
            FD_SET(clientSocket, &fdRead);
            FD_SET(clientSocket, &fdWrite);
            timeval t = {0, 10};
            int ret = select(clientSocket + 1, &fdRead, &fdWrite, nullptr, &t);
            // memcpy(&fdWrite, &fdRead, sizeof(fd_set));
            if(ret < 0){
                std::cout<<"select is over"<<std::endl;
                return;
            }
            ReadData(fdRead);
            WriteData(fdWrite);
            
        }
    }

    void WriteData(fd_set& fdWrite){
        if(FD_ISSET(clientSocket, &fdWrite)){
            FD_CLR(clientSocket, &fdWrite);
            if(-1 == client.SendDataReal()){
                Close();
                std::cout<<"select done"<<std::endl;
            }
        }
    }

    void ReadData(fd_set& fdWrite){
        if(FD_ISSET(clientSocket, &fdWrite)){
            FD_CLR(clientSocket, &fdWrite);
            if(-1 == RecvData()){
                Close();
                std::cout<<"select done"<<std::endl;
            }
        }
    }

    int RecvData(){
       int len = client.RecvData();
        if(len <=0)
            return -1;
        while(!client.empty()){
            Message(client.front());
            client.pop_front();
        }
        return 0;
    }

    int Message(DataHeader* header){
        ++_recvcount;
        switch(header->cmd){
            case CMD_LOGIN_RESULT:{
                ClientLoginResult *login_ret = (ClientLoginResult*)header;
                // std::cout<<(login_ret->result ? "login success" : "user or pwd is incorrect")<<std::endl;
            } break;
            case CMD_LOGOUT_RESULT:{
                ClientLogoutResult *LogoutRet = (ClientLogoutResult*)header;
                // std::cout<<"receive data, toal: "<<_recvcount<<std::endl;
                // std::cout<<(LogoutRet->result ? "logout success" : "can not exit")<<std::endl;
            } break;
            case CMD_EXPIRE:{
                std::cout<<"receive serever not expire\n";
            } break;
            default:{
                std::cout<<"unknowed cmd"<<std::endl;
            } break;
        
        }
        return 1;
    }
    
    int SendData(const std::shared_ptr<DataHeader>& header){
        return client.SendData(header);
    }

private:
    unsigned int _recvcount;
    int clientSocket;
    ThreadClient client;
    bool isRun;
};

#endif