#include "PacketFactoryAsciiLines.h"

namespace Threader {

namespace Threads {


PacketAsciiLines::PacketAsciiLines(const QStringList &lines)
    : PacketBase(0)
    , _lines(lines)
{
}

QStringList PacketAsciiLines::lines() const
{
    return _lines;
}

bool PacketAsciiLines::write(DataStream &stream) const
{
    QString data = "";
    for (auto line : _lines)
        data += line + PacketFactoryAsciiLines::CRLF;
    return stream.writeString(data);
}


const QString PacketFactoryAsciiLines::CRLF = "\r\n";

PacketFactoryAsciiLines::PacketFactoryAsciiLines()
    : PacketFactoryBase()
{
}

PacketBase::Ptr PacketFactoryAsciiLines::tryExtractPacket(QByteArray &data)
{
    if (data.size() <= 0)
        return nullptr;

    int conveyorPosition = 0;
    int conveyorLength = data.size();
    int deleteToPosition = -1;
    QString currentLine = "";
    QStringList lines;

    while (conveyorPosition < conveyorLength) {
        char currentChar = data.at(conveyorPosition);

        if (CRLF.indexOf(currentChar) < 0) {
            currentLine.append(currentChar);
        }
        else
        {
            if (!currentLine.isEmpty()) {
                lines.append(currentLine);
                currentLine.clear();
            }
            deleteToPosition = conveyorPosition;
        }

        conveyorPosition++;
    }

    if (deleteToPosition >= 0) {
        data.remove(0, deleteToPosition + 1);
    }

    if (lines.count() > 0) {
        return std::make_shared<PacketAsciiLines>(lines);
    }

    return nullptr;
}

PacketBase::Ptr PacketFactoryAsciiLines::buildPacket(const QByteArray &data,
                                                     bool generatePacketId)
{
    QString stringData = QString(data);
    return std::make_shared<PacketAsciiLines>(stringData.split(CRLF));

    Q_UNUSED(generatePacketId)
}


}}
