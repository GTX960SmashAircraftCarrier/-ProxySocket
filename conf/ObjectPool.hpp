#ifndef _OBJECTPOOL_HPP_
#define _OBJECTPOOL_HPP_
#include <string.h>
#include <mutex>
#include <list>
#include <memory>
#include <functional>
#include "TimeStamp.hpp"
//time to recycle ms
#define recycletime 10000

template<class T, size_t initnum, size_t increase = initnum / 10, size_t reduce = initnum / 10>
class ObjectPool{
public:
    ObjectPool(){
        _ObjDeleter = [&](T* obj){
            if(Deconstruct){
                // printf("no use, delete by orignal new\n");
                delete obj;
            }
            else{
                std::lock_guard<std::mutex> lk(_mutex);
                // printf("recycle by pool\n");
                // deaddobj();
                --_UseNum;
                _free_list.push_back(std::shared_ptr<T>(obj, _ObjDeleter));
            }
        };
        init();
    }
    ~ObjectPool(){
        Deconstruct = true;
    }
    bool init(){
        if( initnum < 0 || increase <= 0 || reduce <= 0){
            return false;
        }
        _UseNum = 0;
        _InitNum = initnum;
        _IncreaseStep = increase;
        _ShrinkNum = reduce;
        _CurNum = initnum;
        _curtime = 0;
        for(int i = 0; i < _InitNum; i++){
            _free_list.push_front(std::shared_ptr<T>(new T(), _ObjDeleter));
            // _free_list.push_front(std::shared_ptr<T>(&data[i], _ObjDeleter));
        }
        printf("Pool ini succee, total ojb: %ld \n", initnum);
        return true;
    }
    
    std::shared_ptr<T> alloc(){
        std::shared_ptr<T>ret = nullptr;
        std::lock_guard<std::mutex> lk(_mutex);
        if(!_free_list.empty()){
            ret = _free_list.front();
            _free_list.pop_front();
            ++_UseNum;
            printf("get one from pool\n");
            return ret;
        }

        // if (!ret){
        //     ret.reset(new T());
        //     printf("pool is full, get new one !!!\n");
        // }
        // return ret;

        addnewobj();
        ++_UseNum;
        ret = _free_list.front();
        _free_list.pop_front();
        return ret;
    }

    void Dealloc(std::shared_ptr<T> &obj){
        if(!obj) return;
        std::lock_guard<std::mutex> lk(_mutex);

        _free_list.push_back(obj);
        printf("recycle one \n");
        _UseNum--;
        deaddobj();
    }

    int getUsed(){
        return _UseNum;
    }
    int getUnused(){
        return _CurNum - _UseNum;
    }

    
    void Resettime(){
        _curtime = 0;
    }

    bool checkexpire(time_t time){
        _curtime += time;
        return _curtime >= recycletime;
    }
private:
    void addnewobj(){
        // std::lock_guard<std::mutex> lk(_mutex);
        for(int i = 0; i < _IncreaseStep; i++){
            _free_list.push_front(std::shared_ptr<T>(new T(), _ObjDeleter));
        }
        printf("new obj: %d\n", _IncreaseStep);
        _CurNum += _IncreaseStep;
    }
    void deaddobj(){
        // std::lock_guard<std::mutex> lk(_mutex);
        if((_CurNum - _UseNum ) <= _IncreaseStep * 2 || _CurNum - _InitNum <= _IncreaseStep || !checkexpire(Time::time())) return;
        Resettime();
        Deconstruct = false;
        for(int i = 0; i < _IncreaseStep; i++)
             _free_list.pop_back();
        Deconstruct = true;
        printf("delete obj: %d\n", _IncreaseStep);
    }
private:
    std::function<void(T* obj)> _ObjDeleter;
    std::list<std::shared_ptr<T>> _free_list;
    int _InitNum;
    int _CurNum;
    int _UseNum;
    int _IncreaseStep;
    int _ShrinkNum;
    std::mutex _mutex;
    volatile bool Deconstruct = false;
    time_t _curtime;
};

#endif