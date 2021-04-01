#pragma once

#include "ThreadMainDaemon.h"

#include "HandlerUdpSocket.h"

#include <QObject>

using namespace Threader::Threads;

class ThreadMainUdp : public ThreadMainDaemon
{
    Q_OBJECT
public:
    explicit ThreadMainUdp();

    static void setSocketType(const HandlerUdpSocket::SocketType &socketType);

protected:
    void startChildThreads() override;

private:
    ThreadBase *_threadUdp;

    static HandlerUdpSocket::SocketType _socketType;
};
