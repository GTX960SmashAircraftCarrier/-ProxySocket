// #include "ObjectPool.hpp"
#include<iostream>
#include<vector>
#include <map>
using namespace std;
// class A{
// public:
//     A(){ 
//         a = 3;
//         // cout<<"init"<<endl;
//         }
//     ~A(){
//         cout<<"directly free"<<endl;
//     }
//     int geta(){
//         return a;
//     }
// private:
//     int a;
// };



int main()
{
    map<int, int> m;
    m.insert(make_pair<int, int>(1,10));
    // m.insert(make_pair<int, int>(2,11));
    // m.insert(make_pair<int, int>(3,12));
    auto it = m.begin();
    it = m.erase(it);
    cout<<(it == m.end())<<endl;
    for(auto it= m.begin(); it != m.end(); it++){
        cout<<"brefor: "<<it->first<<endl;
        if(it->first == 1){
            it = m.erase(it);
            if(it != m.end())
                it--;
        }
        // cout<<"after: "<<it->first<<endl;
        
    // }
    // A *p = new A[10];
    // vector<shared_ptr<A>> list;
    // for(int i = 0; i < 10; i++){
    //     list.push_back(shared_ptr<A>(p));
    // }
    // shared_ptr<A []> aa(new A[10], ArrayDeleter);
    // for(int i = 0; i < 10; i++){
    //    cout<<aa<<endl;
    // }

    // vector<shared_ptr<A>> list;
    // for(int i = 0; i < 10; i++){
    //     list.push_back(shared_ptr<A>(new A));
    // }
    // for(int i = 0; i < 10; i++){
    //    cout<<list[i].get()<<endl;
    // }

    // for(int i = 0; i < 10; i++){
    //     cout<<list[i].get()<<endl;
    // }

    // vector<int> aa(100);
    // for(auto e:list)
    //     cout<<e;
    // shared_ptr<A> *p = new A[len]();


    // ObjectPool<A,10,5> pool;
    // shared_ptr<A> p[30];
    // for(int i = 0; i < 30; i++){
    //    p[i]  = pool.alloc();
    //    cout<<pool.getUsed()<<"  "<<pool.getUnused()<<endl;
    // }
    // for(int i = 0; i < 30; i++){
    //     p[i].reset();
    //     cout<<pool.getUsed()<<"  "<<pool.getUnused()<<endl;
    // }

    // }
}