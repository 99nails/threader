#include "ThreadMainSimpleServer.h"

#include "ThreadSimpleServerListenSocket.h"

ThreadMainSimpleServer::ThreadMainSimpleServer()
    : ThreadMainDaemon("Thread.Main",
                       "SimpleService",
                       "Тестовый TCP сервер")
{
}


void ThreadMainSimpleServer::startChildThreads()
{
    _threadListen = new ThreadSimpleServerListenSocket(this);
    registerAndStartChildThread(_threadListen);
}
