#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/types.h>
#include <mutex>
#include <unordered_map>

#include "Tunnel.h"
#include "lib/Channel.h"
#include "lib/CtlConnect.h"
#include "lib/Event_loop.h"
#include "lib/Thread_EventLoop_pool.h"
#include "lib/ProxyConnect.h"
#include "lib/Utils.h"

class Client : public std::enable_shared_from_this<Client> {
public:
    // workthreadNum : 初始化线程池数量
    // server_host : 外网服务器Host，可以为域名
    // local_server_host : 本地服务
    Client(int workThreadNum, std::string server_host, u_int32_t server_port, std::string local_server_host, u_int32_t  local_server_port)
        : proxy_server_host_(server_host),
        proxy_server_port_(server_port),
        server_host_(local_server_host),
        server_port_(local_server_port),
        loop_(new EventLoop()),
        thead_event_pool_(new ThreadEventLoopPoll(workThreadNum)){
            ignoreSigPipe();
            std::cout<<"Client init success\n";
        }
    
    void Run();

    std::string getProxyServerHost(){
        return proxy_server_host_;
    }

    SP_CtlConnect getConnect() { return ctlconnect_; }

    void shutdownLocal(std::string tun_id, std::string proxy_id, uint32_t trans_count);

    
private:
    friend class Tunnel;
    void initConnect();
    //第一次建立请求
    void NewCtlReq();
    
    void NewTunnelReq();


    void handleNewCtlRsp(void* msg, SP_CtlConnect conn);
    
    
    void handleNewTunnelRsp(void* msg, SP_CtlConnect conn);
    
    void handleCtlConnClose(SP_CtlConnect conn) { abort(); }
    
    void handleProxyNotify(void*msg, SP_CtlConnect);
    void handleShutdownLocalConn(void* msg, SP_CtlConnect);
    

private:
    std::string proxy_server_host_;
    u_int32_t proxy_server_port_;
    std::string server_host_;
    u_int32_t server_port_;
    std::string client_id_;
    std::unordered_map<std::string, SP_Tunnel> tunnel_map_;
    
    SP_CtlConnect ctlconnect_;
    SP_EventLoop loop_;
    SP_ThreadEventLoopPoll thead_event_pool_;
};
#endif