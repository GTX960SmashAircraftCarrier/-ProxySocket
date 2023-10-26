# 反向代理 + 并发收发网络
## 项目介绍
本项目实现一个反向代理收发并发网络，包含内网服务器LocalServer，可以访问外网的ProxyClient，公网的ProxySerer，外网Client；
ProxyClient和LocalServer运行在一个局域网内，ProxySerer有公网IP；
## 项目特色
- 服务器端采用跨平台select，代理服务器采用epoll，均采用主从Reactor多线程并发模型设计模式。
- 收发服务端采用最小连接数算法均衡服务器负载。
- 代理服务器与收发服务器均采用控制与业务解耦。
- 收发服务器采用智能指针对象池、内存池技术，实现分配与自动回收，扩容与回收空间。
- 代理服务器采用线程池技术，复用proxy连接。
- 代理服务器采用splice零拷贝，减少上下文切换与拷贝次数。
- 借鉴Redis过期策略，采用定期删除和主动询问关闭过期连接，定期回收内存池未使用内存。
- 信号量机制控制客户端和服务端的线程安全顺序退出

## 运行流程
```bash
# 编译proxy
mkdir build
cd build
cmake ..
make
# 编译client\server
cd ..
g++ -o Client/client Client/client.cpp -lpthread
g++ -o Server/server Server/server.cpp -lpthread

# 启动proxy server
./Proxyserver/Proxyserver --proxy_port=8090 --work_thread_nums=1
./Proxyclient/Proxyclient  --local_server=10.101.29.12:7070 --cproxy_server=10.101.29.12:8080

./Server/server
./Client/client
```
## 运行细节
![](https://raw.githubusercontent.com/GTX960SmashAircraftCarrier/imgs/master/20231026100212.png)

## 并发结构
### 客户端架构
![](https://raw.githubusercontent.com/GTX960SmashAircraftCarrier/imgs/master/20231026102910.png)
### 服务端架构
![](https://raw.githubusercontent.com/GTX960SmashAircraftCarrier/imgs/master/20231026102916.png)