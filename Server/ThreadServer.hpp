#ifndef _THREAD_SERVER_HPP_
#define _THREAD_SERVER_HPP_
#include "Server.hpp"
#include "NetEvent.hpp"
#include <list>
#include <map>
#include <memory>

class ThreadServer{
public:
    ThreadServer(int id, int socket = -1){
        _socket = socket;
        _pthreadId = id;
        _pNetEvent = nullptr;
        clientchange = true;
        // _pthread = nullptr;
        lasttime =  Time::time();
        std::cout<<"threadserver si cerat: "<<id<<std::endl;
    }
    ~ThreadServer(){
        Close();
        _socket = -1;   
    }
    
    void Close(){
        printf("CELLServer%d.Close begin\n", _pthreadId);
        _isRun = false;
        taskServer.Close();
        _thread.Close();
        printf("CELLServer%d.Close begin\n", _pthreadId);
        // for(auto it= Clients.begin(); it != Clients.end(); it++){
        //     close(it->first);
        //     std::cout<<"Delte PtherClient\n";
        //     // delete it->second;
        // }
        // for(auto it= ClientsBuf.begin(); it != ClientsBuf.end(); it++){
        //     close((*it)->GetSocketId());
        // }
        // Clients.clear();
        // ClientsBuf.clear();
    }

    void SetEvent(NetEvent *event){
        _pNetEvent = event;
    }

    void Start(){
        taskServer.Start();
        _thread.Start(nullptr, [this](ThreadControl* pthread){ Run(pthread); }, [this](ThreadControl* pthread){ ClearAllCLient(); });

        // if(1){
        //     // _pthread = new std::thread(std::mem_fn(&ThreadServer::Run), this);
        //     std::thread t = std::thread(std::mem_fn(&ThreadServer::Run), this);
        //     t.detach();
        //     taskServer.Start();
        //     _isRun = true;
        // }
    }

    bool Run(ThreadControl* pThread){
        std::cout<<"main threadserving is runing\n";
        while(pThread->IsRun()){
            if (!ClientsBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : ClientsBuf)
				{
					Clients[pClient->GetSocketId()] = pClient;
				}
				ClientsBuf.clear();
                clientchange = true;
			}

			if (Clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
             
            fd_set fdRead, fdWrite, fdErro;
            if(clientchange){
                clientchange = false;
                FD_ZERO(&fdRead);
                maxSock = Clients.begin()->second->GetSocketId();
                for(auto it= Clients.begin(); it != Clients.end(); it++){
                    FD_SET(it->second->GetSocketId(), &fdRead);
                    maxSock = it->second->GetSocketId() > maxSock ? it->second->GetSocketId():maxSock;
                }
                memcpy(&fdRead_back, &fdRead, sizeof(fd_set));
            }
            else{
                memcpy(&fdRead, &fdRead_back, sizeof(fd_set));
            }
            memcpy(&fdWrite, &fdRead_back, sizeof(fd_set));
            // memcpy(&fdErro, &fdRead_back, sizeof(fd_set));

            timeval t{0, 10};
            int ret = select(maxSock + 1, &fdRead, &fdWrite, nullptr, &t);
            if(ret < 0){
                std::cout<<"seletc is done"<<std::endl;
               _thread.Exit();
                return false;
            }
            ReadData(fdRead);
            WiriteData(fdWrite);
            checkTime();
            // ErroData(fdErro);
        }
        return 1;
    }
    
    void checkTime(){
        // std::cout<<"called\n";
       time_t curtime =  Time::time();
       time_t passby = curtime - lasttime;
       lasttime = curtime;
        for(auto it= Clients.begin(); it != Clients.end(); it++){
            //dead check 
            if(it->second->checkexpire(passby)){
                if(_pNetEvent)
                    _pNetEvent->OnNetLeave(it->second);
                clientchange = true;
                std::cout<<"long time no use close\n";
                close(it->first);
                it = Clients.erase(it);
                if(it == Clients.end()) break;
                it--;
            }
            //send check
            // it->second->checksendtime(passby);
        }
    }

    void WiriteData(fd_set& fdWrite){
        for(auto it= Clients.begin(); it != Clients.end(); it++){
            if(FD_ISSET(it->second->GetSocketId(), &fdWrite)){
                if (-1 == it->second->SendDataReal()){
                    if(_pNetEvent)
                        _pNetEvent->OnNetLeave(it->second);
                    clientchange = true;
                    // std::cout<<"all done, close: "<<it->second->GetSocketId()<<std::endl;
                    // temp.push_back(it->second);
                    //  list vector  can delete like this ,   map can not !!!!
                    std::cout<<"write close: "<<it->second->GetSocketId()<<std::endl;
                    it = Clients.erase(it);
                    if(it == Clients.end()) break;
                    it--;
                }
            }
                        
        }
    }

    void ReadData(fd_set& fdRead){
        // std::vector<ThreadClientptr> temp;
            for(auto it= Clients.begin(); it != Clients.end(); it++){
                if(FD_ISSET(it->second->GetSocketId(), &fdRead)){
                    if (-1 == RecvData(it->second)){
                        if(_pNetEvent)
                            _pNetEvent->OnNetLeave(it->second);
                        clientchange = true;
                        close(it->first);
                        // temp.push_back(it->second);
                        //  list vector  can delete like this ,   map can not !!!!
                        std::cout<<"read close: "<<it->second->GetSocketId()<<std::endl;
                        it = Clients.erase(it);
                        if(it == Clients.end()) break;
                        it--;
                    }
                }
                
        }
        // for(auto pclient: temp){
        //     Clients.erase(pclient->GetSocketId());
        //     // while(tas kServer.AllDone())
        //     //     ;
        //     close(pclient->GetSocketId());
        //     std::cout<<"Delte ThreadClient\n";
        //     // delete pclient;
        // }
    }

    int RecvData(const ThreadClientptr& client){

        int len = client->RecvData();
        if(len <=0)
            return -1;
        _pNetEvent->OnNetRecv(client);

        while(!client->empty()){
            OnNetMsg(client, client->front());
            client->pop_front();
        }
        return 0;
    }
 
    virtual void OnNetMsg(const ThreadClientptr& pclient, DataHeader *header){
        // std::cout<<"ThreadServer "<<_pthreadId<< " OnNetMsg is called\n";
        _pNetEvent->OnNetMsg(this, pclient, header);
    }

    void addClient(const ThreadClientptr& pclient){
        std::lock_guard<std::mutex> lock(_mutex);
        ClientsBuf.push_back(pclient);
        std::cout<<"toatl client:"<<getTotalClient()<<std::endl;
    }

    size_t getTotalClient(){    
        return Clients.size() + ClientsBuf.size();
    }

    void addSendTask(const ThreadClientptr& pclient, const std::shared_ptr<DataHeader> &header){
        std::shared_ptr<Send2Client> task = std::make_shared<Send2Client>(pclient, header);
        // Send2Client* task = new  Send2Client(pclient, header);
        taskServer.addTask(task);
    }

    int getTheadServerId(){
        return _pthreadId;
    }

private:
    void ClearAllCLient(){
        //auto ptr,  only clear
        Clients.clear();
        ClientsBuf.clear();
    }

private:
    std::map<int, ThreadClientptr> Clients;
    std::list<ThreadClientptr> ClientsBuf;
    // std::thread* _pthread;
    ThreadTaskServer taskServer;
    ThreadControl _thread;
    NetEvent* _pNetEvent;
    std::mutex _mutex;
    fd_set fdRead_back;
    int maxSock;
    time_t lasttime;
    int _pthreadId;
    int _socket;
    bool clientchange;
    bool _isRun;
};

#endif