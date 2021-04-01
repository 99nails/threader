#include "ThreadUdpSocket.h"

#include "../Common/Logs/LogMessagesTemplatesDemo.h"


ThreadUdpSocket::ThreadUdpSocket(IMessageSubscriber *parent,
                                 const QString &host,
                                 const uint16_t &port,
                                 const HandlerUdpSocket::SocketType &socketType)
    : ThreadHandler(parent,
                    new HandlerUdpSocket(host, port),
                    1,
                    nullptr,
                    "Thread.Udp")
    , _socketType(socketType)
{
    setTimeout(1000);
}


void ThreadUdpSocket::onThreadStarted()
{
    writeLog(nullptr, Message3000,
             STRLOG(handler()->deviceName()));
}

void ThreadUdpSocket::onThreadFinishing()
{
    writeLog(nullptr, Message3001,
             STRLOG(handler()->deviceName()));
}

void ThreadUdpSocket::onThreadFinished()
{
    writeLog(nullptr, Message3002,
             STRLOG(handler()->deviceName()));
}

void ThreadUdpSocket::onIdle()
{
    if (HandlerUdpSocket::SocketType::Sender == _socketType && handler())
    {
        QString sendString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
        outputStream()->writeString(sendString);
    }
}

void ThreadUdpSocket::onConnecting()
{
    writeLog(nullptr, Message3003,
             STRLOG(handler()->deviceName()));
}

void ThreadUdpSocket::onConnected()
{
    writeLog(nullptr, Message3004,
             STRLOG(handler()->deviceName()));
}

void ThreadUdpSocket::onDisconnected()
{
    writeLog(nullptr, Message3006,
             STRLOG(handler()->deviceName()));
}

void ThreadUdpSocket::onError(const int errorCode)
{

    writeLog(nullptr, Message3005,
             STRLOG(handler()->deviceName()),
             errorCode,
             STRLOG(errorString(errorCode)));
}

void ThreadUdpSocket::onReadData(const char *data,
                                 const int &count)
{
    writeLog(nullptr, Message3007,
             count, STRLOG(handler()->deviceName()));

    writeLog(nullptr, Message3009,
             STRLOG(handler()->deviceName()), data);

    if (HandlerUdpSocket::SocketType::Listener == _socketType && handler())
    {
        outputStream()->write(data, count);
    }
}

void ThreadUdpSocket::onWriteData(const char *data, const int &count)
{
    writeLog(nullptr, Message3008,
             count, STRLOG(handler()->deviceName()));
    Q_UNUSED(data)
}

bool ThreadUdpSocket::onPacketReceived(const PacketBase::Ptr &)
{
    return true;
}

bool ThreadUdpSocket::onPacketSent(const PacketBase::Ptr &)
{
    return true;
}
