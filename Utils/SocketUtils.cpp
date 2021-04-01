#include "SocketUtils.h"

#include <QString>

namespace Threader {

namespace Utils {


bool SocketUtils::_isSocketsInitialized = false;


bool SocketUtils::initializeSockets()
{
#ifdef Q_OS_WIN
    if (_isSocketsInitialized)
        return true;

    uint16_t version = WINSOCK_VERSION;
    WSADATA wsaData;
    auto result = WSAStartup(version, &wsaData);
    _isSocketsInitialized = (0 == result);
#endif

#ifdef Q_OS_LINUX
    _isSocketsInitialized = true;
#endif
    return _isSocketsInitialized;
}

void SocketUtils::finalizeSockets()
{
#ifdef Q_OS_WIN
   if (_isSocketsInitialized)
       WSACleanup();
#endif
}

bool SocketUtils::checkSocketsInitialization()
{
    if (_isSocketsInitialized)
        return true;
    return initializeSockets();
}

bool SocketUtils::isIpAddressString(const QString &ipAddressString)
{
    if (!checkSocketsInitialization())
        return false;

    auto addr = inet_addr(ipAddressString.toLocal8Bit().constData());
    return (addr != INADDR_NONE && addr != INADDR_ANY);
}

QString SocketUtils::sockAddrToString(const sockaddr_in &addr)
{
    return QString(inet_ntoa(addr.sin_addr));
}

sockaddr_in SocketUtils::ipAddressStringToAddress(const QString &ipAddress,
                                                  const uint16_t &port)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipAddress.toLocal8Bit().constData());
    addr.sin_port = port;
    return addr;
}

QString SocketUtils::hostToIpAddressString(const QString &host)
{
    QString result = host;
    if (!checkSocketsInitialization())
        return result;

#ifdef Q_OS_WIN

    auto entry = gethostbyname(host.toLocal8Bit().constData());
    if (!entry)
        return result;

    int i = 0;
    if (entry->h_addrtype == AF_INET)
    {
        struct in_addr addr;
        while (entry->h_addr_list[i] != 0) {
            addr.s_addr = *(u_long *) entry->h_addr_list[i++];
            result = QString(inet_ntoa(addr));
            break;
        }
    }

#endif

#ifdef Q_OS_LINUX
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    int errcode = getaddrinfo(host.toLocal8Bit().constData(), nullptr, &hints, &res);
    if (errcode != 0)
        return result;

    auto ptr = (reinterpret_cast<struct sockaddr_in*>(res->ai_addr))->sin_addr;
    result = QString(inet_ntoa(ptr));
    freeaddrinfo(res);

#endif
    return result;
}

int SocketUtils::lastError()
{
#ifdef Q_OS_WIN
    return WSAGetLastError();
#endif
#ifdef Q_OS_LINUX
    return errno;
#endif
}

void SocketUtils::closeSocket(Descriptor socket)
{
#ifdef Q_OS_WIN
    shutdown((SOCKET)socket, SD_BOTH);
    closesocket((SOCKET)socket);
#endif

#ifdef Q_OS_LINUX
    shutdown(socket, SHUT_RDWR);
    close(socket);
#endif
}

#ifdef Q_OS_WIN

