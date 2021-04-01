#include "PacketFactoryBase.h"

#include "../Utils/DateUtils.h"


namespace Threader {

namespace Threads {


PacketBase::PacketBase(const char *data,
                       const int size)
    : _createdUtc(QDateTime::currentDateTimeUtc())
    , _createdTickCount(DateUtils::getTickCount())
{
    _data = QByteArray(data, size);
}

PacketBase::PacketBase(int size)
    : _createdUtc(QDateTime::currentDateTimeUtc())
    , _createdTickCount(DateUtils::getTickCount())
    , _data(QByteArray(size, char(0)))
{
}

PacketBase::~PacketBase()
{
}

QDateTime PacketBase::created() const
{
    return _createdUtc;
}

qint64 PacketBase::createdTickCount() const
{
    return _createdTickCount;
}

QByteArray* PacketBase::data() const
{
    return const_cast<QByteArray*>(&_data);
}

void PacketBase::setData(const QByteArray &data)
{
    _data.clear();
    _data.append(data);
}

void PacketFactoryBase::setLastResult(int value)
{
    _lastResult = value;
}

int PacketFactoryBase::lastResult() const
{
    return 0;
}

}}
