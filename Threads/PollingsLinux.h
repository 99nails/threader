#pragma once


#include "ThreadsCommon.h"
#include "../threader_global.h"


#include <QtGlobal>

#ifdef Q_OS_LINUX

#include <QObject>
#include <QList>
#include <QVector>
#include <poll.h>

namespace Threader {

namespace Threads {

/**
 * @brief The PollerBase class
 */
class THREADERSHARED_EXPORT PollerBase : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief PollerBase - Конструктор
     * @param descriptor - Дескриптор голосующего
     * @param events - Маска событий
     */
    explicit PollerBase(const int &descriptor = -1,
                        const short int &events = 0);

    /**
     * @brief assign - Заполнение данными голосующего pollfd структуры,
     * предоставляемой снаружи для формирования массива ожидания
     * @param event - pollfd структура голосующего
     */
    virtual void assign(pollfd &event);

    /**
     * @brief process - Обработка события голосования
     * @param event - pollfd структура голосующего
     * @return - Признак обработки события
     */
    virtual bool process(const pollfd &event);

    /**
     * @brief descriptor - Получение дескриптора голосующего
     * @return  - Дескриптор голосующего
     */
    virtual int descriptor() const;

    /**
     * @brief setDescriptor - Установка дескриптора голосующего
     * @param descriptor - Дескритор голосующего
     */
    virtual void setDescriptor(const Descriptor &descriptor);

    /**
     * @brief events - Получение маски события голосующего
     * @return - Маска события
     */
    short int events();

    /**
     * @brief setEvents - Получение маски события голосующего
     * @param events - Маска события
     */
    void setEvents(short int events);

private:
    int _descriptor;
    short int _events;

signals:
    void signalOnPollEvent(const PollerBase *sender, const pollfd &event);
};

using  PollersList = QList<PollerBase*>;

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
     * @brief indexOfDescriptor - Получение индекса голосующего по его дескриптору
     * @param descriptor - Дескриптор
     * @return - Индекс голосующего
     */
    int indexOfDescriptor(const int &descriptor);

    /**
     * @brief _pollers - Список голосующих
     */
    PollersList _pollers;

    /**
     * @brief _pollStructArray - Массив для хранения структур голосующих
     */
    QVector<pollfd> _pollStructArray;

    /**
     * @brief _waitCount - Время в миллисекундах, проведеное в ожидании
     */
    qint64 _waitCount;
};

}}

#endif
