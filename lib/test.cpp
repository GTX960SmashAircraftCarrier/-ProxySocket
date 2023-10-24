#include "Buffer.h"
#include <iostream>
using namespace std;
int main() {
    Buffer buf(1024,1024);
    char* data1 = (char*)malloc(1023);
    cout<<buf.getunused()<<endl;
    buf.WriteToBuffer(data1, 1023);
    cout<<buf.getunused()<<endl;
    buf.read(data1, 512);
    cout<<buf.getunused()<<endl;
    buf.WriteToBuffer(data1, 128);
    cout<<buf.getunused()<<endl;
    buf.read(data1, 512);
    cout<<buf.getunused()<<endl;
    free(data1);
    return 0;
}