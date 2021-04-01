#include "HandlerTcpSocket.h"

#include "../Utils/SocketUtils.h"

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


HandlerTcpSocket::HandlerTcpSocket(const QString &host,
                                   const uint16_t port,
                                   const Descriptor descriptor)
    : HandlerBase(host + ":" + QString::number(port), descriptor)
    , _host(host)
    , _port(port)
{
    _ipAddress = _host;
    if (!SocketUtils::isIpAddressString(_host))
        _ipAddress = SocketUtils::hostToIpAddressString(_host);

    _socketType = (INVALID_DESCRIPTOR == descriptor)
            ? SocketType::Client : SocketType::Server;

#ifdef Q_OS_WIN
    registerEvent();
#endif
    if (INVALID_DESCRIPTOR != descriptor)
    {
        SocketUtils::setBlockingMode(descriptor, false);
        setConnectionState(ConnectionState::Connected);
    }
}

HandlerTcpSocket::~HandlerTcpSocket()
{
    if (ConnectionState::Disconnected != connectionState())
        close();
}

QString HandlerTcpSocket::host() const
{
    return _host;
}

uint16_t HandlerTcpSocket::port() const
{
    return _port;
}

QString HandlerTcpSocket::ipAddress() const
{
    return _ipAddress;
}

int HandlerTcpSocket::inputBytesAvailable()
{
    return SocketUtils::inputBytesAvalable(descriptor());
}

int HandlerTcpSocket::outputBytesAvailable()
{
    return -1;
}

#ifdef Q_OS_LINUX

bool HandlerTcpSocket::open()
{
    if (SocketType::Client != _socketType)
        return false;

    setConnectionState(ConnectionState::Connecting);

    int errorCode;
    auto descriptor = SocketUtils::openTcpClientSocket(_ipAddress, _port, errorCode);
    if (errorCode != 0 && errorCode != EINPROGRESS)
    {
        doError(errorCode);
    }
    else
    {
        setDescriptor(descriptor);
    }
    return  descriptor != INVALID_DESCRIPTOR;
}

void HandlerTcpSocket::close()
{
    Descriptor aDescriptor = descriptor();
    if (INVALID_DESCRIPTOR == aDescriptor)
        return;

    SocketUtils::closeSocket(aDescriptor);
    setDescriptor(INVALID_DESCRIPTOR);
}


void HandlerTcpSocket::assign(pollfd &event)
{
    auto cs = connectionState();
    if (cs == ConnectionState::Disconnected)
        return;

    event.fd = descriptor();
    event.revents = 0;
    event.events = POLLIN | POLLERR | POLLHUP | POLLNVAL | POLLRDHUP;
    if (needsToWrite() || ConnectionState::Connecting == cs)
        event.events |= POLLOUT;
}

bool HandlerTcpSocket::process(const pollfd &event)
{
    auto d = descriptor();

    if (ConnectionState::Disconnected == connectionState()
            || INVALID_DESCRIPTOR == d)
        return false;

    if (event.fd != d)
        return false;

    // если произошла ошибка, то её нужно отработать в первую очередь
    // пока всё ещё живо
    if (event.revents & (POLLERR | POLLNVAL))
    {
        doError(SocketUtils::lastError());
        close();
    }

    // следующим нужно проверять закрытие удаленной стороной
    if (event.revents & (POLLHUP | POLLRDHUP))
    {
        // прочитать всё, что есть на входе
        doReadyRead();
        // закрыть подключение
        close();
        // с вещами на выход
        return true;
    }

    // если есть возможность записать или произошло подключение
    if (event.revents & POLLOUT)
    {
        // обработка события подключения
        if (connectionState() == ConnectionState::Connecting)
        {

            // чтение готовности сокета, проверка ошибки отложенного открытия
            int32_t error = 0;
            uint32_t errorLength = sizeof(error);
            if (getsockopt(d, SOL_SOCKET, SO_ERROR, &error, &errorLength) == 0)
            {
                // проверка готовности сокета
                if (error == 0)
                {
                    setConnectionState(ConnectionState::Connected);
                    uint32_t aliveTimeout = 10000;
                    setsockopt(d, SOL_TCP, TCP_USER_TIMEOUT, &aliveTimeout, sizeof(aliveTimeout));
                }
                else
                {
                    doError(SocketUtils::lastError());
                    close();
                    return false;
                };
            }
            else
            {
                doError(SocketUtils::lastError());
                close();
                return false;
            }
        }

        // есть возможность отправить данные
        doReadyWrite();
    }

    // если есть что прочитать
    if (event.revents & POLLIN)
        doReadyRead();

    return true;
}

#endif

#ifdef Q_OS_WIN

bool HandlerTcpSocket::open()
{
    if (SocketType::Client != _socketType)
        return false;

    setConnectionState(ConnectionState::Connecting);

    int errorCode;
    Descriptor _descriptor = SocketUtils::openTcpClientSocket(_ipAddress, _port, errorCode);

    if (errorCode !=0 && errorCode != WSAEWOULDBLOCK)
    {
        doError(errorCode);
        setConnectionState(ConnectionState::Disconnected);
    }
    else
    {
        setDescriptor(_descriptor);
        registerEvent();
    }

    return _descriptor != INVALID_DESCRIPTOR;
}

