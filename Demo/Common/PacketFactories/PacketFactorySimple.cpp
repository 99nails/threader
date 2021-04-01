#include "PacketFactorySimple.h"

#include "../../../Utils/CrcUtils.h"

#include <memory>

using namespace Threader::Utils;

PacketSimple::PacketSimple(const PacketSimpleHeader &header,
                           const char *data, int size)
    : PacketBase(data, size)
    , _header(header)
{
}

PacketSimpleHeader PacketSimple::header() const
{
    return _header;
}

void PacketSimple::setHeader(const PacketSimpleHeader &header)
{
    _header = header;
}

PacketIdType PacketSimple::packetId()
{
    return _header.packetId;
}

PacketTypeType PacketSimple::packetType()
{
    return _header.packetType;
}

void PacketSimple::setPacketType(PacketTypeType value)
{
    _header.packetType = value;
}

bool PacketSimple::write(DataStream &stream) const
{
    return write(&stream);
}

bool PacketSimple::write(DataStream *stream) const
{
    return stream->write(&_header, sizeof(_header))
            && stream->write(data()->constData(), data()->length());

}

PacketFactorySimple::PacketFactorySimple()
    : PacketFactoryBase()
    , _packetId(0)
{
    _packetTypesNames = {
        {SimplePacketType::Ticket, "Ticket"},
        {SimplePacketType::Hello, "Hello"},
        {SimplePacketType::Welcome, "Welcome"},
        {SimplePacketType::Close, "Close"},
        {SimplePacketType::Alive, "Alive"},
    };
}


PacketBase::Ptr PacketFactorySimple::tryExtractPacket(QByteArray &data)
{
    // поиск начала пакета
    int headerPosition = data.indexOf(0x55);
    if (headerPosition < 0)
    {
        setLastResult(int(PacketFactorySimpleResults::HeaderNotFound));
        return nullptr;
    }

    // удаление мусора в начале данных
    if (headerPosition > 0)
        data.remove(0, headerPosition);

    uint dataCount = uint(data.count());

    // проверка возможности чтения заголовка
    if (dataCount < sizeof(PacketSimpleHeader))
    {
        setLastResult(int(PacketFactorySimpleResults::NotEnoughForHeader));
        return nullptr;
    }

    const PacketSimpleHeader header = *(reinterpret_cast<PacketSimpleHeader*>(const_cast<char*>(data.constData())));

    // проверка полного приема пакета
    auto headerSize = sizeof(header);
    if (dataCount < headerSize + header.length)
    {
        setLastResult(int(PacketFactorySimpleResults::NotEnoughForBody));
        return nullptr;
    }

    // проверка контрольной суммы
    uchar* dataPointer = reinterpret_cast<uchar*>(const_cast<char*>(data.constData())) + sizeof(header);
    const auto receivedCrc = CrcUtils::Crc16(0, dataPointer, static_cast<int>(header.length));
    if (receivedCrc != header.checkSumm)
    {
        setLastResult(int(PacketFactorySimpleResults::BadCheckSumm));
        return nullptr;
    }

    // всё прошло успешно, формирование пакета
    PacketSimple::Ptr result = std::make_shared<PacketSimple>(header, reinterpret_cast<char*>(dataPointer), header.length);
    data.remove(0, static_cast<int>(sizeof(header) + header.length));
    return std::move(result);
}

PacketBase::Ptr PacketFactorySimple::buildPacket(const QByteArray &data,
                                                 bool generatePacketId)
{
    PacketSimpleHeader header(static_cast<uint32_t>(data.length()),
                              CrcUtils::Crc16(0, reinterpret_cast<uchar*>(const_cast<char*>(data.constData())), data.length()),
                              generateNextPacketId());
    return std::make_shared<PacketSimple>(header, data.constData(), data.length());

    Q_UNUSED(generatePacketId)
}

PacketBase::Ptr PacketFactorySimple::buildPacket(const PacketTypeType packetType, const QByteArray &data)
{
    PacketSimpleHeader header(static_cast<uint32_t>(data.length()),
                              CrcUtils::Crc16(0, reinterpret_cast<uchar*>(const_cast<char*>(data.constData())), data.length()),
                              generateNextPacketId(),
                              packetType);
    return std::make_shared<PacketSimple>(header, data.constData(), data.length());
}

uint32_t PacketFactorySimple::generateNextPacketId()
{
    _packetId++;
    if (_packetId >= 0x1FFFFFFF)
        _packetId = 1;
    return _packetId;
}

PacketBase::Ptr PacketFactorySimple::buildPacketHello(const QString &name)
{
    QByteArray data;
    _stream.setData(&data);
    char zero = 0;

    _stream.write(name.toLocal8Bit().constData(), name.length());
    _stream.write(&zero, sizeof(zero));
    auto packet = buildPacket(PacketTypeType(SimplePacketType::Hello), data);

    _stream.setData(nullptr);
    return packet;
}

PacketBase::Ptr PacketFactorySimple::buildPacketWelcome(const QString &name)
{
    QByteArray data;
    _stream.setData(&data);

    _stream.write(name.toLocal8Bit().constData(), name.length());
    auto packet = buildPacket(PacketTypeType(SimplePacketType::Welcome), data);

    _stream.setData(nullptr);
    return packet;
}

PacketBase::Ptr PacketFactorySimple::buildPacketAlive(const qint64 &currentTime,
                                                      const qint64 &tickCount)
{
    PacketDataAlive alive(currentTime, tickCount);

    QByteArray data;
    _stream.setData(&data);

    _stream.write(&alive, sizeof(alive));
    auto packet = buildPacket(PacketTypeType(SimplePacketType::Alive), data);
    _stream.setData(nullptr);

    return packet;
}

QString PacketFactorySimple::packetTypeName(SimplePacketType type)
{
    if (_packetTypesNames.contains(type))
        return _packetTypesNames[type];
    else
        return "";
}
