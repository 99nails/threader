#include "ThreadMainUdp.h"

#include "ThreadUdpSocket.h"

HandlerUdpSocket::SocketType ThreadMainUdp::_socketType = HandlerUdpSocket::SocketType::Listener;

ThreadMainUdp::ThreadMainUdp()
    : ThreadMainDaemon("Thread.Main",
                       "UdpCondole",
                       "Udp Console",
                       nullptr)
    , _threadUdp(nullptr)
{

}

void ThreadMainUdp::startChildThreads()
{
    QString ipAddress = (_socketType == HandlerUdpSocket::SocketType::Listener)
            ? "0.0.0.0" : "localhost";

    _threadUdp = registerAndStartChildThread(new ThreadUdpSocket(this,
                                                                 ipAddress,
                                                                 58964,
                                                                 _socketType));
}

void ThreadMainUdp::setSocketType(const HandlerUdpSocket::SocketType &socketType)
{
    _socketType = socketType;
}