Descriptor SocketUtils::createTcpServerSocket(uint16_t port,
                                              bool isBlocking,
                                              sockaddr_in &addr,
                                              int &errorCode)
{
    if (!checkSocketsInitialization())
        return (Descriptor)INVALID_SOCKET;

    errorCode = 0;

    // создание сокета
    Descriptor result = (Descriptor)WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
    if (INVALID_SOCKET == (SOCKET)result)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    // установка режима блокировки сокета
    u_long arg = (isBlocking) ? 0 : 1;
    if (ioctlsocket((SOCKET)result, FIONBIO, &arg) != 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    // привязка сокета к порту
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    if (bind((SOCKET)result, (struct sockaddr*)&addr, sizeof(addr)) != 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    // запуск прослушивания запросов на подключение
    if (listen((SOCKET)result, SOMAXCONN) != 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    return (Descriptor)result;
}

Descriptor SocketUtils::openTcpClientSocket(const QString &ipAddress,
                                            const uint16_t port,
                                            int &errorCode,
                                            const int timeout)
{
    if (!checkSocketsInitialization())
        return (Descriptor)INVALID_SOCKET;

    errorCode = 0;

    // создание сокета
    Descriptor result = (Descriptor)WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
    if (INVALID_SOCKET == (SOCKET)result)
    {
        errorCode = lastError();
        return INVALID_DESCRIPTOR;
    }

    // неблокирующий режим
    if (!setBlockingMode(result, false))
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    // привязка сокета к порту
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipAddress.toLocal8Bit().constData());
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    // подключение
    int operationResult = connect((SOCKET)result, (struct sockaddr*)&addr, sizeof(addr));
    if (SOCKET_ERROR == operationResult)
    {
        errorCode = lastError();
        if (errorCode != WSAEWOULDBLOCK)
        {
            closeSocket(result);
            return INVALID_DESCRIPTOR;
        }

        if (0 != timeout)
        {
            Sleep(timeout);
            fd_set setw, sete;

            FD_ZERO(&setw);
            FD_SET((SOCKET)result, &setw);

            FD_ZERO(&sete);
            FD_SET((SOCKET)result, &sete);

            select(0, nullptr, &setw, &sete, nullptr);
            if (!FD_ISSET((SOCKET)result, &setw))
            {
                errorCode = lastError();
                closeSocket(result);
                return INVALID_DESCRIPTOR;
            }
        }
    }

    return result;
}

int SocketUtils::inputBytesAvalable(const Descriptor descriptor)
{
    if (descriptor != INVALID_DESCRIPTOR)
    {
        u_long result;
        ioctlsocket((SOCKET)descriptor, FIONREAD, &result);
        return result;
    }
    else
      return 0;
}

#endif

#ifdef Q_OS_UNIX

Descriptor SocketUtils::createTcpServerSocket(uint16_t port,
                                              bool isBlocking,
                                              sockaddr_in &addr,
                                              int &errorCode)
{
    errorCode = 0;

    // создание сокета, принимающего подключения
    Descriptor result = socket(AF_INET, SOCK_STREAM, 0);
    if (result < 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    // установка признака повторного использования сокета
    int flag = 1;
    int callResult = setsockopt(result, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if (callResult < 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    // установка неблокирующего режима
    // все получаемые входящие сокеты тоже будут неблокирующие (ПИЗДЕЖЖЖ!!!)
    if (!isBlocking)
    {
        // перевод сокета в неблокирующий режим
        callResult = fcntl(result, F_SETFL, O_NONBLOCK);
        if (callResult != 0)
        {
            errorCode = lastError();
            closeSocket(result);
            return INVALID_DESCRIPTOR;
        }
    }

    // привязка сокета к порту
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    callResult = bind(result, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (callResult < 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    // запуск прослушивания запросов на подключение
    callResult = listen(result, 64);
    if (callResult < 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    return result;
}

Descriptor SocketUtils::openTcpClientSocket(const QString &ipAddress,
                                            const uint16_t port,
                                            int &errorCode,
                                            const int)
{
    struct addrinfo hints, *addr_result;
    memset (&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST;

    // получение информации об удаленном хосте
    int callResult = getaddrinfo(ipAddress.toLocal8Bit().constData(),
                                 QString::number(port).toLocal8Bit().constData(),
                                 &hints, &addr_result);
    if (callResult != 0)
    {
        errorCode = lastError();
        return INVALID_DESCRIPTOR;
    }

    // создание сокета
    Descriptor result = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (result < 0)
    {
        errorCode = lastError();
        freeaddrinfo(addr_result);
        return INVALID_DESCRIPTOR;
    }

    // перевод сокета в неблокирующий режим
    callResult = fcntl(result, F_SETFL, O_NONBLOCK);
    if (callResult != 0)
    {
        errorCode = lastError();
        closeSocket(result);
        freeaddrinfo(addr_result);
        return INVALID_DESCRIPTOR;
    }

    // установка таймаутов на время ожидания при передаче сообщения об ошибке
    timespec t;
    t.tv_sec = 5;
    t.tv_nsec = 0;
    callResult = setsockopt(result, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t));
    if (callResult != 0)
    {
        errorCode = lastError();
        closeSocket(result);
        freeaddrinfo(addr_result);
        return INVALID_DESCRIPTOR;
    }

    // установка таймаутов на время ожидания при приеме сообщения об ошибке
    callResult = setsockopt(result, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
    if (callResult != 0)
    {
        errorCode = lastError();
        closeSocket(result);
        freeaddrinfo(addr_result);
        return INVALID_DESCRIPTOR;
    }

    // установка количества повторных передач SYN, которое должен послать TCP
    // до отмены попытки установки соединения
    int32_t synCount = 3;
    callResult = setsockopt(result, IPPROTO_TCP, TCP_SYNCNT, &synCount, sizeof(synCount));
    if (callResult != 0)
    {
        errorCode = lastError();
        closeSocket(result);
        freeaddrinfo(addr_result);
        return INVALID_DESCRIPTOR;
    }

    callResult = connect(result, addr_result->ai_addr, addr_result->ai_addrlen);
    if (callResult == -1)
    {
        errorCode = lastError();
        if (errorCode != EINPROGRESS)
        {
            closeSocket(result);
            result = INVALID_DESCRIPTOR;
        }
        freeaddrinfo(addr_result);
        return result;
    }

    return result;
}

#endif

Descriptor SocketUtils::createUdpSocket(const uint16_t &port,
                                        sockaddr_in &addr,
                                        int &errorCode)
{
    errorCode= 0;
#ifdef Q_OS_WIN
    if (!checkSocketsInitialization())
        return (Descriptor)INVALID_SOCKET;
#endif

//    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));
//    addr.sin_family = AF_INET;
//    addr.sin_addr.s_addr = INADDR_ANY;
//    addr.sin_port = htons(port);

#ifdef Q_OS_WIN
    Descriptor result = (Descriptor)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
#ifdef Q_OS_LINUX
    Descriptor result = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif

    // NOTE: Проверить возвращаемые значения для Windows
    if (INVALID_DESCRIPTOR == result)
    {
        errorCode = lastError();
        return result;
    }

#ifdef Q_OS_WIN
    auto callResult = bind((SOCKET)result, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
#endif
#ifdef Q_OS_LINUX
    auto callResult = bind(result, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
#endif
    if (callResult < 0)
    {
        errorCode = lastError();
        closeSocket(result);
        return INVALID_DESCRIPTOR;
    }

    return result;

    Q_UNUSED(port)
}

#ifdef Q_OS_LINUX

int SocketUtils::inputBytesAvalable(const Descriptor descriptor)
{
    if (descriptor != INVALID_DESCRIPTOR)
    {
        int32_t result;
        auto callResult = ioctl(descriptor, FIONREAD, &result);
        return  (0 == callResult) ? result : 0;
    }
    else
      return 0;
}

#endif

bool SocketUtils::setBlockingMode(Descriptor socket, bool isBlocking)
{

#ifdef Q_OS_WIN
    u_long arg = (isBlocking) ? 0 : 1;
    return ioctlsocket((SOCKET)socket, FIONBIO, &arg) == 0;
#endif

#ifdef Q_OS_LINUX
    int arg = fcntl(socket, F_GETFL, 0);
    arg =(isBlocking)
            ? arg & ~O_NONBLOCK
            : arg | O_NONBLOCK;
    auto result = fcntl(socket, F_SETFL, arg);
    return 0 == result;
#endif
}

}}
