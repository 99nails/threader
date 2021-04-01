#include "HandlerBase.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

namespace Threader {

namespace Threads {

HandlerBase::HandlerBase(const QString &deviceName,
                         const Descriptor descriptor)
#ifdef Q_OS_WIN
    : PollerBase()
    #endif
    #ifdef Q_OS_LINUX
    : PollerBase(descriptor)
    #endif
    , _deviceName(deviceName)
    , _connectionState(ConnectionState::Disconnected)
    , _needsToWrite(false)
    #ifdef Q_OS_WIN
    , _descriptor(descriptor)
    #endif
{
    setConnectionState((INVALID_DESCRIPTOR == this->descriptor())
            ? ConnectionState::Disconnected
            : ConnectionState::Connected);
}

#ifdef Q_OS_WIN

Descriptor HandlerBase::descriptor() const
{
    return _descriptor;
}

#endif

void HandlerBase::setDescriptor(const Descriptor &value)
{
#ifdef Q_OS_WIN
    _descriptor = value;
#endif

#ifdef Q_OS_LINUX
    PollerBase::setDescriptor(value);
#endif

    if (INVALID_DESCRIPTOR == descriptor())
    {
        setConnectionState(ConnectionState::Disconnected);
    }
    else if (ConnectionState::Connecting != _connectionState)
    {
        setConnectionState(ConnectionState::Connected);
    }
}

HandlerBase::ConnectionState HandlerBase::connectionState()
{
    return _connectionState;
}

bool HandlerBase::isConnected()
{
    return ConnectionState::Connected == _connectionState;
}

QString HandlerBase::deviceName() const
{
    return _deviceName;
}

void HandlerBase::setDeviceName(const QString &deviceName)
{
    if (ConnectionState::Disconnected != _connectionState)
        close();
    _deviceName = deviceName;
}

bool HandlerBase::needsToWrite() const
{
    return _needsToWrite;
}

void HandlerBase::setNeedsToWrite(const bool needsToWrite)
{
    _needsToWrite = needsToWrite;
}

#ifdef Q_OS_LINUX
int HandlerBase::read(char *data, int count)
{
    return int(::read(descriptor(), data, count));
}

int HandlerBase::write(const char *data, int count)
{
    return int(::write(descriptor(), data, count));
}

#endif

QString HandlerBase::readString()
{
    QString result = "";
    auto bufferSize = inputBytesAvailable();
    if (bufferSize > 0)
    {
        auto data = new char[bufferSize + 1];
        auto readCount = read(data, bufferSize);
        if (readCount)
        {
            data[readCount] = 0;
            result = QString(data);
        }
        delete[] data;
    }
    return result;
}

int HandlerBase::writeString(const QString &data)
{
    return int(write(data.toLocal8Bit().constData(), data.length()));
}

QByteArray HandlerBase::readBytes()
{
    QByteArray result = {};

    auto bufferSize = inputBytesAvailable();
    if (bufferSize > 0)
    {
        auto data = new char[bufferSize];
        auto readCount = read(data, bufferSize);
        if (readCount)
        {
            result.append(data, readCount);
        }
        delete[] data;
    }
    return result;
}

int HandlerBase::writeBytes(const QByteArray &data)
{
    return write(data.constData(), data.length());
}

void HandlerBase::setConnectionState(HandlerBase::ConnectionState value)
{
    if (ConnectionState::Disconnected == value && descriptor() != INVALID_DESCRIPTOR)
        close();

    if (value != _connectionState)
    {
        auto oldState = _connectionState;
        _connectionState = value;
        emit signalOnConnectionStateChanged(this, oldState,  _connectionState);
    }
}

void HandlerBase::doReadyRead()
{
    emit signalOnReadyRead(this);
}

void HandlerBase::doReadyWrite()
{
    emit signalOnReadyWrite(this);
}

void HandlerBase::doError(int errorCode)
{
    emit signalOnError(this, errorCode);
}

}}
