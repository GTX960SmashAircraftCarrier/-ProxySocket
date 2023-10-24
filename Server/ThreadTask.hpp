#ifndef _THREAD_TASK_HPP_
#define _THREAD_TASK_HPP_
#include <thread>
#include <mutex>
#include <list>
#include <functional>
#include "ThreadControl.hpp"
typedef std::shared_ptr<ThreadClient> ThreadClientptr;
class ThreadTask{
public:
	ThreadTask(){}
	virtual ~ThreadTask(){}
	
	virtual void Deal(){}
};


class Send2Client: public ThreadTask{
public:
    Send2Client(const ThreadClientptr& pclient, std::shared_ptr<DataHeader> header):pclient(pclient), header(header){}
	~Send2Client(){}
	void Deal(){
        // std::cout<<"deal is called\n";
        if(pclient->SendData(header) < 0){

		}
		// do not need to free
		// delete(header)
    }
private:
    ThreadClientptr pclient;
    std::shared_ptr<DataHeader>	 header;
};


typedef std::shared_ptr<ThreadTask> ThreadTaskPtr;
class ThreadTaskServer{
public:
    void addTask(const ThreadTaskPtr& task){
		std::lock_guard<std::mutex> lock(_mutex);
		tasksBuf.push_back(task);
	}
    void Start(){
		_thread.Start(nullptr, [this](ThreadControl* pThead){Run(pThead);});
		// std::cout<<"thread task is creating\n";
		// std::thread t(std::mem_fn(&ThreadTaskServer::Run),this);
		// t.detach();
	}
	
	void Close(){
		printf("CELLTaskServer.Close begin\n");
		_isRun = false;
		printf("CELLTaskServer.Close begin\n");
	}

	bool AllDone(){
		return (task.size() + tasksBuf.size()) == 0;
	}
protected:
    void Run(ThreadControl* pThead){
        while(pThead->IsRun())
		{
			if (!tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : tasksBuf)
					task.push_back(pTask);
				
                tasksBuf.clear();
			}

			if (task.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			for (auto ptask : task)
			{
				ptask->Deal();
			}
			task.clear();
		}
    }


private:
    std::list<ThreadTaskPtr> task;
    std::list<ThreadTaskPtr> tasksBuf;
	ThreadControl _thread;
    std::mutex _mutex;
	bool _isRun;
}; 

#endif