#ifndef _THREAD_CONTROL_HPP
#define _THREAD_CONTROL_HPP
#include "ThreadSemaphore.hpp"
#include<functional>
class ThreadControl{
private:
    typedef std::function<void(ThreadControl*)> Call;
public:
    void Start(Call Create = nullptr, Call Run = nullptr, Call Destory = nullptr){
        std::lock_guard<std::mutex> lk(_mutex);
        if(!isRun){
            isRun = true;
            if(Create) _Create = Create;
            if(Run) _Run = Run;
            if(Destory) _Destory = Destory;
            std::thread t(std::mem_fn(&ThreadControl::Run), this);
            t.detach();
        }
    }

    void Close(){
        std::lock_guard<std::mutex> lk(_mutex);
        if(isRun){
            isRun = false;
            _sem.wait();
        }
    }

    void Exit(){
        std::lock_guard<std::mutex> lk(_mutex);
        if(isRun){
            isRun = false;
        }
    }
    bool IsRun(){
        return isRun;
    }
protected:
    void Run(){
        if(_Create) _Create(this);
        if(_Run) _Run(this);
        if(_Destory) _Destory(this);
        _sem.wakeup();
    }
private:
    Call _Create;
    Call _Run;
    Call _Destory;
    std::mutex _mutex;
    ThreadSemaphore _sem;
    bool isRun = false;
};

#endif