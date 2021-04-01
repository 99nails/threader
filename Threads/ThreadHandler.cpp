#include "ThreadHandler.h"

#include "MessageBinary.h"

#include "../Utils/DateUtils.h"
#include "../Utils/DataStream.h"

namespace Threader {

namespace Threads {

const QString ThreadHandler::MESSAGE_NAME_DEVICE_CONNECTING = "Device.Connecting";
const QString ThreadHandler::MESSAGE_NAME_DEVICE_CONNECTED = "Device.Connected";
const QString ThreadHandler::MESSAGE_NAME_DEVICE_DISCONNECTED = "Device.Disconnected";
const QString ThreadHandler::MESSAGE_NAME_DEVICE_ERROR = "Device.Error";
const QString ThreadHandler::MESSAGE_NAME_DEVICE_DATA_INPUT = "Device.Data.Input";
const QString ThreadHandler::MESSAGE_NAME_DEVICE_DATA_OUTPUT = "Device.Data.Output";

ThreadHandler::ThreadHandler(IMessageSubscriber *parent,
                             HandlerBase *handler,
                             const uint reconnectTimeout,
                             PacketFactoryBase *packetFactory,
                             const QString &threadName,
                             const bool &portionedIO)
    : ThreadBase(parent, threadName, ThreadRunMode::Polling)
    , _reconnectTimeout(reconnectTimeout)
    , _nextTryToConnect(DateUtils::getTickCount())
    , _inputBuffer(QByteArray())
    , _outputBuffer(QByteArray())
    , _inputStream(DataStream(&_inputBuffer))
    , _outputStream(DataStream(&_outputBuffer))
    , _packetFactory(packetFactory)
    , _lastPacketSent(DateUtils::getTickCount())
    , _lastPacketReceived(_lastPacketSent)
    , _portionedIO(portionedIO)
{
    // перенос обработчика в контекст текущего потока
    handler->moveToThread(this);

    // установка обработчика, его сигналов и слотов
    setHandler(handler);
}

ThreadHandler::~ThreadHandler()
{
    if (_handler)
    {
        delete _handler;
        _handler = nullptr;
    }

    if (_packetFactory)
    {
        delete _packetFactory;
        _packetFactory = nullptr;
    }
}

HandlerBase *ThreadHandler::handler()
{
    return _handler;
}

void ThreadHandler::setHandler(HandlerBase *handler)
{
    _handler = handler;
    connect(_handler, &HandlerBase::signalOnConnectionStateChanged,
            this, &ThreadHandler::slotOnConnectionStateChanged, Qt::DirectConnection);
    connect(_handler, &HandlerBase::signalOnError,
            this, &ThreadHandler::slotOnError, Qt::DirectConnection);
    connect(_handler, &HandlerBase::signalOnReadyRead,
            this, &ThreadHandler::slotOnReadyToRead, Qt::DirectConnection);
    connect(_handler, &HandlerBase::signalOnReadyWrite,
            this, &ThreadHandler::slotOnReadyToWrite, Qt::DirectConnection);

    polling()->registerPoller(_handler);

    if (handler->connectionState() != HandlerBase::ConnectionState::Disconnected)
        slotOnConnectionStateChanged(handler,
                                     HandlerBase::ConnectionState::Disconnected,
                                     handler->connectionState());
}

PacketFactoryBase *ThreadHandler::packetFactory()
{
    return _packetFactory;
}

DataStream *ThreadHandler::inputStream()
{
    return &_inputStream;
}

DataStream *ThreadHandler::outputStream()
{
    return &_outputStream;
}

void ThreadHandler::clearBuffers()
{
    _inputBuffer.clear();
    _outputBuffer.clear();
}

void ThreadHandler::terminateChildThreads()
{
    ThreadBase::terminateChildThreads();
    if (_handler->isConnected())
        _handler->close();
}

void ThreadHandler::onBeforeWaitEvents()
{
    if (!_handler)
        return;

    // проверка необходимости открытия устройства
    if (_reconnectTimeout > 0 && _handler->connectionState() == HandlerBase::ConnectionState::Disconnected
            && _nextTryToConnect <= DateUtils::getTickCount())
    {
        _handler->open();
    }

    // если идет подключение или подключен
    if (_handler->connectionState() != HandlerBase::ConnectionState::Disconnected)
    {
        // проверка необходимости записи в устройство
        _handler->setNeedsToWrite(_outputBuffer.count() > 0);
    }
}

bool ThreadHandler::sendPacket(const PacketBase::Ptr &packet)
{
    if (!packet || !_handler->isConnected())
        return false;

    bool result = packet->write(*outputStream());
    slotOnReadyToWrite(handler());

    _lastPacketSent = DateUtils::getTickCount();
    onPacketSent(packet);
    return result;
}

qint64 ThreadHandler::lastPacketSent() const
{
    return _lastPacketSent;
}

qint64 ThreadHandler::lastPacketReceived() const
{
    return _lastPacketReceived;
}

TrafficCounter *ThreadHandler::inputTrafficCounter()
{
    return &_inputTrafficCounter;
}

TrafficCounter *ThreadHandler::outputTrafficCounter()
{
    return &_outputTrafficCounter;
}

void ThreadHandler::slotOnConnectionStateChanged(HandlerBase *sender,
                                                 HandlerBase::ConnectionState oldState,
                                                 HandlerBase::ConnectionState newState)
{
    clearBuffers();

    switch (newState) {
    case HandlerBase::ConnectionState::Connecting:
        onConnecting();
        break;
    case HandlerBase::ConnectionState::Connected:
        _lastPacketSent = DateUtils::getTickCount();
        _lastPacketReceived = _lastPacketSent;

        _inputTrafficCounter.clear();
        _outputTrafficCounter.clear();

        onConnected();
        break;
    case HandlerBase::ConnectionState::Disconnected:
        onDisconnected();
        _nextTryToConnect = DateUtils::getNextTickCount(_reconnectTimeout);
        break;
    }
    Q_UNUSED(sender)
    Q_UNUSED(oldState)
}

void ThreadHandler::slotOnReadyToRead(HandlerBase *sender)
{
    if (!sender || sender->connectionState() == HandlerBase::ConnectionState::Disconnected)
        return;

    // чтение данных из входного буфера
    char buffer[65536];
    int readCount;
    uint totalReadCount = 0;

    QByteArray localInputBuffer;
    localInputBuffer.reserve(1024 * 1024);

    do
    {
        // иницализация шага цикла чтения
        memset(buffer, 0, sizeof(buffer));

        // чтение данных
        readCount = sender->read(buffer, sizeof(buffer));

        if (readCount > 0) {
            // если обмен данными происходит фиксированными порциями как в UDP
            if (_portionedIO)
            {
                onReadData(buffer, readCount);
            }
            else
            {
                // сохранение прочитанных данных при успешном чтении
                localInputBuffer.append(buffer, readCount);
                totalReadCount += uint(readCount);
            }
        }
    } while (readCount > 0);

    // подсчет трафика
    _inputTrafficCounter.append(QDateTime::currentDateTime(), totalReadCount);

    // если обмен данными не происходит фиксированными порциями как в UDP
    if (!_portionedIO)
    {
        // добавление во входящий буфер
        _inputBuffer.append(localInputBuffer);

        // вызов заглушки события приема данных
        onReadData(localInputBuffer.constData(), localInputBuffer.size());

        // если фабрика пакетов не определена
        if (!_packetFactory)
        {
            if (parentThread())
            {
                // создание сообщения с бинарными данными
                MessageBase::Ptr message(
                            std::make_shared<MessageBinary>(
                                MESSAGE_NAME_DEVICE_DATA_INPUT,
                                _inputBuffer.data(),
                                _inputBuffer.size()));
                // отправка сообщения с бинарными данными
                parentThread()->postMessage(message);
            }
            _inputBuffer.clear();
        }
        else
        {
            // попытка распознавания пакетов с данными, пока они распознаются
            while(true)
            {
                PacketBase::Ptr packet = _packetFactory->tryExtractPacket(_inputBuffer);
                // если пакет распознан
                if (packet)
                {
                    _lastPacketReceived = DateUtils::getTickCount();
                    onPacketReceived(packet);
                }
                else
                {
                    int result = _packetFactory->lastResult();
                    QByteArray code(1, char(result & 0xFF));
                    auto message(std::make_shared<MessageBinary>(MESSAGE_NAME_DEVICE_ERROR, code));
                    postMessage(message);
                    break;
                }
            }
        }
    }
}

void ThreadHandler::slotOnReadyToWrite(HandlerBase *sender)
{
    // идем лесом если нечего отправить в устройство
    if (_outputBuffer.size() == 0)
        return;

    // непосредственно отправка данных
    int written = sender->write(_outputBuffer.constData(), _outputBuffer.size());

    // если хоть что-то ушло в устройство
    if (written > 0) {
        // подсчет отправленных данных
        _outputTrafficCounter.append(QDateTime::currentDateTime(), static_cast<uint>(written));
        // вызов заглушки протоколирования отправки
        onWriteData(_outputBuffer.constData(), written);

        // удаление отправленных данных из буфера
        _outputBuffer.remove(0, written);
        // сдвиг указателя потока на written байт к началу
        _outputStream.setPosition(_outputStream.position() - written);
    }
}

void ThreadHandler::slotOnError(HandlerBase *sender, int errorCode)
{
    onError(errorCode);
    Q_UNUSED(sender)
}

}}
