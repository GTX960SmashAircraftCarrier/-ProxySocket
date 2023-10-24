g++ -c Client.cpp
g++ -c Tunnel.cpp
g++ -c Client_connect.cpp
g++ -o test main.cpp Client.o Tunnel.o  Client_connect.o