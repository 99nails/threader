#pragma once

#include "../../Threads/PacketFactoryBase.h"

#include <QMap>

using namespace Threader::Threads;

using PacketIdType = uint32_t;
using PacketTypeType = uint16_t;

enum class SimplePacketType
{
    Ticket,
    Hello,
    Welcome,
    Close,
    Alive
};

#pragma pack(push, 1)

struct PacketSimpleHeader
{
    PacketSimpleHeader(uint32_t Length = 0,
                       uint16_t CheckSumm = 0,
                       PacketIdType PacketId = 1,
                       PacketTypeType PacketType = 0)
    {
        tag = 0xAA55;
        length = Length;
        checkSumm = CheckSumm;
        packetId = PacketId;
        packetType = PacketType;
    }

    uint16_t tag;
    uint32_t length;
    uint16_t checkSumm;
    PacketIdType packetId;
    PacketTypeType packetType;
};


struct PacketDataAlive
{
    PacketDataAlive(const qint64 &currentTime,
                    const qint64 &tickCount)
    {
        CurrentTime = currentTime;
        TickCount = tickCount;
    }

    qint64 CurrentTime;
    qint64 TickCount;
};

#pragma pack(pop)

using SimplePacketTypeMap = QMap<SimplePacketType, QString>;

class PacketSimple : public PacketBase
{
public:
    typedef std::shared_ptr<PacketSimple> Ptr;

public:
    explicit PacketSimple(const PacketSimpleHeader &header,
                          const char *data,
                          int size);

    PacketSimpleHeader header() const;
    void setHeader(const PacketSimpleHeader &header);

    PacketIdType packetId();

    PacketTypeType packetType();
    void setPacketType(PacketTypeType value);

    bool write(DataStream &stream) const override;
    bool write(DataStream *stream) const;

private:
    PacketSimpleHeader _header;

};

enum class PacketFactorySimpleResults
{
    HeaderNotFound,
    NotEnoughForHeader,
    NotEnoughForBody,
    BadCheckSumm
};


class PacketFactorySimple : public PacketFactoryBase
{
public:
    explicit PacketFactorySimple();

    PacketBase::Ptr tryExtractPacket(QByteArray &data) override;
    PacketBase::Ptr buildPacket(const QByteArray &data,
                                bool generatePacketId) override;
    PacketBase::Ptr buildPacket(const PacketTypeType packetType, const QByteArray &data);
    uint32_t generateNextPacketId();

    PacketBase::Ptr buildPacketHello(const QString &name);
    PacketBase::Ptr buildPacketWelcome(const QString &name);
    PacketBase::Ptr buildPacketAlive(const qint64 &currentTime,
                                     const qint64 &tickCount);

    QString packetTypeName(SimplePacketType type);
private:
    PacketIdType _packetId;
    DataStream _stream;

    SimplePacketTypeMap _packetTypesNames;
};
