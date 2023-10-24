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

const unsigned int totalClient = 8;
const int totalSend = 4;
std::atomic<unsigned int> Sendcount(0);
std::atomic<unsigned int> readycount(0);
EasyClient* client[totalClient];

void receive_thread(int begin, int end){
    while(keepRunning){
        for(int i = begin; i < end; i++)
            client[i]->Handel();
    }
}

void sendThread(int id){
    std::cout<<"thread: "<<id<<" is runing\n";
    int c = totalClient /totalSend;
    int begin = id * c;
    int end = (id + 1) * c;
    for(int i = begin; i < end; i++)
        client[i] = new EasyClient();
    for(int i = begin; i < end; i++)
        client[i]->Connect("10.101.29.12", 7070);

    printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

    readycount++;
    while(readycount < totalSend){
        std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
    }
    
    std::thread t1(receive_thread, begin, end);
	t1.detach();

    ClientLogin login[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].user, "cqy");
		strcpy(login[n].pwd, "123");
	}
	const int nLen = sizeof(login);

    while(keepRunning){
        for (int n = begin; n < end; n++)
		{
			if (client[n]->SendData(login, nLen) != -1)
			{
				Sendcount++;
			}
		}
        std::chrono::milliseconds t(100);
		std::this_thread::sleep_for(t);
    }
    std::cout<<" cleanup all, please wait ~~~~~\n";
    sleep(10);
    for (int i = begin; i < end; i++)
	{
		client[i]->Close();
		delete client[i];
        
	}
    
    printf("thread<%d>,exit\n", id);
}

int main(){
    signal(SIGINT, sig_handler);

    for (int i = 0; i < totalSend; i++)
	{
		std::thread t1(sendThread,i);
		t1.detach();
	}
    
    TimeStamp Time;
    while(keepRunning){
        auto t = Time.getElapseSecond();
        if (t >= 1.0)
		{
			printf("thread<%d>,clients<%d>,time<%lf>,send<%d>\n",totalSend, totalClient,t,(int)(Sendcount/ t));
			Sendcount = 0;
			Time.update();
            
		}
        sleep(1);
    }


    sleep(10);
    std::cout<<"all done\n";
    return 0;
}