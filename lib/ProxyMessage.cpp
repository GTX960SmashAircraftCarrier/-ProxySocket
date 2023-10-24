#include "ProxyMessage.h"

ProxyMsg makeProxyMsg(ProxyMsgType type, char* data, size_t len) {
    ProxyMsg msg = {};
    msg.type = type;
    memcpy(msg.data, data, len);
    msg.len = sizeof(u_int32_t) + sizeof(ProxyMsgType) + len;
    return msg;
}

size_t getProxyMsgBodySize(const ProxyMsg& msg){
    return msg.len - sizeof(ProxyMsgType) - sizeof(u_int32_t);
}