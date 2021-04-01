#pragma once

#include "PollerListenSocket.h"
#include "ThreadBase.h"

#include "../threader_global.h"

#include <QObject>

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT ThreadListenSocket : public ThreadBase
{
    Q_OBJECT
public:
    explicit ThreadListenSocket(uint16_t port,
                                IMessageSubscriber *parent = nullptr);
    ~ThreadListenSocket() override;
    uint16_t port() const;

protected:
    void onBeforeWaitEvents() override;
    void terminateChildThreads() override;

    virtual void onBeforeListenSocketInitialization() = 0;
    virtual void onListenSocketInitialized() = 0;
    virtual void onAcceptConnectionRequest(Descriptor socket,
                                           const QString &ipAddress,
                                           bool &accept) = 0;
    virtual void onListenSocketError(PollerListenSocket *sender,
                                     int errorCode) = 0;

private:
    uint16_t _port;
    PollerListenSocket *_listener;
    qint64 _nextStartListening;

    static const uint TIMEOUT_REOPEN_LISTEN_SOCKET_MILLISECONDS;

protected slots:
    void slotOnListenSocketError(PollerListenSocket *sender,
                                   int errorCode);

    void slotOnConnectionRequest(PollerListenSocket *sender,
                                 Descriptor socket,
                                 const sockaddr_in &addr,
                                 bool &accept);

};

}

}
