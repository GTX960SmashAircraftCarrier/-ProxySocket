# 反向代理 + 并发收发网络
## 项目介绍
本项目实现一个反向代理收发并发网络，包含内网服务器LocalServer，可以访问外网的ProxyClient，公网的ProxySerer，外网Client；
ProxyClient和LocalServer运行在一个局域网内，ProxySerer有公网IP；
# 运行流程
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
./Proxyerver/Proxyserver --proxy_port=8090 --work_thread_nums=1
./Proxyclient/Proxyclient  --local_server=10.101.29.12:7070 --cproxy_server=10.101.29.12:8080

./Server/server
./Client/client
```

![](https://raw.githubusercontent.com/GTX960SmashAircraftCarrier/imgs/master/20231023172216.png)