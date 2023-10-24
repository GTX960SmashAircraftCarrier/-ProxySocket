#ifndef PROXYMESSAGE_HPP
#define PROXYMESSAGE_HPP
#include <sys/types.h>
#include <string.h>
const int MAX_LEN = 1024;

enum class ProxyMsgType {
    ProxyMetaSet,
    ProxyConnectReq,
    ProxyConnectRsp
};

struct ProxyMsg{
   u_int32_t len;
   ProxyMsgType type;
   char data[MAX_LEN];
};

struct ProxyMetaSetMsg
{
    char ctl_id[10];
    char tun_id[10];
    char proxy_id[10];
};

struct ProxyConnectReqMsg
{
    u_int32_t fd;
    char proxy_id[10];
};

struct ProxyConnectRspMsg
{
    u_int32_t fd;
    char proxy_id[10];
};

ProxyMsg makeProxyMsg(ProxyMsgType type, char* data, size_t len);

size_t getProxyMsgBodySize(const ProxyMsg& msg);

#endif