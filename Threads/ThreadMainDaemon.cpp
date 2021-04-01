#include "ThreadMainDaemon.h"

#include "ListThreads.h"
#include "LogMessagesTemplates.h"
#include "MessageWriteToFile.h"

#include "../Utils/DateUtils.h"

#include <QCoreApplication>

#include <iostream>

#ifdef Q_OS_WIN

#include "Psapi.h"

#endif

#ifdef Q_OS_LINUX

#include <sys/time.h>
#include <sys/resource.h>

extern void setThreadInfo(const QString & info);

#endif

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


#define TIMEOUT_BUILD_STATISTIC_MILLISECONDS 10000

bool ThreadMainDaemon::_runConsole = false;

ThreadMainDaemon::ThreadMainDaemon(const QString &threadName,
                                   const QString &serviceName,
                                   const QString &applicationName,
                                   const SettingsBundle::Ptr &settings)
    : ThreadBase(nullptr, threadName)
    , _serviceName(serviceName)
    , _applicationName(applicationName)
    , _settings(settings)
    , _nextBuildStatisticTickCount(DateUtils::getNextTickCount(TIMEOUT_BUILD_STATISTIC_MILLISECONDS))
    , _nextBuildCommonStatisticTickCount(0)
    , _threadStatisticFileName(QCoreApplication::applicationName() + ".Threads")
{
    setTimeout(1000);
}

ThreadMainDaemon::~ThreadMainDaemon()
{
}

QString ThreadMainDaemon::serviceName() const
{
    return _serviceName;
}

QString ThreadMainDaemon::applicationName() const
{
    return _applicationName;
}

bool ThreadMainDaemon::runConsole()
{
    return _runConsole;
}

void ThreadMainDaemon::setRunConsole(bool runConsole)
{
    _runConsole = runConsole;
}

void ThreadMainDaemon::onThreadStarted()
{
    writeLog(Message100, _applicationName.toUtf8().constData());
    snapshotThreads(true);

    if (_settings)
    {
        QString fileName = _settings->fileName();
        writeLog(Message110, fileName.toUtf8().constData());

        bool optionsRead = _settings->readBundle();
        if (optionsRead)
        {
            writeLog(Message111, fileName.toUtf8().constData());
        }
        else
        {
            writeLog(Message112, fileName.toUtf8().constData());

            _settings->writeBundle();
        }
    }
}

void ThreadMainDaemon::onThreadFinishing()
{
    writeLog(Message101, _applicationName.toUtf8().constData());
    snapshotThreads(true);
}

void ThreadMainDaemon::onThreadFinished()
{
    writeLog(Message102, _applicationName.toUtf8().constData());
    snapshotThreads(true);
}

void ThreadMainDaemon::onIdle()
{
}

bool ThreadMainDaemon::accumulateStatistic()
{
    bool result = ThreadBase::accumulateStatistic();

    snapshotThreads();

    return result;
}

SettingsBundle::Ptr ThreadMainDaemon::settings()
{
    return _settings;
}

void ThreadMainDaemon::snapshotThreads(bool rightNow)
{
    auto tickCount = DateUtils::getTickCount();
    if (!rightNow && tickCount < _nextBuildStatisticTickCount)
        return;

    _nextBuildStatisticTickCount = DateUtils::getNextTickCount(TIMEOUT_BUILD_STATISTIC_MILLISECONDS);

    ListThreads *threads = ListThreads::instance();
    if (!threads)
        return;

    if (!logThread())
        return;

    ListStatistic statisticList = threads->statistic();

    QString statisticString = "";

    long int memoryUsage = 0;
#ifdef Q_OS_LINUX
    rusage ru;
    if (0 == getrusage(RUSAGE_SELF, &ru))
    {
        memoryUsage = ru.ru_maxrss;
        statisticString = QString("Memory usage: %1 Kb\r\n").arg(memoryUsage);
    }
#endif

#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS  pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        memoryUsage = pmc.WorkingSetSize / 1024;
        statisticString = QString("Memory usage: %1 Kb\r\n").arg(pmc.WorkingSetSize / 1024);
    }
#endif

    int messagesCount = MessageBase::referenceCount();
    statisticString += QString("Messages count: %1\r\n")
            .arg(messagesCount);

    for (int i = 0; i < statisticList.count(); i++)
    {
        ThreadStatisticStruct statistic = statisticList.at(i);
        qint64 workingTotalMSecs = (statistic.AliveMsecsSinceEpoch - statistic.StartedMsecsSinceEpoch);
        qint64 workingMSecs = workingTotalMSecs - statistic.WaitMSecsCount;

        QDateTime alive = QDateTime::fromMSecsSinceEpoch(statistic.AliveMsecsSinceEpoch);
        statisticString += QString("%1. (Id: %2) %3 %4 (%5/%6) Queue: %7").
                arg(i + 1).
                arg(statistic.ThreadId).
                arg(statistic.ThreadName).
                arg(alive.toString("dd.MM.yy hh:mm:ss.zzz")).
                arg(workingMSecs).
                arg(workingTotalMSecs).
                arg(statistic.QueueCount)
                + ((statistic.Terminated) ? " Terminated" : "") + "\r\n";
    }

#ifdef Q_OS_LINUX
    setThreadInfo(statisticString);
#endif

    statisticString += "{EOF}" + QString(QDateTime::currentMSecsSinceEpoch() % 8, ' ');

    ThreadBase *logThreadInstance = logThread();
    if (logThreadInstance)
    {
        logThreadInstance->postMessage(
                    std::make_shared<MessageWriteToFile>(_threadStatisticFileName,
                                                         statisticString,
                                                         MessageWriteToFile::WriteMode::Truncate));
    }

    if (rightNow || _nextBuildCommonStatisticTickCount <= tickCount)
    {
        writeLog(Message120, statisticList.count(), memoryUsage, messagesCount);

        _nextBuildCommonStatisticTickCount = DateUtils::getNextTickCount(60000);
    }
}

}}
