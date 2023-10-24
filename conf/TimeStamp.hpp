#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP
#include<chrono>

class Time{
public:
    static size_t time(){
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
};

class TimeStamp{
public:
    TimeStamp(){
        update();
    }
    ~TimeStamp(){}
    void update(){
        begin = std::chrono::high_resolution_clock::now();
    } 

    double getElapseSecond(){
        return this->getElapseUs() * 0.000001;
    }
    double getElapseMs(){
        return this->getElapseUs() * 0.001;
    }
    int64_t getElapseUs(){

        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> begin;
};
#endif