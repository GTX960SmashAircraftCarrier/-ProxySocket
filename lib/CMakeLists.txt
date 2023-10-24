set(lib
    Buffer.cpp
    Channel.cpp
    CtlConnect.cpp
    Epoll.cpp
    Event_loop.cpp
    Message.cpp
    ProxyConnect.cpp
    ProxyMessage.cpp
    Thread_Event_loop.cpp
    Thread_EventLoop_pool.cpp
    Utils.cpp
)

add_library(lib ${lib})
# 使用多线程需要引入的静态库
target_link_libraries(lib pthread)