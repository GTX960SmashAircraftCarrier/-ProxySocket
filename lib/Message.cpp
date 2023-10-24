#include <string.h>
#include "Message.h"

Msg MakeMsg(MsgType type, char* data, size_t len){
    Msg msg = Msg{};
    msg.type  = type;
    memcpy(msg.data, data, len);
    msg.len = sizeof(u_int32_t) + sizeof(MsgType) + len;
    return msg;
}

size_t getMsgBodySize(const Msg& msg) {
    return msg.len - sizeof(MsgType) - sizeof(u_int32_t);
}
