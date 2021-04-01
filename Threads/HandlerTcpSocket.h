#pragma once

#include "HandlerBase.h"

#include "../threader_global.h"

#include <QObject>

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT HandlerTcpSocket : public HandlerBase
{
public:
    enum class SocketType
    {
        Server,
        Client
    };

    explicit HandlerTcpSocket(const QString &host,
                              const uint16_t port,
                              const Descriptor descriptor = INVALID_DESCRIPTOR);
    ~HandlerTcpSocket() override;

    QString host() const;

    uint16_t port() const;

    QString ipAddress() const;

    int inputBytesAvailable() override;

    int outputBytesAvailable() override;

    bool open() override;

    void close() override;

#ifdef Q_OS_LINUX
    void assign(pollfd &event) override;

    bool process(const pollfd &event) override;
#endif


#ifdef Q_OS_WIN
    int read(char *data, int count) override;

    int write(const char *data, int count) override;

    DescriptorsVector *events() override;

    bool process(Descriptor eventToProcess) override;
#endif

private:
    QString _host;
    uint16_t _port;
    QString _ipAddress;
    SocketType _socketType;

#ifdef Q_OS_WIN
    void registerEvent();
    void unregisterEvent();
#endif
};


}}
