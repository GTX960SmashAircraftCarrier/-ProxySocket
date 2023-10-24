cmake_minimum_required(VERSION 3.10)

project(Proxy)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(lib)
add_subdirectory(Proxyclient)
add_subdirectory(Proxyserver)
