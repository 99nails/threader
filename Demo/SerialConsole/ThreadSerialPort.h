#pragma once

#include "ThreadHandler.h"

#include <QObject>

using namespace Threader::Threads;

class ThreadSerialPort : public ThreadHandler
{
    Q_OBJECT
public:
    explicit ThreadSerialPort(IMessageSubscriber *parent,
                              const QString &deviceName);

protected:
    void onThreadStarted() override;
    void onThreadFinishing() override;
    void onThreadFinished() override;
    void onIdle() override;

    void onConnecting() override;
    void onConnected() override;
    void onDisconnected() override;
    void onError(const int errorCode) override;
    bool onPacketReceived(const PacketBase::Ptr &) override;
    bool onPacketSent(const PacketBase::Ptr &) override;
    void onReadData(const char *data, const int &count) override;
    void onWriteData(const char *data, const int &) override;
};
