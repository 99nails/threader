#include "QueueDataFrames.h"

#include "MessageQueue.h"

namespace Threader {

namespace Frames {


//#define FRAMES_PACKET_MAXIMUM_DATA_SIZE_MAGIC 65400
//#define FRAME_LENGTH_SIZE 2

QueueDataFrames::QueueDataFrames(const QString &alias,
                                 const ConnectionType &connectionType,
                                 const bool &useQueue)
    : QObject(nullptr)
    , _alias(alias)
    , _mutex(new QMutex())
    , _appendingMaximumPriority(1)
    , _queuedMaximumPriority(1)
    , _thread(nullptr)
    , _connectionType(connectionType)
    , _fileName(_alias + ((ConnectionType::Server == _connectionType) ? ".Hub" : "") + ".queue")
    , _useQueue(useQueue)
    , _maximumPacketSize(1 * 1024 * 1024)
{
}

QueueDataFrames::~QueueDataFrames()
{
    delete _mutex;
}

int QueueDataFrames::count()
{
    QMutexLocker locker(_mutex);
    return _queuedFramesList.count();
}

int QueueDataFrames::collectAppendingFrame(DataFrameRawData::Ptr &frame)
{
    if (frame)
    {
        // добавление фрейма в очередь на добавление
        _appendedFramesList.append(frame);

        // вычисление максимального приоритета добавляемого набора фреймов
        uint8_t framePriority = frame->priority();
        if (framePriority > _appendingMaximumPriority)
            _appendingMaximumPriority = framePriority;
    }
    return _appendedFramesList.count();
}

int QueueDataFrames::collectAppendingFramesList(DataFrameRawDataList &list)
{
    _appendedFramesList.append(list);
    for (int i = 0; i < _appendedFramesList.count(); i ++)
    {
        DataFrameRawData::Ptr frame = _appendedFramesList.at(i);

        // вычисление максимального приоритета добавляемого набора фреймов
        uint8_t framePriority = frame->priority();
        if (framePriority > _appendingMaximumPriority)
            _appendingMaximumPriority = framePriority;
    }
    return _appendedFramesList.count();
}

int QueueDataFrames::applyCollectedFrames(QByteArray *snapshot)
{
    // проверка наличия данных для добавления
    if (_appendedFramesList.count() == 0)
        return 0;

    QMutexLocker locker(_mutex);

    // блочное добавление
    _queuedFramesList.append(_appendedFramesList);

    // очистка очереди на добавление
    _appendedFramesList.clear();

    // вычисление максимального приоритета в основной очереди
    if (_queuedMaximumPriority < _appendingMaximumPriority)
        _queuedMaximumPriority = _appendingMaximumPriority;

    // сброс приоритета в очереди на добавление
    _appendingMaximumPriority = 1;

    if (nullptr != snapshot)
        makeSnapshot(*snapshot);

    // уведомление потока
    notifyThreadQueueChanged();

    return _queuedFramesList.count();
}

int QueueDataFrames::appendingFramesCount()
{
    return _appendedFramesList.count();
}

void QueueDataFrames::prepareSendingList(const ThreadBase *threadFor,
                                         PacketIdType &packetId,
                                         DataFrameRawDataList &list)
{
    QMutexLocker locker(_mutex);

    // проверка потока на соотвествие зарегистрированному обработчику очереди
    if (nullptr == threadFor && _thread != threadFor)
        return;

    // очистка списка на отправку
    list.clear();
    // счетчик отправляемых данных
    quint64 dataSize = 0;
    for (int i = 0; i < _queuedFramesList.count(); i++)
    {
        // для всех фреймов от начала очереди
        DataFrameRawData::Ptr frame = _queuedFramesList[i];
        if (frame)
        {
            // если не назначен пакет и приоритет не ниже зарегистрированного на отправку
            if (frame->packetId() < 1 && frame->priority() >= _queuedMaximumPriority)
            {
                int requedDataSize = frame->length() + sizeof(FrameSizeType);
                // если фрейм помещается в пакет данных
                if (dataSize + requedDataSize < _maximumPacketSize)
                {
                    // добавление в список на отправку
                    list.append(frame);
                    // установка номера пакета, как признака фрейма в обработке на отправку
                    frame->setPacketId(packetId);
                    // увеличение размера отправляемого пакета
                    dataSize += requedDataSize;
                }
                else
                    break;
            }
        }
    }
}

int QueueDataFrames::applyTicket(const ThreadBase *threadFor,
                                 PacketIdType &packetId,
                                 QByteArray *snapshot)
{
    QMutexLocker locker(_mutex);

    if (nullptr == threadFor && _thread != threadFor)
        return 0;

    int count = 0;
    _queuedMaximumPriority = 1;

    // проход в обратную сторону списка, т.к. возможно удаление из списка
    for (int i = _queuedFramesList.count() - 1; i >= 0; i--)
    {
        DataFrameRawData::Ptr frame = _queuedFramesList[i];
        if (frame)
        {
            PacketIdType framePacketId = frame->packetId();
            // если идентификатор пакета совпадает с подтвержденным пакетом
            if (framePacketId == packetId)
            {
                // учет количества подтвержденных фреймов
                count++;
                // забываем о фрейме
                _queuedFramesList.removeAt(i);
            }
            else
            {
                // вычисление максимального приоритета в очереди по ходу пьесы
                int framePriority = frame->priority();
                if (framePriority > _queuedMaximumPriority)
                    _queuedMaximumPriority = framePriority;
            }
        }
    }

    if (nullptr != snapshot)
        makeSnapshot(*snapshot);

    return count;
}

void QueueDataFrames::reset()
{
    QMutexLocker locker(_mutex);

    for (int i = 0; i < _queuedFramesList.count();  i++)
    {
        DataFrameRawData::Ptr frame = _queuedFramesList[i];
        frame->setPacketId(-1);
    }
}

QString QueueDataFrames::alias() const
{
    return _alias;
}

ThreadBase *QueueDataFrames::thread() const
{
    QMutexLocker locker(_mutex);

    return _thread;
}

void QueueDataFrames::bindThread(ThreadBase *thread)

{
    QMutexLocker locker(_mutex);

    // пустой поток привязать нельзя
    if (nullptr == thread)
        return;

    // если поток уже установлен, его нужно остановить
    if (_thread != thread && nullptr != _thread)
    {
        _thread->postTerminateEvent();
    }

    // и привязать к очереди новый поток
    _thread = thread;

    // и передать потоку экземпляр очереди, т.е. саму себя
    notifyThreadQueueChanged();
}

void QueueDataFrames::unbindThread(ThreadBase *thread)
{
    QMutexLocker locker(_mutex);

    // пустой поток отвязать нельзя
    if (nullptr == thread)
        return;

    // нельзя отвязывать другой поток
    if (_thread != thread)
        return;

    // в случае сопадения экземпляров потоков остановить поток
    _thread->postTerminateEvent();
    _thread = nullptr;
}

void QueueDataFrames::notifyThreadQueueChanged()
{
    if (nullptr != _thread)
    {
        _thread->postMessage(std::make_shared<MessageQueue>(this, _alias));
    }
}

QueueDataFrames::ConnectionType QueueDataFrames::connectionType() const
{
    return _connectionType;
}

QString QueueDataFrames::fileName() const
{
    return _fileName;
}

bool QueueDataFrames::useQueue() const
{
    return _useQueue;
}

void QueueDataFrames::setUseQueue(bool useQueue)
{
    _useQueue = useQueue;
}

void QueueDataFrames::makeSnapshot(QByteArray &data)
{
    // подсчет размера данных очереди
    int dataSize = 0;
    for (int i = 0; i < _queuedFramesList.count(); i++)
        dataSize += _queuedFramesList.at(i)->streamSize();

    // очистка и резервирование размера снимка
    data.clear();
    data.reserve(dataSize);

    // сохранение очереди в массив
    for (int i = 0; i < _queuedFramesList.count(); i++)
        _queuedFramesList.at(i)->appendByteArray(data, false);
}


}}
