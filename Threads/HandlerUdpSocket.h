#pragma once

#include "HandlerBase.h"

#include "../threader_global.h"

#include <QObject>

#ifdef Q_OS_LINUX
#include <netinet/in.h>
#endif

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT HandlerUdpSocket : public HandlerBase
{
    Q_OBJECT
public:
    enum class SocketType
    {
        Listener,
        Sender
    };

    explicit HandlerUdpSocket(const QString &host,
                              const uint16_t port);

    QString host() const;

    uint16_t port() const;

    QString ipAddress() const;

    SocketType socketType() const;

    bool open() override;

    void close() override;

    int inputBytesAvailable() override;

    int outputBytesAvailable() override;

#ifdef Q_OS_LINUX
    void assign(pollfd &event) override;

    bool process(const pollfd &event) override;
#endif

#ifdef Q_OS_WIN
    DescriptorsVector *events() override;

    bool process(Descriptor eventToProcess) override;
#endif

    int read(char *data, int count) override;

    int write(const char *data, int count) override;

    sockaddr_in remoteAddress() const;

private:
    QString _host;
    uint16_t _port;
    QString _ipAddress;
    SocketType _socketType;
    sockaddr_in _localAddress;
    sockaddr_in _remoteAddress;

#ifdef Q_OS_WIN
    void registerEvent();
    void unregisterEvent();
#endif
};

}}
