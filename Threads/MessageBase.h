#pragma once

#include "../threader_global.h"

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QMutex>
#include <QThread>
#include <QVector>

#include <memory>

namespace Threader {

namespace Threads {

/**
 * @brief MessageBase - Базовый класс сообщения, передаваемого между потоками
 */
class THREADERSHARED_EXPORT MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageBase>;

public:
    /**
     * @brief BaseMessage - Конструктор класса с задаваемым именем сообщения
     * @param name - Имя сообщения
     */
    explicit MessageBase(const QString& name);

    /**
     * @brief ~MessageBase - Деструктор класса
     */
    virtual ~MessageBase();

    /**
     * @brief created - Получение даты и времени создания сообщения
     * @return - Дата и время создания сообщения
     */
    QDateTime created();

    /**
     * @brief name - Получение имени сообщения
     * @return - Имя сообщения
     */
    QString name() const;

    /**
     * @brief threadId - Получение идентификатора потока, создавшего сообщение
     * @return - Идентификатор потока
     */
    long threadId() const;

    static int referenceCount();

protected:

    /**
     * @brief setCreated - Установка даты и времени создания сообщения
     * @param created - Новые дата и время создания сообщения
     */
    void setCreated(const QDateTime &created);

private:
    /**
     * @brief _created - Дата и время создания сообщения
     */
    QDateTime _created;

    /**
     * @brief _name - Имя сообщения
     */
    QString _name;

    /**
     * @brief _threadId - Идентификатор потока
     */
    long _threadId;

    /**
     * @brief _referenceCount - Счетчик экземпляров
     */
    static int _referenceCount;

    static QMutex _mutexReferenceCount;
    void incrementReferenceCount();
    void decrementReferenceCount();
};

using MessagesList = QList<MessageBase::Ptr>;

using MessagesVector = QVector<MessageBase::Ptr>;

/**
 * @brief ISubscriber - Интерфейс потребителя сообщений
 */
class IMessageSubscriber
{
public:

    virtual ~IMessageSubscriber() = default;

    /**
     * @brief postMessage - Оправка сообщения потоку
     * @param message - Сообщения для потока
     */
    virtual void postMessage(const MessageBase::Ptr &message) = 0;

    /**
     * @brief postMessages - Отправка потоку группы сообщений
     * @param messagesList - Группа сообщений
     */
    virtual void postMessages(const MessagesList &messagesList) = 0;

    /**
     * @brief postTerminateEvent - Отправка события завершения потока
     */
    virtual void postTerminateEvent() = 0;

    /**
     * @brief postEventChildThreadTerminated - отправка события завершения потоку владельцу
     */
    virtual void postEventChildThreadTerminated() = 0;
};

using MessageSubscribersVector = QVector<IMessageSubscriber*>;

}}

Q_DECLARE_METATYPE(Threader::Threads::MessagesList)
Q_DECLARE_METATYPE(Threader::Threads::MessagesVector)

const int MessagesListTypeId = qRegisterMetaType<Threader::Threads::MessagesList>();
const int MessagesVectorTypeId = qRegisterMetaType<Threader::Threads::MessagesVector>();


