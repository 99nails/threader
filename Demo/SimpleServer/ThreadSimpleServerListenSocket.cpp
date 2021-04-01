#include "ThreadSimpleServerListenSocket.h"

#include "ThreadSimpleServerSocket.h"

#include "../Common/Logs/LogMessagesTemplatesDemo.h"

ThreadSimpleServerListenSocket::ThreadSimpleServerListenSocket(ThreadBase *parent)
    : ThreadListenSocket(53817, parent)
{
    setThreadName("Thread.Listen:" + QString("%1").arg(port()));
}


void ThreadSimpleServerListenSocket::onThreadStarted()
{
    writeLog(nullptr, Message1000, port());
}

void ThreadSimpleServerListenSocket::onThreadFinishing()
{
    writeLog(nullptr, Message1001, port());
}

void ThreadSimpleServerListenSocket::onThreadFinished()
{
    writeLog(nullptr, Message1002, port());
}

void ThreadSimpleServerListenSocket::onBeforeListenSocketInitialization()
{
    writeLog(nullptr, Message1003, port());
}

void ThreadSimpleServerListenSocket::onListenSocketInitialized()
{
    writeLog(nullptr, Message1004, port());
}

void ThreadSimpleServerListenSocket::onAcceptConnectionRequest(Descriptor socket,
                                                               const QString &ipAddress,
                                                               bool &accept)
{
    writeLog(nullptr, Message1006,
             port(),
             ipAddress.toUtf8().constData());
    accept = true;

    ThreadBase *thread = new ThreadSimpleServerSocket(this,
                                                      socket,
                                                      ipAddress,
                                                      port());

    registerAndStartChildThread(thread);
}


void ThreadSimpleServerListenSocket::onListenSocketError(PollerListenSocket *sender,
                                                         int errorCode)
{
    writeLog(nullptr, Message1005,
             port(),
             errorCode,
             errorString(errorCode).toUtf8().constData());

    Q_UNUSED(sender)
}
