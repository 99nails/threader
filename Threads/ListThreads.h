#pragma once

#include "ThreadBase.h"
#include "../threader_global.h"

#include <QList>
#include <QHash>
#include <QMutex>

namespace Threader {

namespace Threads {


typedef struct ThreadStatistic
{
    long ThreadId;
    QString ThreadName;
    qint64 StartedMsecsSinceEpoch;
    qint64 AliveMsecsSinceEpoch;
    qint64 WaitMSecsCount;
    int QueueCount;
    bool Terminated;
} ThreadStatisticStruct;

using ListStatistic = QList<ThreadStatisticStruct>;

using HashThreadsStatistic = QHash<ThreadBase*, ThreadStatisticStruct>;

class THREADERSHARED_EXPORT ListThreads
{
public:
    explicit ListThreads();
    virtual ~ListThreads();

    static ListThreads *instance();

    /**
     * @brief registerThread - Регистрация потока в общем списке потоков
     * @param thread - Регистрирующийся поток
     */
    void registerThread(ThreadBase *thread);

    /**
     * @brief unregisterThread - Отмена регистрации потока в списке потоков
     * @param thread - Удаляемый поток
     */
    void unregisterThread(ThreadBase *thread);

    /**
     * @brief threadByClassName - Получение экземпляра потока по имени класса потока
     * @param threadClassName - Имя класса потока
     * @return - Экземпляр класса потока
     */
    ThreadBase* threadByClassName(const QString& threadClassName);

    /**
     * @brief accumulateStatistic - Хранение статистики в памяти
     * @param thread - Указатель на поток
     * @param startedMsecsSinceEpoch - Время старта
     * @param aliveMsecsSinceEpoch - Время снятия статистики
     * @param waitMSecsCount - Ожидание в миллисекундах
     * @return - Статистика добавлена
     */
    bool accumulateStatistic(ThreadBase *thread,
                             const long threadId,
                             const QString &threadName,
                             const qint64 &startedMsecsSinceEpoch,
                             const qint64 &aliveMsecsSinceEpoch,
                             const uint64_t &waitMSecsCount,
                             const int &queueCount,
                             const bool terminated);

    /**
     * @brief statistic - Получение списка со статистикой потоков
     * @return - Список со статистикой потоков
     */
    ListStatistic statistic();

private:

    static ListThreads *_instance;

    ThreadsList _threads;

    HashThreadsStatistic _statistic;

    QMutex *_mutex;
};

}}
