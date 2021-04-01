#pragma once

#include <QtGlobal>
#include "../threader_global.h"

#ifdef Q_OS_WIN

#include "ThreadsCommon.h"

#include <QObject>
#include <QVector>

#include <windows.h>

namespace Threader {

namespace Threads {

using DescriptorsVector = QVector<Descriptor>;

class THREADERSHARED_EXPORT PollerBase : public QObject
{
    Q_OBJECT
public:
    explicit PollerBase(const DescriptorsVector &events = {});
    virtual ~PollerBase();

    bool hasEvent(Descriptor event);
    virtual DescriptorsVector *events();
    virtual void assign(const DescriptorsVector &events);
    virtual bool process(Descriptor eventToProcess);

private:
    DescriptorsVector _events;

signals:
    void signalOnPollEvent(PollerBase *sender);
};

using PollersList = QList<PollerBase*>;

/**
 * @brief Polling - Класс-обертка функционала polling-а
 */
class THREADERSHARED_EXPORT Polling
{
public:
    /**
     * @brief Polling - Конструктор
     */
    explicit Polling();

    /**
     * @brief pollersCount - Получение количества зарегистрированных голосующих
     * @return - Количество зарегистрированных голосующих
     */
    int pollersCount();

    /**
     * @brief registerPoller - Регистрация голосующего
     * @param poller - Голосующий
     * @return - Количество зарегистрированных голосующих
     */
    int registerPoller(PollerBase *poller);

    /**
     * @brief unregisterPoller - Снятие регистрации голосующего
     * @param poller - Голосующий
     * @return - Количество зарегистрированных голосующих
     */
    int unregisterPoller(PollerBase *poller);

    /**
     * @brief poll - Выполнение голосования
     * @param timeout - Время ожидания события голосования в милисекундах
     * @return
     */
    int poll(const uint32_t &timeout);

    /**
     * @brief waitCount - Получение времени режиме ожидания голосования в миллисекундах
     * @return - Время в режиме ожидания голосования в миллисекундах
     */
    qint64 waitCount() const;
private:
    /**
     * @brief _pollers - Список голосующих
     */
    PollersList _pollers;

    /**
     * @brief _waitCount - Время в миллисекундах, проведеное в ожидании
     */
    qint64 _waitCount;

};

}}

#endif
