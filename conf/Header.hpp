#ifndef HEADER_HPP
#define HEADER_HPP

enum CMD{
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_EXPIRE,
    CMD_ERROR,
};
struct DataHeader{
    DataHeader(){
        len =sizeof(DataHeader);
        cmd = CMD_ERROR;
    }
    short len;
    short cmd;
};

struct ClientLogin : public DataHeader{
    ClientLogin(){
        len = sizeof(ClientLogin);
        cmd = CMD_LOGIN;
    }
    char user[32];
    char pwd[32];
};

struct ClientLoginResult : public DataHeader{
    ClientLoginResult(){
        len = sizeof(ClientLoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    int result;
};

struct ClientLogout : public DataHeader{
    ClientLogout(){
        len = sizeof(ClientLogout);
        cmd = CMD_LOGOUT;
    }
    char user[32];
};

struct ClientLogoutResult : public DataHeader{
    ClientLogoutResult(){
        len = sizeof(ClientLogoutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    }
    int result;
};

struct NewClientJoin : public DataHeader{
    NewClientJoin(){
        len = sizeof(NewClientJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock = 1;
};

struct Expire : public DataHeader{
    Expire(){
        len = sizeof(NewClientJoin);
        cmd = CMD_EXPIRE;
        ret = 1;
    }
    int ret;
};


#endif