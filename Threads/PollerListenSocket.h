#pragma once

#include "ThreadsCommon.h"
#include "PollingsLinux.h"
#include "PollingsWindows.h"
#include "../threader_global.h"

#ifdef Q_OS_LINUX
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT PollerListenSocket : public PollerBase
{
    Q_OBJECT
public:
    explicit PollerListenSocket(uint16_t port);
    ~PollerListenSocket() override;

    bool isInitialized() const;

    uint16_t port() const;

    bool initialize();
    void finalize();

#ifdef Q_OS_WIN
    Descriptor descriptor() const;
    void setDescriptor(const Descriptor descriptor);
#endif

#ifdef Q_OS_LINUX
    bool process(const pollfd &event) override;
    int processConnectionRequests();
#endif

private:
    bool _isInitialized;
    uint16_t _port;

#ifdef Q_OS_WIN
    Descriptor _descriptor;
    Descriptor _event;

    void finalizeWindows();
    bool process(Descriptor eventToProcess) override;
#endif

#ifdef Q_OS_LINUX
    void finalizeLinux();
#endif


signals:
    void signalOnListenSocketError(PollerListenSocket *sender,
                                   int errorCode);

    void signalConnectionRequest(PollerListenSocket *sender,
                                 Descriptor socket,
                                 const sockaddr_in &addr,
                                 bool &accept);
};

}}