void HandlerTcpSocket::close()
{
    Descriptor aDescriptor = descriptor();
    if (INVALID_DESCRIPTOR == aDescriptor)
        return;

    unregisterEvent();

    SocketUtils::closeSocket(aDescriptor);
    setDescriptor(INVALID_DESCRIPTOR);
}

int HandlerTcpSocket::read(char *data, int count)
{
    SOCKET socket = (SOCKET)descriptor();
    if (socket == (SOCKET)INVALID_DESCRIPTOR)
        return 0;

    return recv(socket, (char*)data, count, 0);
}

int HandlerTcpSocket::write(const char *data, int count)
{
    SOCKET socket = (SOCKET)descriptor();
    if (socket == (SOCKET)INVALID_DESCRIPTOR)
        return 0;

    return send(socket, (char*)data, count, 0);
}

/*
ВАЖНО! Тонкость работы сокетов Windows в том, что событие FD_WRITE приходит в двух случаях:
1. При открытии сокета;
2. В ситуации когда при записи количество записываемызх в сокет данных превышает размер исходящего буфера.
Поэтому перед вызовом ожидания события производится запись данных в сокет и при невозможности
записи поднимается флаг ожидания события FD_WRITE
*/

DescriptorsVector *HandlerTcpSocket::events()
{
    auto eventsVector = HandlerBase::events();

    if (descriptor() == INVALID_DESCRIPTOR || eventsVector->count() < 1)
        return eventsVector;

    // событие закрытия сокета принимается всегда
    long mask = FD_CLOSE;

    // если сокет подключен и
    if (isConnected())
    {
        // включение ожидания получения данных
        mask |= FD_READ;
        // если есть данные на отправку
        if (needsToWrite())
        {
            mask |= FD_WRITE;
            // вызов события готовности к отправке данных
            doReadyWrite();
        }
    }
    else
    {
        // если клиентский сокет не подключен, то включение ожидания подключения
        if (SocketType::Client == _socketType)
            mask |= FD_CONNECT;
    }

    // установка маски на событие сокета
    int operationResult = WSAEventSelect((SOCKET)descriptor(), eventsVector->at(0), mask);
    if (operationResult != 0)
        doError(SocketUtils::lastError());

    return eventsVector;
}

bool HandlerTcpSocket::process(Descriptor eventToProcess)
{
    auto eventsVector = events();
    auto socket = (SOCKET)descriptor();
    if (socket == (SOCKET)INVALID_DESCRIPTOR || eventsVector->count() < 1 ||
            eventsVector->at(0) != eventToProcess)
        return false;

    ResetEvent(eventToProcess);

    WSANETWORKEVENTS NetworkEvents;
    if (WSAEnumNetworkEvents(socket, eventToProcess, &NetworkEvents) != 0)
    {
        doError(SocketUtils::lastError());
        return false;
    }

    // событие подключения
    if (NetworkEvents.lNetworkEvents & FD_CONNECT)
    {
        // если подключение установлено
        if (NetworkEvents.iErrorCode[FD_CONNECT_BIT] == 0)
        {
            // установка состояния подключения - подключено
            // событие изменения состояния вызовется автоматически
            setConnectionState(ConnectionState::Connected);
        }
        else
        {
            // Произошла ошибка. Вызов события обработки ошибки
            doError(NetworkEvents.iErrorCode[FD_CONNECT_BIT]);
        }
    }

    // событие прихода данных
    if (NetworkEvents.lNetworkEvents & FD_READ)
    {
        if (NetworkEvents.iErrorCode[FD_READ_BIT] == 0)
        {
            // вызов события прихода данных
            doReadyRead();;
        }
        else
        {
            // Произошла ошибка. Вызов события обработки ошибки
            doError(NetworkEvents.iErrorCode[FD_READ_BIT]);
        }
    }

    // событие готовности к отправке данных
    if (NetworkEvents.lNetworkEvents & FD_WRITE)
    {
        if (NetworkEvents.iErrorCode[FD_WRITE_BIT] == 0)
        {
            // вызов события готовности к отправке данных
            doReadyWrite();
        }
        else {
            // Произошла ошибка. Вызов события обработки ошибки
            doError(NetworkEvents.iErrorCode[FD_WRITE_BIT]);
        }
    }

    // событие закрытия подключения удаленным хостом
    if (NetworkEvents.lNetworkEvents & FD_CLOSE)
    {
        if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] == 0)
        {
            // установка состояния подключения - отключено
            // событие изменения состояния вызовется автоматически
            setConnectionState(ConnectionState::Disconnected);
        }
        else
        {
            // Произошла ошибка. Вызов события обработки ошибки
            doError(NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
        };
    };

    //    ResetEvent(eventToProcess);

    return true;
}

void HandlerTcpSocket::registerEvent()
{
    unregisterEvent();

    assign({WSACreateEvent()});
}

void HandlerTcpSocket::unregisterEvent()
{
    auto eventsArray = events();
    for (auto e : *eventsArray)
    {
        WSACloseEvent(e);
    }
    assign({});
}

#endif

}}
