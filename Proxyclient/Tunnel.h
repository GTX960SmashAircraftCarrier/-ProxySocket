#ifndef TUNNEL_HPP
#define TUNNEL_HPP
#include <memory>

#include "Client_connect.h"
#include "lib/Connect.h"
#include "lib/Thread_EventLoop_pool.h"
#include "lib/ProxyConnect.h"
#include "lib/Utils.h"

class Client;

class Tunnel {
public:
    Tunnel(std::string id, std::string sh, u_int32_t sp, std::string psh, u_int32_t psp, SP_ThreadEventLoopPoll workthreadpool, Client* client)
    :   tunnel_id_(id),
        server_host_(sh),
        server_port_(sp),
        proxy_server_host_(psh),
        proxy_server_port_(psp),
        work_pool_(workthreadpool),
        client_(client) {}
    
    std::string getValidProxyID();
    
    void shutdownFromClient(std::string proxy_id, u_int32_t tran_count);
    void shutdonwLocalConn(SP_ProxyConnect);
    
    SP_ProxyConnect createProxyConn(u_int32_t proxy_port);
    SP_ClientConnect createLocalConn(SP_ProxyConnect);
public:
    int client_fd_start;
    int client_fd_closed_;
    //维护已近建立好的proxy连接，复用
    threadsafe_unordered_map<std::string , SP_ProxyConnect> proxy_connect_map;

private:
    void HandleProxyConnectReq(void*, SP_ProxyConnect);
private:
    std::string tunnel_id_;
    std::string server_host_;
    u_int32_t server_port_;
    std::string proxy_server_host_;
    u_int32_t proxy_server_port_;
    SP_ThreadEventLoopPoll work_pool_;
    Client* client_;
};

using SP_Tunnel = std::shared_ptr<Tunnel>;
#endif


