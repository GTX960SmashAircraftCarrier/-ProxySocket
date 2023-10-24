#ifndef _SERVER_HPP_
#define _SERVER_HPP_
#endif

#include <mutex>
#include <atomic>
#include <iostream>
#include <netinet/in.h>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../conf/Header.hpp"
#include "../conf/ThreadClient.hpp"
#include "../conf/TimeStamp.hpp"
#include "ThreadTask.hpp"
#include "../conf/ObjectPool.hpp"
#include "ThreadSemaphore.hpp"
#include "ThreadControl.hpp"
#define _ThreadServer_COUNT 4
#define max_obj_nmber 10

typedef std::shared_ptr<ThreadClient> ThreadClientptr;