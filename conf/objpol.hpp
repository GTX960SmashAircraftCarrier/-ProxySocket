#include<stdlib.h>
#include<mutex>

template<class T, size_t PoolSize>
class ObjecPoolBlock{
public:
    ObjecPoolBlock(){
        init();
    }
    ~ObjecPoolBlock(){
        if(_mem)
            delete[] _mem;
    }
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    
    void* allocObjectMem(size_t size){
        std::lock_guard<std::mutex> lk(_mutex);
        if(!_mem)
            init();
        Node *AllocRet = nullptr;
        if(_header == nullptr){
            // all used
            AllocRet = (Node*)new char(sizeof(T) + sizeof(Node));
            AllocRet->Id = -1;
            AllocRet->nRef = -1;
            AllocRet->bPool = false;
            AllocRet->next = nullptr;
        }
        else{
            //return one 
            AllocRet = _header;
            _header = _header->next;
            assert(AllocRet->nRef == 0);
            AllocRet->nRef = 1;
        }
        std::printf("has been new\n");
        return ((char*)AllocRet + sizeof(Node));
    }

    void DeallocMem(void* Mem){
        Node* block = (Node*)((char*)Mem - sizeof(Node));
        // unique own
        assert(block->nRef == 1);
        if(block->bPool){
            std::lock_guard<std::mutex> lk(_mutex);
            //interrupt by other
            if(--block->nRef !=0)
                return;
            // add to header
            block->next = _header;
            _header = block;
        }
        else{
            if(--block->nRef != 0)
                return;
            delete[] block;
        }
        std::printf("has been denew\n");
    }

private:
    void init(){
        assert(_mem == nullptr);
        if(_mem) return;
        size_t  newsize = sizeof(T) + sizeof(Node);
        size_t  totalmemsize = PoolSize * newsize;
        _mem = new char[totalmemsize];

        _header = (Node*)_mem;
        _header->Id = 0;
        _header->nRef = 0;
        _header->bPool = true;
        _header->next = nullptr;

        Node* temp = _header;
        for(size_t i = 1; i < PoolSize; i++){
            Node* cur = (Node*)(_mem + (i * newsize));
            _header->Id = i;
            _header->nRef = 0;
            _header->bPool = true;
            _header->next = nullptr;
            temp->next = cur;
            temp = cur;
        }
    }

    class Node{
    public:
        Node* next;
        int nId;
        int nRef;
        bool Bpool;
    };
    Node* _header;
    char* _mem;
    std::mutex _mutex;
};


template<class T, size_t PoolSize>
class ObjecPoolAllocator{
public:
    void* operator new(size_t size){
        return objectpool()->allocObjectMem(size);
    }

    void operator delete(void* p){
        objectpool()->DeallocObjectMem(p);
    }

private:
    typedef ObjecPoolBlock<T, PoolSize> ObjecPool;
    static ObjecPoolAllocator& objectpool(){
        static ObjecPool pool;
        return pool;
    }

};
