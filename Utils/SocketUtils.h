#pragma once

#include "../Threads/ThreadsCommon.h"

#include <QtGlobal>

#ifdef Q_OS_LINUX

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#ifdef Q_OS_WIN

#include "windows.h"

#endif

#include "../threader_global.h"

namespace Threader {

namespace Utils {

class THREADERSHARED_EXPORT SocketUtils
{
public:
    static bool initializeSockets();
    static void finalizeSockets();
    static bool checkSocketsInitialization();

    static bool isIpAddressString(const QString &ipAddressString);

    static QString sockAddrToString(const sockaddr_in &addr);
    static sockaddr_in ipAddressStringToAddress(const QString &ipAddress,
                                                const uint16_t &port);

    static QString hostToIpAddressString(const QString &host);

    static int lastError();
    static void closeSocket(Descriptor socket);
    static Descriptor createTcpServerSocket(uint16_t port,
                                            bool isBlocking,
                                            sockaddr_in &addr,
                                            int &errorCode);

    static Descriptor openTcpClientSocket(const QString &ipAddress,
                                          const uint16_t port,
                                          int &errorCode,
                                          const int = 0);

    static Descriptor createUdpSocket(const uint16_t &port,
                                      sockaddr_in &addr,
                                      int &errorCode);

    static bool setBlockingMode(Descriptor socket,
                                bool isBlocking);

    static int inputBytesAvalable(const Descriptor descriptor);

private:
    static bool _isSocketsInitialized;
};

}}
