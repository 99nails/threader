#include "ThreadListenSocket.h"

#include "../Utils/DateUtils.h"
#include "../Utils/SocketUtils.h"

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


const uint ThreadListenSocket::TIMEOUT_REOPEN_LISTEN_SOCKET_MILLISECONDS = 30000;


ThreadListenSocket::ThreadListenSocket(uint16_t port,
                                       IMessageSubscriber *parent)
    : ThreadBase(parent)
    , _port(port)
    , _listener(new PollerListenSocket(port))
    , _nextStartListening(DateUtils::getTickCount())
{
    _listener->moveToThread(this);
    connect(_listener, &PollerListenSocket::signalOnListenSocketError,
            this, &ThreadListenSocket::slotOnListenSocketError, Qt::DirectConnection);
    connect(_listener, &PollerListenSocket::signalConnectionRequest,
            this, &ThreadListenSocket::slotOnConnectionRequest, Qt::DirectConnection);
}

ThreadListenSocket::~ThreadListenSocket()
{
    delete _listener;
}

uint16_t ThreadListenSocket::port() const
{
    return _port;
}

void ThreadListenSocket::onBeforeWaitEvents()
{
    // проверка необходимости инициализации слушающего сокета
    if (_listener->isInitialized() || _nextStartListening > DateUtils::getTickCount())
        return;

    // вызов заглушки перед стартом инициализации
    onBeforeListenSocketInitialization();

    // инициализация слущающего сокета
    if (_listener->initialize())
    {
        // регистрация в списке голосующих
        polling()->registerPoller(_listener);
        // вызов заглушки корректной ининциализации
        onListenSocketInitialized();
    }
    else
    {
        // формирование времени следующего срабатывания ининциализации
        _nextStartListening = DateUtils::getNextTickCount(TIMEOUT_REOPEN_LISTEN_SOCKET_MILLISECONDS);
        // обработчик ошибок вызывается голосующим
    }

}

void ThreadListenSocket::terminateChildThreads()
{
    _listener->finalize();
    ThreadBase::terminateChildThreads();
}

void ThreadListenSocket::slotOnListenSocketError(PollerListenSocket *sender,
                                                 int errorCode)
{
    onListenSocketError(sender, errorCode);
}

void ThreadListenSocket::slotOnConnectionRequest(PollerListenSocket *sender,
                                                 Descriptor socket,
                                                 const sockaddr_in &addr,
                                                 bool &accept)
{
    QString ipAddress = SocketUtils::sockAddrToString(addr);
    onAcceptConnectionRequest(socket, ipAddress, accept);
    Q_UNUSED(sender)
}

}}
