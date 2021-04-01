#pragma once

#include "ThreadListenSocket.h"

#include <QObject>

using namespace Threader::Threads;

class ThreadSimpleServerListenSocket : public ThreadListenSocket
{
    Q_OBJECT
public:
    explicit ThreadSimpleServerListenSocket(ThreadBase *parent = nullptr);

protected:
    void onThreadStarted() override;
    void onThreadFinishing() override;
    void onThreadFinished() override;

    void onBeforeListenSocketInitialization() override;
    void onListenSocketInitialized() override;
    void onAcceptConnectionRequest(Descriptor socket,
                                   const QString &ipAddress,
                                   bool &accept) override;
    void onListenSocketError(PollerListenSocket *sender, int errorCode) override;
};
