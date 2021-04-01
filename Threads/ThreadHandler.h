#pragma once

#include "HandlerBase.h"
#include "PacketFactoryBase.h"
#include "ThreadBase.h"

#include "../Utils/DataStream.h"
#include "../Utils/TrafficCounter.h"

#include "../threader_global.h"

#include <QObject>

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


class THREADERSHARED_EXPORT ThreadHandler : public ThreadBase
{
    Q_OBJECT
public:
    static const QString MESSAGE_NAME_DEVICE_DATA_INPUT;
    static const QString MESSAGE_NAME_DEVICE_DATA_OUTPUT;
    static const QString MESSAGE_NAME_DEVICE_CONNECTING;
    static const QString MESSAGE_NAME_DEVICE_CONNECTED;
    static const QString MESSAGE_NAME_DEVICE_DISCONNECTED;
    static const QString MESSAGE_NAME_DEVICE_ERROR;

    explicit ThreadHandler(IMessageSubscriber *parent,
                           HandlerBase *handler,
                           const uint reconnectTimeout = 0,
                           PacketFactoryBase *packetFactory = nullptr,
                           const QString &threadName = "",
                           const bool &portionedIO = false);
    ~ThreadHandler() override;

protected:
    HandlerBase *handler();
    void setHandler(HandlerBase *handler);

    PacketFactoryBase *packetFactory();

    DataStream *inputStream();
    DataStream *outputStream();

    void clearBuffers();

    void terminateChildThreads() override;
    void onBeforeWaitEvents() override;

    virtual void onConnecting(){}
    virtual void onConnected(){}
    virtual void onDisconnected(){}

    virtual void onReadData(const char *, const int &){}
    virtual void onWriteData(const char *, const int &){}
    virtual void onError(const int){}

    virtual bool onPacketReceived(const PacketBase::Ptr &) = 0;
    virtual bool onPacketSent(const PacketBase::Ptr &) = 0;

    virtual bool sendPacket(const PacketBase::Ptr &packet);

    qint64 lastPacketSent() const;
    qint64 lastPacketReceived() const;

    TrafficCounter *inputTrafficCounter();
    TrafficCounter *outputTrafficCounter();

private:
    HandlerBase *_handler;
    uint _reconnectTimeout;
    qint64 _nextTryToConnect;

    QByteArray _inputBuffer;
    QByteArray _outputBuffer;

    // TODO: Возможно надо убрать
    DataStream _inputStream;
    DataStream _outputStream;

    PacketFactoryBase *_packetFactory;

    qint64 _lastPacketSent;
    qint64 _lastPacketReceived;

    TrafficCounter _inputTrafficCounter;
    TrafficCounter _outputTrafficCounter;

    bool _portionedIO;

private slots:
    void slotOnConnectionStateChanged(HandlerBase *sender,
                                    HandlerBase::ConnectionState oldState,
                                    HandlerBase::ConnectionState newState);
    void slotOnReadyToRead(HandlerBase *handler);
    void slotOnReadyToWrite(HandlerBase *handler);
    void slotOnError(HandlerBase *sender, int errorCode);
};


}}
