#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include "Easyclient.hpp"
#include "../conf/TimeStamp.hpp"
#include <csignal>
#include<atomic>

static volatile int keepRunning = 1;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        keepRunning = 0;
    }
}

void receive_thread(EasyClient *client){
    while(keepRunning){
        client->Handel();
    }
}

int main(){
    signal(SIGINT, sig_handler);

    EasyClient client;
    client.Connect("127.0.0.1",7777);

    std::thread t1(receive_thread, &client);
	t1.detach();
    
    // ClientLogin login[10];
	// for (int n = 0; n < 10; n++)
	// {
	// 	strcpy(login[n].user, "cqy");
	// 	strcpy(login[n].pwd, "123");
	// }
    // ClientLogin login;
    std::shared_ptr<ClientLogin> login = std::make_shared<ClientLogin>();
    while(keepRunning){
        // std::cout<<"sending \n";
        if(client.SendData(login) == 0){
            std::cout<<"full sendbuf\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
   
    sleep(2);
    // client.Close();
    std::cout<<"all done\n";
    return 0;
}