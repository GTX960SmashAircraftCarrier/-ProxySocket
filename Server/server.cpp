// #include "../Memory/Alloctor.h"
#include "Easyserver.hpp"
#include <csignal>


class Server: public EasyServer{
    virtual void OnNetJoin(const ThreadClientptr& pClient)
	{
		EasyServer::OnNetJoin(pClient);
		
	}
    virtual void OnNetleave(const ThreadClientptr& pClient)
	{
		EasyServer::OnNetLeave(pClient);
	}

    virtual void OnNetMsg(ThreadServer* pthreadServer, const ThreadClientptr& pClient, DataHeader* header)
	{
		// std::cout<<"recvem but not send\n";
		EasyServer::OnNetMsg(pthreadServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->Resettime();

			ClientLogin* login = (ClientLogin*)header;

			std::shared_ptr<ClientLoginResult> ret =   std::make_shared<ClientLoginResult>();

			if(pClient->SendData(ret) == 0){
				std::cout<<"full sendbuf\n";
			}

			// pthreadServer->addSendTask(pClient, ret);
		}//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
		break;
		case CMD_LOGOUT:
		{
			ClientLogout* logout = (ClientLogout*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//netmsg_LogoutR ret;
			//SendData(cSock, &ret);
		}
		break;
		case CMD_EXPIRE:{
			pClient->Resettime();
			std::shared_ptr<Expire> ret =  std::make_shared<Expire>();
			pClient->SendData(ret);

			// pthreadServer->addSendTask(pClient, ret);
		}
		break;
		default:
		{
            std::cout<<"???"<<std::endl;
		}
		break;
		}
	}
};

static volatile int keepRunning = 1;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        keepRunning = 0;
    }
}


int main(){
    signal(SIGINT, sig_handler);
    Server server;
    // server.InitALL();
    server.InitSocket();
    server.Bindserver(7070);
    server.Listen();
    server.Start(_ThreadServer_COUNT);

	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else {
			printf("undefine cmd\n");
		}
	}

    // server.Close();
	sleep(5);
    return 0;
}