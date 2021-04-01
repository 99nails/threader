#pragma once

#include "MessageBase.h"
#include "../threader_global.h"

#include <QObject>
#include <QList>
#include <QQueue>
#include <QMutexLocker>


namespace Threader {

namespace Threads {

/**
 * @brief MessageQueue - Класс-хранище очереди сообщений для асинхоронного обмена между потоками
 */
class THREADERSHARED_EXPORT QueueMessages
{
public:

    /**
     * @brief MessagesQueue - Конструктор класса очереди
     */
    explicit QueueMessages();

    /**
     * @brief ~MessageQueue - Деструктор
     */
    virtual ~QueueMessages();

    /**
     * @brief count - Получение количества сообщений в очереди
     * @return - Количество сообщений в очереди
     */
    int count();

    /**
     * @brief enqueue - Размещение сообщеиня в очереди
     * @param message - Размещаемое сообщение
     * @return - Количество сообщений в очереди
     */
    int enqueue(const MessageBase::Ptr& message);

    /**
     * @brief enqueue - Размещеине списка сообщений в очереди
     * @param list - список сообщений
     * @return - Количество сообщений в списке
     */
    int enqueue(const MessagesList &list);

    /**
     * @brief dequeue - Извлечение сообщения из очереди
     * @return - Извлеченное сообщение. Если очередь пуста, то возвращается NULL
     */
    MessageBase::Ptr dequeue();

    /**
     * @brief dequeue - Извлечение заданного количества сообщений из очереди
     * @param count - Требуемое количество извлекаемых сообщений
     * @return - Список извлеченных сообщений
     */
    MessagesList dequeue(int count);

    /**
     * @brief dequeueAll - Извлечение из очереди всех сообщений
     * @return - Список извлеченных сообщений
     */
    MessagesList dequeueAll();

private:

    /**
     * @brief _queue - Хранилище очереди сообщений
     */
    QQueue<MessageBase::Ptr> _queue;

    /**
     * @brief _mutex - Мьютекс синхронизации доступа к хранилищу очереди
     */
    QMutex *_mutex;
};

}}
