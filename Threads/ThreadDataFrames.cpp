#include "ThreadDataFrames.h"

#include "HandlerBase.h"
#include "HandlerTcpSocket.h"

#include "../Utils/DateUtils.h"

namespace Threader {

namespace Threads {

using namespace Threader::Utils;

ThreadDataFrames::ThreadDataFrames(IMessageSubscriber *parent,
                                   const Descriptor &socket,
                                   const QString &host,
                                   const int &port,
                                   const bool &useQueue)
    : ThreadHandler(parent,
                    new HandlerTcpSocket(host, port, socket),
                    0,
                    new DataFramesPacketFactory())
    , _currentPacket(nullptr)
    , _queuePackets(nullptr)
{
    if (useQueue)
        _queuePackets = new QueueDataFramesPackets();
    _dataFramesPacketFactory = dynamic_cast<DataFramesPacketFactory*>(packetFactory());
}

ThreadDataFrames::ThreadDataFrames(IMessageSubscriber *parent,
                                   const QString &host,
                                   const int &port,
                                   const uint &reconnectTimeout, const bool &useQueue)
    : ThreadHandler(parent,
                    new HandlerTcpSocket(host, port),
                    reconnectTimeout,
                    new DataFramesPacketFactory())
    , _currentPacket(nullptr)
    , _queuePackets(nullptr)
{
    if (useQueue)
        _queuePackets = new QueueDataFramesPackets();
    _dataFramesPacketFactory = dynamic_cast<DataFramesPacketFactory*>(packetFactory());
}

ThreadDataFrames::~ThreadDataFrames()
{
    if (_queuePackets)
    {
        delete _queuePackets;
        _queuePackets = nullptr;
    }
}

bool ThreadDataFrames::isAuthorized() const
{
    return _isAuthorized;
}

void ThreadDataFrames::setIsAuthorized(bool isAuthorized)
{
    _isAuthorized = isAuthorized;
}

void ThreadDataFrames::onConnected()
{
    _nextPacketId = -1;
}

bool ThreadDataFrames::onPacketReceived(const PacketBase::Ptr &packet)
{
    DataFramesPacket::Ptr dataFramesPacket = std::dynamic_pointer_cast<DataFramesPacket>(packet);
    if (!dataFramesPacket)
        return false;

    DataFramesPacketHeader header = dataFramesPacket->header();
    PacketIdType packetId = header.packetId;

    // ???????????????? ???????????? ????????????
    if (_nextPacketId >= 0 && _nextPacketId != packetId)
    {
        onPacketIdDiscontinuity(_nextPacketId, packetId);
    }
    _nextPacketId = packetId + 1;

    switch (header.packetType) {

    case PacketType::PacketNeedsTicket:
    case PacketType::PacketWithoutTicket:
    {
        // ???????????????? ??????????????????
        auto packetTicket = _dataFramesPacketFactory->buildPacketTicket(packetId);
        sendPacket(packetTicket);

        if (PacketType::PacketWithoutTicket == header.packetType)
        {
            // ???????????????????????????? ??????????????
            auto dataFramesFactory = framesFactory();
            if (!dataFramesFactory)
                return false;

            auto frames = dataFramesFactory->readFrames(packet->data());
            if (frames.count() <= 0)
                return false;

            onFrameReceived(frames, packetId);
        }
        return true;
    }

    case PacketType::Ticket:
    {
        PacketIdType packetId = 0;
        if ((size_t)packet->data()->size() < sizeof(packetId))
            return false;

        packetId = *((PacketIdType*)packet->data()->constData());

        applyTicket(packetId);
        return true;
    }
    default:
        return false;
    }
    return false;
}

bool ThreadDataFrames::onPacketSent(const PacketBase::Ptr &)
{
    return true;
}

int ThreadDataFrames::applyTicket(PacketIdType packetId)
{
    if (!_queuePackets)
        return -1;

    auto result = _queuePackets->applyTicket(packetId);
    if (result > 0)
    {
        onPacketApplied(packetId);

        _currentPacket = nullptr;
        processQueue();
    }

    return result;
}

bool ThreadDataFrames::sendFrame(DataFrame::Ptr &frame)
{
    if (!frame)
        return false;

    bool needsTicket = frame->needsTicket();

    auto packet = std::dynamic_pointer_cast<DataFramesPacket>(_dataFramesPacketFactory->buildPacketFromFrame(frame, false));
    if (!packet)
        return false;

    packet->setPacketName(frame->name());

    if (_queuePackets && needsTicket)
    {
        _queuePackets->enqueue(packet);
        onPacketQueued(packet);

        processQueue();
    }
    else
    {
        packet->setPacketId(_dataFramesPacketFactory->generateNextPacketId());
        sendPacket(packet);
    }

    return true;
}

QueueDataFramesPackets *ThreadDataFrames::queuePackets() const
{
    return _queuePackets;
}

void ThreadDataFrames::resetQueue()
{
    _currentPacket = nullptr;
    if (_queuePackets)
        _queuePackets->reset();
}

void ThreadDataFrames::processQueue()
{
    if (!_queuePackets)
        return;

    auto nowTickCount = DateUtils::getTickCount();
    // ???????? ?????????????? ?????????? ??????????????????
    if (_currentPacket)
    {
        // ???????? ???????????? ?????????? ???????????????????? ?????????? ????????????????
        if (lastPacketSent() + 2000 < nowTickCount)
            // ???????????????? ????????????
            sendPacket(_currentPacket);
    }
    else
    {
        // ???????? ?????????????? ?????????? ???? ??????????????????, ???? ???????????????? ???? ?????????????? ???? ??????????????????????????
        _currentPacket = _queuePackets->first();
        if (_currentPacket)
        {
            // ?????????????????? ???????????? ???????????? ???????????? ???????????? ???? ??????????????
            _currentPacket->setPacketId(_dataFramesPacketFactory->generateNextPacketId());
            // ???????????????? ????????????
            sendPacket(_currentPacket);
        }
    }
}

}}
