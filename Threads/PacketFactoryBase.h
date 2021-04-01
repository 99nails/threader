#pragma once

#include "../Utils/DataStream.h"
#include "../threader_global.h"

#include <QDateTime>

#include <memory>

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


class THREADERSHARED_EXPORT PacketBase
{
public:
    using Ptr = std::shared_ptr<PacketBase>;

public:
    explicit PacketBase(const char* data,
                        const int size);
    explicit PacketBase(int size);
    virtual ~PacketBase();

    QDateTime created() const;
    qint64 createdTickCount() const;

    QByteArray *data() const;
    void setData(const QByteArray& data);

    virtual bool write(DataStream &stream) const = 0;
private:
    QDateTime _createdUtc;
    qint64 _createdTickCount;
    QByteArray _data;
};

class THREADERSHARED_EXPORT PacketFactoryBase
{
public:
    explicit PacketFactoryBase() = default;
    virtual ~PacketFactoryBase() = default;
    virtual PacketBase::Ptr tryExtractPacket(QByteArray &) = 0;
    virtual PacketBase::Ptr buildPacket(const QByteArray &data,
                                        bool generatePacketId) = 0;
    virtual int lastResult() const;

protected:
    void setLastResult(int value);

private:
    int _lastResult;
};

}}
