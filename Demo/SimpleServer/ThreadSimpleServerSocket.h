#pragma once

#include "../Common/PacketFactories/PacketFactorySimple.h"

#include "ThreadHandler.h"

#include <QObject>

using namespace Threader::Threads;

class ThreadSimpleServerSocket : public ThreadHandler
{
public:
    explicit ThreadSimpleServerSocket(IMessageSubscriber *parent,
                                      Descriptor socket,
                                      const QString host,
                                      const uint16_t port);

protected:
    void onThreadStarted() override;
    void onThreadFinishing() override;
    void onThreadFinished() override;
    void onIdle() override;

    void onConnecting() override;
    void onConnected() override;
    void onDisconnected() override;
    void onError(const int errorCode) override;

    void onReadData(const char *, const int &size) override;
    void onWriteData(const char *, const int &size) override;

    bool onPacketReceived(const PacketBase::Ptr &packet) override;
    bool onPacketSent(const PacketBase::Ptr &packet) override;

private:
    void doPacketHello(PacketSimple::Ptr packet);
    void doPacketAlive(PacketSimple::Ptr packet);

    void sendPacketWelcome(const QString &name);
    void sendPacketAlive();
};
