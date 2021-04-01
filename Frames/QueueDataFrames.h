#pragma once

#include "../threader_global.h"

#include "DataFrameRawData.h"
#include "DataFramesCommon.h"
#include "DataFramesPackets.h"

#include "../Threads/ThreadBase.h"

#include <QHash>
#include <QMutex>

namespace Threader {

namespace Frames {

using namespace Threader::Threads;

//#define MESSAGE_NAME_QUEUE_UPDATED

/**
 * @brief QueueDataFrames - Класс очереди данных фреймов в памяти (DataFramesRawData)
 * Поддерживается потоко-защищенное добавление и извлечение фреймов
 * для использования в нескольких потоках по схеме:
 * один пишущий поток - один читающий поток
 */
class THREADERSHARED_EXPORT QueueDataFrames : public QObject
{
    Q_OBJECT
public:
    enum class ConnectionType
    {
        Server,
        Client
    };

    /**
     * @brief QueueDataFrames - Конструктор
     * @param alias - Псевдоним принимающей стороны
     * @param connectionType - Тип соединения
     * @param useQueue - Флаг использования очереди фреймов
     */
    explicit QueueDataFrames(const QString &alias = "",
                             const ConnectionType &connectionType = ConnectionType::Client,
                             const bool &useQueue = true);
    /**
     * @brief ~QueueDataFrames - Деструктор
     */
    virtual ~QueueDataFrames();

    /**
     * @brief count - Получение количества фреймов в очереди
     * @return - Количество фреймов в очереди
     */
    int count();

    /**
     * @brief collectAppendingFrame - Коллекционирование фреймов перед добавлением в очередь
     * По факту - Очередь на добавление в очередь
     * @param frame - Добавляемый фрейм
     * @return - Количество фреймов для добавления в очередь
     */
    int collectAppendingFrame(DataFrameRawData::Ptr &frame);

    /**
     * @brief collectAppendingFramesList - Коллекционирование фреймов перед добавлением в очеред
     * Принимает список фреймов
     * @param list - Добавляемый список фреймов
     * @return - Количество фреймов для добавления в очередь
     */
    int collectAppendingFramesList(DataFrameRawDataList &list);

    /**
     * @brief applyCollectedFrames - Единовременное добавление группы накопленных фреймов
     * в очередь с блокировкой доступа к хранилищу основной очереди
     * @return - Количество добавляемых фреймов
     */
    int applyCollectedFrames(QByteArray *snapshot = nullptr);

    /**
     * @brief appendingFramesCount - Получение количества фреймов, ожидающих добавление в очередь
     * @return - Количество фреймов, ожидающих добавление в очередь
     */
    int appendingFramesCount();

    /**
     * @brief prepareSendingList - Построение списка фреймов для отправки с блокировкой доступа
     * к хранилищу очереди
     * @param threadFor - Поток, для которого формируется очередь. Необходим для проверки активного
     * потока, обрабатывающего очередь
     * @param packetId - Идентификатор пакета списка фреймов
     * @param list - Заполняемый список фреймов
     */
    void prepareSendingList(const ThreadBase *threadFor,
                            PacketIdType &packetId,
                            DataFrameRawDataList &list);

    /**
     * @brief applyTicket - Подтверждение приема списка фреймов по идентификатору пакета
     * @param threadFor - Поток, для которого формируется очередь. Необходим для проверки активного
     * потока, обрабатывающего очередь
     * @param packetId - Идентификатор пакета списка фреймов
     * @param snapshot - массив для построения снимка очереди
     * @return - Количество подтвержденных и удаленных их очереди фреймов
     */
    int applyTicket(const ThreadBase *threadFor,
                    PacketIdType &packetId,
                    QByteArray *snapshot = nullptr);

    void reset();

    /**
     * @brief alias - Получение псевдонима принимающе стороны
     * @return - Псевдоним принимающе стороны
     */
    QString alias() const;

    /**
     * @brief thread - Получение потока, для которого формируется очередь
     * @return - Поток, для которого формируется очередь
     */
    ThreadBase *thread() const;

    /**
     * @brief bindThread - Привязка потока, для которого формируется очередь
     * @param thread - Поток, для которого формируется очередь
     */
    void bindThread(ThreadBase *thread);

    /**
     * @brief unbindThread - Отвязывание потока, для которого формируется очередь
     * @param thread - Поток, для которого формируется очередь
     */
    void unbindThread(ThreadBase *thread);

    /**
     * @brief notifyThreadQueueChanged - Уведомление привязанного потока
     * об изменениях в очереди
     */
    void notifyThreadQueueChanged();

    /**
     * @brief connectionType - Получение типа кодключения очереди
     * @return - Тип подключения очереди
     */
    ConnectionType connectionType() const;

    /**
     * @brief fileName - Получение имени файла очереди
     * @return - Имя файла очереди
     */
    QString fileName() const;

    bool useQueue() const;
    void setUseQueue(bool useQueue);

private:
    /**
     * @brief _alias - Псевдоним принимающей стороны
     */
    QString _alias;

    /**
     * @brief _appendedFramesList - Очередь на добавление в очередь
     */
    DataFrameRawDataList _appendedFramesList;

    /**
     * @brief _queuedFramesList - Хранилище основной очереди
     */
    DataFrameRawDataList _queuedFramesList;

    /**
     * @brief _mutex - Мьютекс блокировки очереди
     */
    QMutex *_mutex;

    /**
     * @brief _appendingMaximumPriority - Максимальный приоритет на отправку в очереди на добавление
     */
    uint8_t _appendingMaximumPriority;

    /**
     * @brief _queuedMaximumPriority - Максимальный приоритет на отправку в основной очереди
     */
    uint8_t _queuedMaximumPriority;

    /**
     * @brief _thread - Поток, для которого формируется очередь
     */
    ThreadBase *_thread;

    ConnectionType _connectionType;

    QString _fileName;

    bool _useQueue;

    uint _maximumPacketSize;

    /**
     * @brief makeSnapshot - Построение снимка очереди
     * @param data - Массив с бинарными данными снимка очереди
     */
    void makeSnapshot(QByteArray &data);
};

/**
 * @brief QueueDataFramesHash - Хеш-справочник очередей с индексированием по псевдониму
 */
using QueueDataFramesHash = QHash<QString, QueueDataFrames*>;

}}
