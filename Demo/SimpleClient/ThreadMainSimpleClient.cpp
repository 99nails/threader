#include "ThreadMainSimpleClient.h"

#include "ThreadSimpleClientSocket.h"

ThreadMainSimpleClient::ThreadMainSimpleClient()
    : ThreadMainDaemon("Thread.Main",
                       "SimpleClient",
                       "Тестовый TCP клиент")
    , _threadClient(nullptr)
{
}


void ThreadMainSimpleClient::startChildThreads()
{
    _threadClient = new ThreadSimpleClientSocket(this, INVALID_DESCRIPTOR, "192.168.100.93", 53817);
    registerAndStartChildThread(_threadClient);
}
