#include "ThreadSimpleClientSocket.h"

#include "../Common/Logs/LogMessagesTemplatesDemo.h"

#include "../../Threads/HandlerTcpSocket.h"
#include "../../Utils/SocketUtils.h"
#include "../../Utils/DateUtils.h"

ThreadSimpleClientSocket::ThreadSimpleClientSocket(IMessageSubscriber *parent,
                                                   Descriptor socket,
                                                   const QString host,
                                                   const uint16_t port)
    : ThreadHandler(parent,
                    new HandlerTcpSocket(host, port, socket),
                    10000,
                    new PacketFactorySimple())
{
    setThreadName(QString("Thread.Server.%1").arg(handler()->deviceName()));
    setTimeout(1000);
}


void ThreadSimpleClientSocket::onThreadStarted()
{
    writeLog(nullptr, Message1100,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleClientSocket::onThreadFinishing()
{
    writeLog(nullptr, Message1101,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleClientSocket::onThreadFinished()
{
    writeLog(nullptr, Message1102,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleClientSocket::onIdle()
{
    if (!handler() || handler()->connectionState() != HandlerBase::ConnectionState::Connected)
        return;

    qint64 nextSendAlive = lastPacketSent() + 10000;
    if (nextSendAlive <= DateUtils::getTickCount())
    {
        sendPacketAlive();
    }

    if (lastPacketReceived() + 30000 < DateUtils::getTickCount())
    {
        writeLog(nullptr, Message1202,
                 handler()->deviceName().toUtf8().constData(), 30000);
        handler()->close();
    }
}

void ThreadSimpleClientSocket::onConnecting()
{
    writeLog(nullptr, Message1103,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleClientSocket::onConnected()
{
    writeLog(nullptr, Message1104,
             handler()->deviceName().toUtf8().constData());
    sendPacketHello("Client");
}

void ThreadSimpleClientSocket::onDisconnected()
{
    writeLog(nullptr, Message1105,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleClientSocket::onError(const int errorCode)
{
    writeLog(nullptr, Message1106,
             handler()->deviceName().toUtf8().constData(),
             errorCode,
             errorString(errorCode).toUtf8().constData());
}

void ThreadSimpleClientSocket::onReadData(const char *, const int &size)
{
    writeLog(nullptr, Message1107,
             size,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleClientSocket::onWriteData(const char *, const int &size)
{
    writeLog(nullptr, Message1108,
             size,
             handler()->deviceName().toUtf8().constData());
}


bool ThreadSimpleClientSocket::onPacketReceived(const PacketBase::Ptr &packet)
{
    auto packetSimple = std::dynamic_pointer_cast<PacketSimple>(packet);
    SimplePacketType type = SimplePacketType(packetSimple->header().packetType);
    QString packetTypeName = "Unknown";
    auto *packetFactorySimple = dynamic_cast<PacketFactorySimple*>(packetFactory());
    if (packetFactorySimple)
        packetTypeName = packetFactorySimple->packetTypeName(type);

    writeLog(nullptr, Message1109,
             packetTypeName.toLocal8Bit().constData(),
             packetSimple->header().packetType,
             packetSimple->header().packetId,
             packetSimple->header().length,
             handler()->deviceName().toUtf8().constData());

    switch (SimplePacketType(packetSimple->header().packetType)) {
    case SimplePacketType::Welcome:
        doPacketWelcome(packetSimple);
        break;
    default:
        break;
    }

    return true;
}

bool ThreadSimpleClientSocket::onPacketSent(const PacketBase::Ptr &packet)
{
    auto packetSimple = std::dynamic_pointer_cast<PacketSimple>(packet);
    SimplePacketType type = SimplePacketType(packetSimple->header().packetType);
    QString packetTypeName = "Unknown";
    auto *packetFactorySimple = dynamic_cast<PacketFactorySimple*>(packetFactory());
    if (packetFactorySimple)
        packetTypeName = packetFactorySimple->packetTypeName(type);

    writeLog(nullptr, Message1110,
             packetTypeName.toLocal8Bit().constData(),
             packetSimple->header().packetType,
             packetSimple->header().packetId,
             packetSimple->header().length,
             handler()->deviceName().toUtf8().constData());

    return true;
}

void ThreadSimpleClientSocket::doPacketWelcome(PacketSimple::Ptr packet)
{    
    QString name = QString(*packet->data());
    writeLog(nullptr, Message1201,
             name.toLocal8Bit().constData(),
             handler()->deviceName().toUtf8().constData());

    sendPacketAlive();
}

void ThreadSimpleClientSocket::sendPacketHello(const QString &name)
{
    auto factory = dynamic_cast<PacketFactorySimple*>(packetFactory());
    if (!factory)
        return;

    auto packet = factory->buildPacketHello(name);
    if (!packet)
        return;

    sendPacket(packet);
}

void ThreadSimpleClientSocket::sendPacketAlive()
{
    auto factory = dynamic_cast<PacketFactorySimple*>(packetFactory());
    if (!factory)
        return;

    auto packet = factory->buildPacketAlive(QDateTime::currentDateTime().toMSecsSinceEpoch(),
                                            DateUtils::getTickCount());
    if (!packet)
        return;

    sendPacket(packet);
}
