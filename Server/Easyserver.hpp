
#include <list>
#include "Server.hpp"
#include "ThreadServer.hpp"
#include "ThreadTask.hpp"
#include "NetEvent.hpp"
class EasyServer : public NetEvent{
public:
    EasyServer(){
        ServerSocket = -1;
        MesCount = 0;
        RecvCount = 0;
        ClietnCount = 0;
    }
    
    virtual ~EasyServer(){
        std::cout<<"all clean\n";
    }
    
    int InitSocket(){
        std::cout<<"Server Initializing...  pleast wait"<<std::endl;
        ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(ServerSocket < 0){
            std::cerr<<"Error ceating server socket!"<<std::endl;
            return -1;
        }else{
            std::cout<<"creating network success!"<<std::endl; 
        }
        return 1;
    }
    
    int Bindserver(int port){
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        if(bind(ServerSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
            Close();
            std::cerr<<"Error binding server socket"<<std::endl;
            return -1;
        }else{
            std::cout<<"Binding network success!"<<std::endl; 
        }
        return 1;
    }
    
    int Listen(){
        if(listen(ServerSocket, 1024) < 0){
        std::cerr<<"Error listening server socket"<<std::endl;
            Close();
            return -1;
        }else{
            std::cout<<"Listening...."<<std::endl; 
        }
        return 1;
    }

    void InitALL(int port){
        InitSocket();
        Bindserver(port);
        Listen();
        Start(_ThreadServer_COUNT);
    }

    void Close(){
        printf("EasyTcpServer.Close begin\n");
        _thread.Close();
        if(ServerSocket > 0){
            for(auto it= _ptherdServer.begin(); it != _ptherdServer.end(); it++){
                std::cout<<"Delte PtherServer\n";
                delete *it;
            }
            close(ServerSocket);
            ServerSocket = -1;
            _ptherdServer.clear();
        }
        printf("EasyTcpServer.Close end\n");
    }

    void Start(int n){
        for(int i = 0; i < n; i++){
            auto pthreadserver = new ThreadServer(i, ServerSocket);
			_ptherdServer.push_back(pthreadserver);
            pthreadserver->SetEvent(this);
			pthreadserver->Start();
        }
        _thread.Start(nullptr, [this](ThreadControl* pThread) { Run(pThread);});
    }

    int Accept(){
        sockaddr_in clientAddress;
        socklen_t clientAddresslen = sizeof(clientAddress);
        int clientSocket = accept(ServerSocket, (struct sockaddr*)&clientAddress, &clientAddresslen);
        if(clientSocket < 0){
            std::cerr<<"Error accepting server socket"<<std::endl;
        }
        else{
            std::cout<<"new hreadClient\n";
            ThreadClientptr Client = ClientPool.alloc();
            Client->SetSocket(clientSocket);
            addclient2pThreadServer(Client);
            std::cout<<"new client: "<<inet_ntoa(clientAddress.sin_addr)<<"socketID: "<<clientSocket<<std::endl;
        }
        return clientSocket;
    }

    void addclient2pThreadServer(const ThreadClientptr &client){
        // _clients.push_back(client);
        ThreadServer *temp = nullptr;
        int MaxClient = 1025;
        for(auto it : _ptherdServer){
            if(it->getTotalClient() < MaxClient){
                temp = it;
                MaxClient = it->getTotalClient();
            }
        }
        temp->addClient(client);
        std::cout<<"add new cliet to "<<temp->getTheadServerId()<<std::endl;
        OnNetJoin(client);
    }
   
    int Run(ThreadControl* pThread){
        std::cout<<"main serving is runing\n";
        while(pThread->IsRun()){
            Throughput();
            fd_set fdRead;
            FD_ZERO(&fdRead);
            FD_SET(ServerSocket, &fdRead);
            timeval t = { 0, 50};
            int ret = select(ServerSocket + 1, &fdRead, 0, 0, &t);
            if (ret < 0)
			{
                std::cout<<"select is done \n";
				_thread.Exit();
				return -1;
			}

            if (FD_ISSET(ServerSocket, &fdRead))
			{
				FD_CLR(ServerSocket, &fdRead);
				Accept();
			}
        }
        return -1;
    }
   
    void Throughput(){
        auto t1 = Time.getElapseSecond();
        if( t1 >= 1.0){
            printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recv<%d>,msg<%d>\n", (int)_ptherdServer.size(), t1, ServerSocket,(int)ClietnCount, (int)(RecvCount/ t1), (int)(MesCount / t1));
            RecvCount = 0;
            MesCount = 0;
            Time.update();
        }
    }
    
    virtual void OnNetJoin(const ThreadClientptr& pclient){
        // std::cout<<"EasyServer OnNetJoin is called\n";
        ++ClietnCount;
    }
    virtual void OnNetLeave(const ThreadClientptr& pClient){
        // std::cout<<"EasyServer OnNetleave is called\n";
        
		--ClietnCount;
	}
    virtual void OnNetMsg(ThreadServer* pCellServer, const ThreadClientptr& pClient, DataHeader* header){
        // std::cout<<"EasyServer OnNetMsg is called\n\n";
		++MesCount;
	}
	virtual void OnNetRecv(const ThreadClientptr& pClient){
        // std::cout<<"EasyServer OnNetRecv is called\n";
		++RecvCount;
	}

private:
    int ServerSocket;
    std::list<ThreadServer*> _ptherdServer;
    ObjectPool<ThreadClient, max_obj_nmber> ClientPool;
    ThreadControl _thread;
    TimeStamp Time;
protected:
    std::atomic<unsigned int> RecvCount;
    std::atomic<unsigned int> MesCount;
    std::atomic<unsigned int> ClietnCount;
};