#pragma once

#include "ThreadBase.h"

#include "../threader_global.h"

namespace Threader {

namespace Threads {

/**
 * @brief ThreadTimer - Класс потока таймера
 * Класс не используется вручную, а создается в методе ThreadBase.startTimer
 */
class THREADERSHARED_EXPORT ThreadTimer : public ThreadBase
{
    Q_OBJECT
public:
    /**
     * @brief THREAD_NAME_PREFFIX - префика имени создаваемого потока
     */
    static const QString THREAD_NAME_PREFFIX;

    /**
     * @brief ThreadTimer - Конструктор
     * @param parent - Поток-владелец
     * @param timerName - Имя сообщения таймера
     * @param delayMsecs - Задержка таймера в миллисекундах
     * @param multiShot - Признак множественного срабатывания таймера
     */
    explicit ThreadTimer(IMessageSubscriber *parent,
                         const QString &timerName,
                         const int &delayMsecs,
                         const bool multiShot);

    /**
     * @brief timerName - Получение имени сообщения таймера
     * @return - Имя сообщения таймера
     */
    QString timerName() const;

    /**
     * @brief delay - Получение задержки таймера в миллисекундах
     * @return - Задержка таймера в миллисекундах
     */
    int delay() const;

    /**
     * @brief multiShot  - Получение признака множественного срабатывания таймера
     * @return - Признак множественного срабатывания таймера
     */
    bool multiShot() const;

protected:
    /**
     * @brief onIdle - Обработка ожидания потока
     */
    void onIdle() override;

    /**
     * @brief sendTimerMessage - Отправка сообщения таймера
     */
    virtual void sendTimerMessage();

private:
    /**
     * @brief _timerName - Имя сообщения таймера
     */
    QString _timerName;

    /**
     * @brief _delay - Задержка таймера в миллисекундах
     */
    int _delay;

    /**
     * @brief _multiShot - Признак множественного срабатывания таймера
     */
    bool _multiShot;

    /**
     * @brief _shotCount - Счетчик срабатывания таймера
     */
    qint64 _shotCount;
};

}}
