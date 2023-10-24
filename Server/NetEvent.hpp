#ifndef _NET_EVENT_HPP_
#define _NET_EVENT_HPP_

#include "Server.hpp"

class ThreadServer;

class NetEvent
{
public:
	virtual void OnNetJoin(const ThreadClientptr& pClient) = 0;

	virtual void OnNetLeave(const ThreadClientptr& pClient) = 0;

	virtual void OnNetMsg(ThreadServer* pCellServer, const ThreadClientptr& pClient, DataHeader* header) = 0;
	
	virtual void OnNetRecv(const ThreadClientptr& pClient) = 0;
private:

};

#endif