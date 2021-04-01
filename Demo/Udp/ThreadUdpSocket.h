#pragma once

#include "HandlerUdpSocket.h"
#include "ThreadHandler.h"

#include <QObject>

using namespace Threader::Threads;

class ThreadUdpSocket : public ThreadHandler
{
    Q_OBJECT
public:
    explicit ThreadUdpSocket(IMessageSubscriber *parent,
                             const QString &host,
                             const uint16_t &port,
                             const HandlerUdpSocket::SocketType &socketType);

    // ThreadBase interface
protected:
    void onThreadStarted() override;
    void onThreadFinishing() override;
    void onThreadFinished() override;
    void onIdle() override;

    void onConnecting() override;
    void onConnected() override;
    void onDisconnected() override;
    void onError(const int errorCode) override;
    void onReadData(const char *data, const int &count) override;
    void onWriteData(const char *data, const int &count) override;
    bool onPacketReceived(const PacketBase::Ptr &) override;
    bool onPacketSent(const PacketBase::Ptr &) override;

private:
    HandlerUdpSocket::SocketType _socketType;
};
