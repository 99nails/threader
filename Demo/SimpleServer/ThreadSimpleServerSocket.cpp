#include "ThreadSimpleServerSocket.h"

#include "../Common/Logs/LogMessagesTemplatesDemo.h"

#include "HandlerTcpSocket.h"
#include "DateUtils.h"

ThreadSimpleServerSocket::ThreadSimpleServerSocket(IMessageSubscriber *parent,
                                                   Descriptor socket,
                                                   const QString host,
                                                   const uint16_t port)
    : ThreadHandler(parent,
                    new HandlerTcpSocket(host, port, socket),
                    0,
                    new PacketFactorySimple())
{
    setThreadName(QString("Thread.Server.%1").arg(handler()->deviceName()));
    setTimeout(1000);
}


void ThreadSimpleServerSocket::onThreadStarted()
{
    writeLog(nullptr, Message1100,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleServerSocket::onThreadFinishing()
{
    writeLog(nullptr, Message1101,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleServerSocket::onThreadFinished()
{
    writeLog(nullptr, Message1102,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleServerSocket::onIdle()
{
    if (!handler() || handler()->connectionState() != HandlerBase::ConnectionState::Connected)
        return;

    if (lastPacketReceived() + 30000 <= DateUtils::getTickCount())
    {
        writeLog(nullptr, Message1202,
                 handler()->deviceName().toUtf8().constData(), 30000);
        terminateThread();
    }
}

void ThreadSimpleServerSocket::onConnecting()
{
    writeLog(nullptr, Message1103,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleServerSocket::onConnected()
{
    writeLog(nullptr, Message1104,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleServerSocket::onDisconnected()
{
    writeLog(nullptr, Message1105,
             handler()->deviceName().toUtf8().constData());
    terminateThread();
}

void ThreadSimpleServerSocket::onError(const int errorCode)
{
    writeLog(nullptr, Message1106,
             handler()->deviceName().toUtf8().constData(),
             errorCode,
             errorString(errorCode).toUtf8().constData());
}

void ThreadSimpleServerSocket::onReadData(const char *, const int &size)
{
    writeLog(nullptr, Message1107,
             size,
             handler()->deviceName().toUtf8().constData());
}

void ThreadSimpleServerSocket::onWriteData(const char *, const int &size)
{
    writeLog(nullptr, Message1108,
             size,
             handler()->deviceName().toUtf8().constData());
}

bool ThreadSimpleServerSocket::onPacketReceived(const PacketBase::Ptr &packet)
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
    case SimplePacketType::Hello:
        doPacketHello(packetSimple);
        break;
    case SimplePacketType::Alive:
        doPacketAlive(packetSimple);
        break;
    default:
        break;
    }

    return true;
}

bool ThreadSimpleServerSocket::onPacketSent(const PacketBase::Ptr &packet)
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

void ThreadSimpleServerSocket::doPacketHello(PacketSimple::Ptr packet)
{
    QString name = QString(*packet->data());
    writeLog(nullptr, Message1200,
             name.toLocal8Bit().constData(),
             handler()->deviceName().toUtf8().constData());

    sendPacketWelcome("Server");
}

void ThreadSimpleServerSocket::doPacketAlive(PacketSimple::Ptr packet)
{
    sendPacketAlive();

    Q_UNUSED(packet)
}

void ThreadSimpleServerSocket::sendPacketWelcome(const QString &name)
{
    auto factory = dynamic_cast<PacketFactorySimple*>(packetFactory());
    if (!factory)
        return;

    auto packet = factory->buildPacketWelcome(name);
    if (!packet)
        return;

    sendPacket(packet);

}

void ThreadSimpleServerSocket::sendPacketAlive()
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
