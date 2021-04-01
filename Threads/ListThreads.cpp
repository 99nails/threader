#include "ListThreads.h"

namespace Threader {

namespace Threads {

ListThreads *ListThreads::_instance = nullptr;

ListThreads::ListThreads()
{
    _mutex = new QMutex();
}

ListThreads::~ListThreads()
{
    delete _mutex;
}

ListThreads *ListThreads::instance()
{
    if (!_instance)
        _instance = new ListThreads();
    return _instance;
}

void ListThreads::registerThread(ThreadBase *thread)
{
    QMutexLocker locker(_mutex);
    _threads.append(thread);
}

void ListThreads::unregisterThread(ThreadBase *thread)
{
    QMutexLocker locker(_mutex);
    _threads.removeAll(thread);
    if (_statistic.contains(thread))
        _statistic.remove(thread);
}

ThreadBase *ListThreads::threadByClassName(const QString& threadClassName)
{
    ThreadBase *thread;

    QMutexLocker locker(_mutex);

    for (int i = 0; i < _threads.count();i++)
    {
        thread = _threads.at(i);
        if (thread->threadClassName() == threadClassName)
            return thread;
    }
    return nullptr;
}

bool ListThreads::accumulateStatistic(ThreadBase* thread,
                                      const long threadId,
                                      const QString &threadName,
                                      const qint64 &startedMsecsSinceEpoch,
                                      const qint64 &aliveMsecsSinceEpoch,
                                      const uint64_t &waitMSecsCount,
                                      const int &queueCount,
                                      const bool terminated)
{
    if (!thread)
        return false;

    QMutexLocker locker(_mutex);

    ThreadStatisticStruct statistic;
    statistic.ThreadId = threadId;
    statistic.ThreadName = threadName;
    statistic.StartedMsecsSinceEpoch = startedMsecsSinceEpoch;
    statistic.AliveMsecsSinceEpoch = aliveMsecsSinceEpoch;
    statistic.WaitMSecsCount = qint64(waitMSecsCount);
    statistic.QueueCount = queueCount;
    statistic.Terminated = terminated;
    _statistic[thread] = statistic;
    return true;
}

ListStatistic ListThreads::statistic()
{
    QMutexLocker locker(_mutex);

    ListStatistic result;
    for (int i = 0; i < _threads.count(); i++)
    {
        ThreadBase *thread = _threads.at(i);
        if (_statistic.contains(thread))
        {
            result.append(_statistic[thread]);
        }
    }
    return result;
}

}}
