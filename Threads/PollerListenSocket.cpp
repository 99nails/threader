#include "PollerListenSocket.h"

#include "../Utils/SocketUtils.h"

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


PollerListenSocket::PollerListenSocket(uint16_t port)
    : PollerBase()
    , _isInitialized(false)
    , _port(port)
    #ifdef Q_OS_WIN
    , _descriptor(INVALID_DESCRIPTOR)
    , _event(INVALID_DESCRIPTOR)
    #endif
{
}

PollerListenSocket::~PollerListenSocket()
{

}

bool PollerListenSocket::isInitialized() const
{
    return _isInitialized;
}

uint16_t PollerListenSocket::port() const
{
    return _port;
}

bool PollerListenSocket::initialize()
{
    if (_isInitialized)
        return true;

    sockaddr_in addr;
    int errorCode;

    setDescriptor(SocketUtils::createTcpServerSocket(_port,
                                                     false,
                                                     addr,
                                                     errorCode));

    bool result = (errorCode == 0);
    if (!result)
    {
        emit signalOnListenSocketError(this, errorCode);
        _isInitialized = false;
        return false;
    }
#ifdef Q_OS_LINUX
    setEvents(POLLIN | POLLERR);
#endif
#ifdef Q_OS_WIN
    _event = WSACreateEvent();
    if (WSAEventSelect((SOCKET)_descriptor, _event, FD_ACCEPT) != 0)
    {
        emit signalOnListenSocketError(this, SocketUtils::lastError());
        _isInitialized = false;
        return false;
    }

    assign({_event});
#endif
    _isInitialized = true;
    return result;
}

void PollerListenSocket::finalize()
{
#ifdef Q_OS_WIN
    finalizeWindows();
#endif

#ifdef Q_OS_LINUX
    finalizeLinux();
#endif
    _isInitialized = false;
}

#ifdef Q_OS_WIN

Descriptor PollerListenSocket::descriptor() const
{
    return _descriptor;
}

void PollerListenSocket::setDescriptor(const Descriptor descriptor)
{
    _descriptor = descriptor;
}

void PollerListenSocket::finalizeWindows()
{
    if (INVALID_SOCKET == (SOCKET)_descriptor)
        return;

    closesocket((SOCKET)_descriptor);
    _descriptor = (Descriptor)INVALID_SOCKET;
}

bool PollerListenSocket::process(Descriptor eventToProcess)
{
    if (_event != eventToProcess)
        return false;

    ResetEvent(eventToProcess);

    sockaddr_in clientAddress;
    int clientAddressLength = sizeof(clientAddress);
    auto newSocket = WSAAccept((SOCKET)_descriptor,
                               (SOCKADDR*) &clientAddress,
                               &clientAddressLength,
                               nullptr, 0);

    if (INVALID_SOCKET == newSocket)
    {
        emit signalOnListenSocketError(this, SocketUtils::lastError());
        return true;
    }

    bool acceptConnection = true;
    emit signalConnectionRequest(this,
                                 (Descriptor)newSocket,
                                 (struct sockaddr_in)clientAddress,
                                 acceptConnection);
    if (!acceptConnection)
        SocketUtils::closeSocket((Descriptor)newSocket);
    return true;
}

#endif

#ifdef Q_OS_LINUX

void PollerListenSocket::finalizeLinux()
{
    if (descriptor() == INVALID_DESCRIPTOR)
        return;
    SocketUtils::closeSocket(descriptor());
    setDescriptor(INVALID_DESCRIPTOR);
    setEvents(0);
}

bool PollerListenSocket::process(const pollfd &event)
{
    if (descriptor() != event.fd)
        return false;

    if (event.revents & POLLERR)
        finalize();

    if (event.revents & POLLIN)
        processConnectionRequests();

    return true;
}

int PollerListenSocket::processConnectionRequests()
{
    int result = 0;
    Descriptor newSocket;
    sockaddr_in clientAddress;
    do
    {
        socklen_t clientAddressSize = sizeof(clientAddress);
        newSocket = accept(descriptor(), reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize);

        if (INVALID_DESCRIPTOR == newSocket)
        {
            int errorCode = SocketUtils::lastError();
            if (errorCode != EWOULDBLOCK)
            {
                result++;
                emit signalOnListenSocketError(this, errorCode);
            }
            break;
        }
        else
        {
            result++;
            bool acceptConnection = true;            

            SocketUtils::setBlockingMode(newSocket, false);

            emit signalConnectionRequest(this,
                                         newSocket,
                                         clientAddress,
                                         acceptConnection);
            if (!acceptConnection)
                SocketUtils::closeSocket(newSocket);
        }
    }
    while (newSocket != INVALID_DESCRIPTOR);

    return result;
}

#endif

}}
