#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <sys/types.h>

const unsigned int MAX_MSG_LEN = 1024;

enum class MsgType {
    NewRequest,
    NewResponse,
    NewTunnelReq,
    NewTunnelRsp,
    NotifyClient,
    ShutdownPeerConn,
    FreeConnReq
};

struct Msg
{
    u_int32_t len;
    MsgType type;
    char data[MAX_MSG_LEN];
};

struct NewRequestMsg {};

struct NewResponseMsg
{
    char id[0];
};

struct NewTunnelReqMsg
{
    char server_host[20];
    u_int32_t server_port;
};

struct NewTunnelRspMsg
{
    u_int32_t proxy_server_port;
    u_int32_t server_port;
    char tunnel_id[10];
    char server_host[20];
    char proxy_server_host[20];
};

//通知可以建立的端口号
struct NotifyClientNeedProxyMsg
{
    char tunnel_id[10];
    u_int32_t proxy_server_port;
};

struct ShutdownPeerConnMsg {
    char tunnel_id[10];
    char proxy_id[10];
    u_int32_t tran_count;
};

struct FreeProxyConnReqMsg {
  char tun_id[10];
  char proxy_id[10];
};

Msg MakeMsg(MsgType type, char* data, size_t len);

size_t getMsgBodySize(const Msg& msg);

#endif