#pragma once

#include "ThreadHandler.h"

#include "../Frames/DataFramesPackets.h"
#include "../Frames/DataFramesFactory.h"

#include "../threader_global.h"

#include <QObject>

using namespace Threader::Frames;

namespace Threader {

namespace Threads {


const QString ALIAS_UNDEFINED = "Undefined";

class THREADERSHARED_EXPORT ThreadDataFrames : public ThreadHandler
{
    Q_OBJECT
public:
    enum class DisconnectReason : uint8_t
    {
        StopWorking = 0,
        ConnectionError,
        AuthorizationTimeout,
        AliveTimeout
    };
    Q_ENUM(DisconnectReason)

    explicit ThreadDataFrames(IMessageSubscriber *parent,
                              const Descriptor &socket,
                              const QString &host,
                              const int &port,
                              const bool &useQueue);

    explicit ThreadDataFrames(IMessageSubscriber *parent,
                              const QString &host,
                              const int &port,
                              const uint &reconnectTimeout,
                              const bool &useQueue);

    ~ThreadDataFrames() override;

    virtual DataFramesFactory *framesFactory() = 0;

    bool isAuthorized() const;
    void setIsAuthorized(bool isAuthorized);

protected:
    void onConnected() override;

    bool onPacketReceived(const PacketBase::Ptr &packet) override;
    bool onPacketSent(const PacketBase::Ptr &) override;

    virtual void onPacketIdDiscontinuity(const PacketIdType &waitFor,
                                         const PacketIdType &received) = 0;

    virtual void onPacketQueued(const PacketBase::Ptr &) = 0;

    virtual void onPacketApplied(const PacketIdType &packetId) = 0;

    virtual void onFrameReceived(const DataFramesList &frames,
                                 const PacketIdType &packetId) = 0;

    virtual int applyTicket(PacketIdType packetId);

    virtual bool sendFrame(DataFrame::Ptr &frame);

    QueueDataFramesPackets *queuePackets() const;

    void resetQueue();
    void processQueue();
private:
    bool _isAuthorized = false;    
    qint64 _nextPacketId;

    DataFramesPacket::Ptr _currentPacket;
    DataFramesPacketFactory *_dataFramesPacketFactory;
    QueueDataFramesPackets *_queuePackets;
};

}}
