#pragma once

#include "../threader_global.h"

#include "../Frames/DataFrames.h"

#include "../Threads/PacketFactoryBase.h"

#include <QMap>
#include <QVector>

namespace Threader {

namespace Frames {


using namespace Threader::Threads;


enum class PacketType : uint8_t
{
    PacketWithoutTicket = 0,
    PacketNeedsTicket = 1,
    Ticket = 2,
    MaximumValue
};


using PacketIdType = uint32_t;
using PacketSizeType = uint32_t;

const uint16_t DATAFRAMES_PACKET_TAG = 0x5A5A;
const QByteArray DATAFRAMES_PACKET_TAG_ARRAY = {0x5A, 0x5A};

const PacketIdType PACKET_ID_EMPTY = 0;

#pragma pack(push, 1)


struct THREADERSHARED_EXPORT DataFramesPacketHeader
{
    DataFramesPacketHeader(PacketType _packetType = PacketType::PacketNeedsTicket,
                           PacketSizeType Length = 0,
                           uint16_t CheckSumm = 0,
                           PacketIdType PacketId = 1)
    {
        tag = DATAFRAMES_PACKET_TAG;        
        packetType = _packetType;
        length = Length;
        checkSumm = CheckSumm;
        packetId = PacketId;
    }

    uint16_t tag;
    PacketType packetType;
    PacketSizeType length;
    uint16_t checkSumm;
    PacketIdType packetId;
};

#pragma pack(pop)

class THREADERSHARED_EXPORT DataFramesPacket : public PacketBase
{
public:
    using Ptr = std::shared_ptr<DataFramesPacket>;

public:
    explicit DataFramesPacket(const DataFramesPacketHeader &header,
                              const char *data,
                              const PacketSizeType size);

    explicit DataFramesPacket(const DataFramesPacketHeader &header,
                              const PacketSizeType size);

    bool write(DataStream &stream) const override;

    DataFramesPacketHeader header() const;
    void setHeader(const DataFramesPacketHeader &header);

    PacketIdType packetId();
    void setPacketId(const PacketIdType &packetId);

    PacketType packetType();
    void setPacketType(PacketType packetType);

    PacketSizeType size();

    QString packetName() const;
    void setPacketName(const QString &packetName);

private:
    DataFramesPacketHeader _header;
    QString _packetName;
};

using VectorDataFramesPacket = QVector<DataFramesPacket::Ptr>;

class THREADERSHARED_EXPORT QueueDataFramesPackets
{
public:
    explicit QueueDataFramesPackets();

    int enqueue(const DataFramesPacket::Ptr &packet);

    DataFramesPacket::Ptr first();

    int applyTicket(const PacketIdType &packetId);

    void reset();

    void clear();

    int count();
private:
    VectorDataFramesPacket _packets;
};

enum class DataFramesFactoryResults
{
    HeaderNotFound,
    NotEnoughForHeader,
    NotEnoughForBody,
    BadCheckSumm
};

class THREADERSHARED_EXPORT DataFramesPacketFactory : public PacketFactoryBase
{
public:
    explicit DataFramesPacketFactory();

    PacketBase::Ptr tryExtractPacket(QByteArray &data) override;

    uint32_t generateNextPacketId();

    PacketBase::Ptr buildPacket(const QByteArray &data,
                                bool generatePacketId) override;

    PacketBase::Ptr buildPacketFromFrame(DataFrame::Ptr frame, bool generatePacketId);

    PacketBase::Ptr buildPacketTicket(PacketIdType &packetId);

private:
    PacketIdType _packetId;
    DataStream _stream;
};

}}
