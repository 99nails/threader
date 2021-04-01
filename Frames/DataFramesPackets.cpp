#include "DataFramesPackets.h"

#include "../Utils/CrcUtils.h"

namespace Threader {

namespace Frames {

DataFramesPacket::DataFramesPacket(const DataFramesPacketHeader &header,
                                   const char *data,
                                   const PacketSizeType size)
    : PacketBase(data, size)
    , _header(header)
    , _packetName("")
{
}

DataFramesPacket::DataFramesPacket(const DataFramesPacketHeader &header,
                                   const PacketSizeType size)
    : PacketBase(size)
    , _header(header)
    , _packetName("")
{
}

bool DataFramesPacket::write(DataStream &stream) const
{
    return stream.write(&_header, sizeof(_header))
            && stream.write(data()->constData(), data()->length());
}

DataFramesPacketHeader DataFramesPacket::header() const
{
    return _header;
}

void DataFramesPacket::setHeader(const DataFramesPacketHeader &header)
{
    _header = header;
}

PacketIdType DataFramesPacket::packetId()
{
    return _header.packetId;
}

void DataFramesPacket::setPacketId(const PacketIdType &packetId)
{
    _header.packetId = packetId;
}

PacketType DataFramesPacket::packetType()
{
    return _header.packetType;
}

void DataFramesPacket::setPacketType(PacketType packetType)
{
    _header.packetType = packetType;
}

PacketSizeType DataFramesPacket::size()
{
    return _header.length;
}

QString DataFramesPacket::packetName() const
{
    return _packetName;
}

void DataFramesPacket::setPacketName(const QString &packetName)
{
    _packetName = packetName;
}



QueueDataFramesPackets::QueueDataFramesPackets()
{
}

int QueueDataFramesPackets::enqueue(const DataFramesPacket::Ptr &packet)
{
    if (!packet)
        return -1;

    _packets.append(packet);
    return _packets.count();
}

DataFramesPacket::Ptr QueueDataFramesPackets::first()
{
    if (_packets.count())
    {
        auto result = _packets.at(0);
        return result;
    }
    return nullptr;
}

int QueueDataFramesPackets::applyTicket(const PacketIdType &packetId)
{
    int count = 0;
    for (int i = _packets.count() - 1; i >= 0; i--)
    {
        DataFramesPacket::Ptr packet = _packets.at(i);
        if (packet->packetId() == packetId)
        {
            _packets.remove(i);
            count++;
        }
    }
    return count;
}

void QueueDataFramesPackets::reset()
{
    for (auto packet: _packets)
        packet->setPacketId(0);
}

void QueueDataFramesPackets::clear()
{
    _packets.clear();
}

int QueueDataFramesPackets::count()
{
    return _packets.count();
}


DataFramesPacketFactory::DataFramesPacketFactory()
    : PacketFactoryBase()
    , _packetId(0)
{
}

PacketBase::Ptr DataFramesPacketFactory::tryExtractPacket(QByteArray &data)
{
    DataFramesPacketHeader header;
    uint dataCount = 0;

    // выделение заголовка пакета
    while (true)
    {
        // поиск начала пакета
        int headerPosition = data.indexOf("\0x5A\0x5A");
        if (headerPosition < 0)
        {
            setLastResult(int(DataFramesFactoryResults::HeaderNotFound));
            return nullptr;
        }

        // удаление мусора в начале данных
        if (headerPosition > 0)
            data.remove(0, headerPosition);

        dataCount = uint(data.count());

        // проверка возможности чтения заголовка
        if (dataCount < sizeof(DataFramesPacketHeader))
        {
            setLastResult(int(DataFramesFactoryResults::NotEnoughForHeader));
            return nullptr;
        }

        header = *(reinterpret_cast<DataFramesPacketHeader*>(const_cast<char*>(data.constData())));

        // проверка допустимых типов пакета
        if (header.packetType >= PacketType::MaximumValue)
        {
            // если тип пакета недопустимый, то повторный поиск заголовка
            data.remove(0, 1);
        } else
            // иначе разбор пакета продолжается
            break;
    }

    // проверка полного приема пакета
    auto headerSize = sizeof(header);
    if (dataCount < headerSize + header.length)
    {
        setLastResult(int(DataFramesFactoryResults::NotEnoughForBody));
        return nullptr;
    }

    // проверка контрольной суммы
    uchar* dataPointer = reinterpret_cast<uchar*>(const_cast<char*>(data.constData()))
            + sizeof(header);
    const auto receivedCrc = CrcUtils::Crc16(dataPointer,
                                             static_cast<int>(header.length));
    if (receivedCrc != header.checkSumm)
    {
        setLastResult(int(DataFramesFactoryResults::BadCheckSumm));
        return nullptr;
    }

    // всё прошло успешно, формирование пакета
    DataFramesPacket::Ptr result = std::make_shared<DataFramesPacket>(header, reinterpret_cast<char*>(dataPointer), header.length);
    data.remove(0, static_cast<int>(sizeof(header) + header.length));
    return result;
}

uint32_t DataFramesPacketFactory::generateNextPacketId()
{
    _packetId++;
    if (_packetId >= 0x1FFFFFFF)
        _packetId = 1;
    return _packetId;
}

PacketBase::Ptr DataFramesPacketFactory::buildPacket(const QByteArray &data,
                                                     bool generatePacketId)
{
    PacketIdType packetId = (generatePacketId) ? generateNextPacketId() : PACKET_ID_EMPTY;

    DataFramesPacketHeader header(PacketType::PacketNeedsTicket,
                                  static_cast<PacketSizeType>(data.length()),
                                  CrcUtils::Crc16(reinterpret_cast<uchar*>(const_cast<char*>(data.constData())), data.length()),
                                  packetId);

    return std::make_shared<DataFramesPacket>(header, data.constData(), data.length());
}

PacketBase::Ptr DataFramesPacketFactory::buildPacketFromFrame(DataFrame::Ptr frame,
                                                              bool generatePacketId)
{
    if (!frame || !frame->definition())
        return nullptr;

    PacketType packetType = (frame->definition()->needsTicket()) ? PacketType::PacketNeedsTicket : PacketType::PacketWithoutTicket;

    QByteArray data;
    DataStream stream(&data);
    frame->write(stream);

    DataFramesPacket::Ptr packet = std::dynamic_pointer_cast<DataFramesPacket>(buildPacket(data, generatePacketId));
    packet->setPacketType(packetType);
    return packet;
}

PacketBase::Ptr DataFramesPacketFactory::buildPacketTicket(PacketIdType &packetId)
{
    if (PACKET_ID_EMPTY == packetId)
        return nullptr;

    DataFramesPacketHeader header(PacketType::Ticket,
                                  sizeof(packetId),
                                  CrcUtils::Crc16(reinterpret_cast<uchar*>(&packetId), sizeof(packetId)),
                                  generateNextPacketId());

    return std::make_shared<DataFramesPacket>(header, reinterpret_cast<char*>(&packetId), (PacketSizeType)sizeof(packetId));
}

}}
