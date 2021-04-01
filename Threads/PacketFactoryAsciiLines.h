#pragma once

#include "PacketFactoryBase.h"

namespace Threader {

namespace Threads {


class THREADERSHARED_EXPORT PacketAsciiLines : public PacketBase
{
public:
    using Ptr = std::shared_ptr<PacketAsciiLines>;

public:
    explicit PacketAsciiLines(const QStringList &lines);
    virtual ~PacketAsciiLines() = default;

    bool write(DataStream &stream) const override;

    QStringList lines() const;

protected:
    QStringList _lines;

};


class THREADERSHARED_EXPORT PacketFactoryAsciiLines : public PacketFactoryBase
{
public:
    static const QString CRLF;

    explicit PacketFactoryAsciiLines();

    PacketBase::Ptr tryExtractPacket(QByteArray &data) override;
    PacketBase::Ptr buildPacket(const QByteArray &data,
                                bool generatePacketId = false) override;
};

}}
