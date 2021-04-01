#include "ThreadBase.h"
#include "ListThreads.h"
#include "MessageLog.h"
#include "ThreadTimer.h"

#include "../Utils/DateUtils.h"

#include <QTextCodec>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace Threader
{

namespace Threads
{

using namespace Threader::Utils;

#define STACK_LEVEL 32

const int ThreadBase::WAIT_RESULT_TIMEOUT         = 0;
const int ThreadBase::WAIT_RESULT_ERROR           = -1;
const int ThreadBase::WAIT_RESULT_NOT_INITIALIZED = -2;

const int ThreadBase::TIMEOUT_COLLECT_TERMINATED_THREADS_MILLISECONDS = 1000;
const int ThreadBase::TIMEOUT_ACCUMULATE_STATISTIC_MILLISECONDS       = 10000;

ThreadBase *ThreadBase::_logThread = nullptr;
int ThreadBase::_logLevel = 9;

ThreadBase::ThreadBase(IMessageSubscriber *parent,
                       const QString &threadName,
                       const ThreadRunMode &threadRunMode)
    : QThread(nullptr)
    , _parentThread(parent)
    , _threadName((threadName.trimmed().isEmpty()) ? QString(metaObject()->className()) : threadName)
    , _thisThreadId(0)
    , _aliveCheckedUtc(QDateTime::currentDateTimeUtc())
    , _isTerminated(false)
    , _waitCount(0)
    , _nextDestroyTerminatedThreadsTickCount(DateUtils::getNextTickCount(TIMEOUT_COLLECT_TERMINATED_THREADS_MILLISECONDS))
    , _nextAccumulateStatisticTickCount(DateUtils::getTickCount())
    , _nextCallIdle(0)

    , _messagesLeftToProcess(0)

    , _threadRunMode(threadRunMode)
    , _eventLoop(nullptr)
    , _timerEventLoop(nullptr)
{
    _pollerThread.moveToThread(this);
    if (ThreadRunMode::EventLoop == _threadRunMode)
        this->moveToThread(this);
}

ThreadBase::~ThreadBase()
{
}

IMessageSubscriber *ThreadBase::parentThread() const
{
    return _parentThread;
}

long ThreadBase::thisThreadId() const
{
    return _thisThreadId;
}

QString ThreadBase::threadName() const
{
    return _threadName;
}

void ThreadBase::setThreadName(const QString &threadName)
{
    _threadName = threadName;
}

ThreadBase::ThreadRunMode ThreadBase::threadRunMode() const
{
    return _threadRunMode;
}

void ThreadBase::setThreadRunMode(const ThreadRunMode &threadRunMode)
{
    if (isRunning())
        return;

    _threadRunMode = threadRunMode;

    if (ThreadRunMode::EventLoop == _threadRunMode)
        this->moveToThread(this);
}

QString ThreadBase::threadClassName()
{
    return QString(metaObject()->className());
}

long ThreadBase::threadId()
{
#ifdef Q_OS_LINUX
    return syscall(SYS_gettid);
#endif
#ifdef Q_OS_WIN
    return GetCurrentThreadId();
#endif
}

qint64 ThreadBase::startedTickCount() const
{
    return _startedTickCount;
}

QDateTime ThreadBase::startedUtc() const
{
    return _startedUtc;
}

bool ThreadBase::isTerminated() const
{
    return _isTerminated;
}

Polling *ThreadBase::polling()
{
    return &_polling;
}

void ThreadBase::postMessage(const MessageBase::Ptr &message)
{
    _queue.enqueue(message);
    signalEventWakeUp();
}

void ThreadBase::postMessages(const MessagesList &messagesList)
{
    _queue.enqueue(messagesList);
    signalEventWakeUp();
}

void ThreadBase::postTerminateEvent()
{
    signalEventTerminate();
}

void ThreadBase::postEventChildThreadTerminated()
{
    signalEventChildThreadTerminated();
}

void ThreadBase::initializeThread()
{
    qsrand(uint(clock()));

    if (ListThreads::instance())
        ListThreads::instance()->registerThread(this);

    _thisThreadId = -1;

    switch (_threadRunMode) {
    case ThreadRunMode::Polling:

        connect(&_pollerThread, &PollerThread::signalTerminateThread,
                this, &ThreadBase::slotTerminateThread, Qt::DirectConnection);
        connect(&_pollerThread, &PollerThread::signalTerminateChildThreads,
                this, &ThreadBase::slotTerminateChildThreads, Qt::DirectConnection);
        connect(&_pollerThread, &PollerThread::signalWakeUpThread,
                this, &ThreadBase::slotWakeUpThread, Qt::DirectConnection);

        _polling.registerPoller(&_pollerThread);

        break;
    case ThreadRunMode::EventLoop:
        _eventLoop = new QEventLoop();
        _eventLoop->moveToThread(this);

        _timerEventLoop = new QTimer();
        _timerEventLoop->moveToThread(this);
        _timerEventLoop->setInterval(static_cast<int>(_timeout));
        connect(_timerEventLoop, &QTimer::timeout,
                this, &ThreadBase::slotTimerEventLoop, Qt::AutoConnection);
        _timerEventLoop->start();


        connect(this, &ThreadBase::signalTerminate,
                this, &ThreadBase::slotTerminateThread, Qt::AutoConnection);

        connect(this, &ThreadBase::signalTerminateChildThreads,
                this, &ThreadBase::slotTerminateChildThreads, Qt::AutoConnection);

        connect(this, &ThreadBase::signalWakeUp,
                this, &ThreadBase::slotWakeUpThread, Qt::AutoConnection);
        break;
    }
}

void ThreadBase::finalizeThread()
{
    if (ListThreads::instance())
        ListThreads::instance()->unregisterThread(this);

    if (_eventLoop)
        delete _eventLoop;

    if (_timerEventLoop)
        delete _timerEventLoop;
}

void ThreadBase::onThreadStarted() {}

void ThreadBase::onThreadFinishing() {}

void ThreadBase::onThreadFinished() {}

void ThreadBase::onIdle() {}

void ThreadBase::onBeforeWaitEvents() {}

void ThreadBase::onAfterWaitEvents()
{
    if (ThreadRunMode::EventLoop == _threadRunMode)
        onIdle();
    destroyTerminatedChildThreads(false);

    accumulateStatistic();
}

void ThreadBase::run()
{
    initializeThread();

    _thisThreadId = threadId();

    _startedTickCount = DateUtils::getTickCount();
    _startedUtc = QDateTime::currentDateTimeUtc();

    onThreadStarted();

    MESSAGE_TEMPLATE(50, 0, Debug, "Идентификатор потока [%s]: %d");

    writeLog(Message50, STRLOG(threadName()), threadId());

    startChildThreads();

    if (ThreadRunMode::Polling == _threadRunMode)
    {
        while (!isTerminated())
        {
            onBeforeWaitEvents();

            waitEvents();

            onAfterWaitEvents();
        }
    }
    else
    {
        _eventLoop->exec();
    }

    onThreadFinishing();

    terminateChildThreads();

    processMessages();

    onThreadFinished();

    finalizeThread();

    auto *ownerThread = parentThread();
    if (ownerThread)
        ownerThread->postEventChildThreadTerminated();
}

void ThreadBase::terminateThread()
{
    _isTerminated = true;
}

bool ThreadBase::accumulateStatistic()
{
    if (DateUtils::getTickCount() <= _nextAccumulateStatisticTickCount)
        return false;

    _nextAccumulateStatisticTickCount =
            DateUtils::getNextTickCount(TIMEOUT_ACCUMULATE_STATISTIC_MILLISECONDS);

    ListThreads *list = ListThreads::instance();
    if (!list)
        return false;

    list->accumulateStatistic(this, _thisThreadId, _threadName,
                              _startedUtc.toLocalTime().toMSecsSinceEpoch(),
                              QDateTime::currentDateTime().toMSecsSinceEpoch(),
                              quint64(polling()->waitCount()),
                              _queue.count(),
                              isTerminated());
    return true;
}

int ThreadBase::childthreadsCount()
{
    return _childThreadsList.count();
}

ThreadBase *ThreadBase::childThread(int index)
{
    return _childThreadsList.at(index);
}

ThreadBase *ThreadBase::registerAndStartChildThread(ThreadBase *thread)
{
    if (!thread)
        return nullptr;

    _childThreadsList.append(thread);
    if (!thread->isRunning())
        thread->start();

    return thread;
}

bool ThreadBase::terminateChildThread(ThreadBase *thread)
{
    if (!thread)
        return false;

    if (!thread->isFinished())
    {
        thread->postTerminateEvent();
        while (!thread->isFinished())
        {
            processMessages();
        }

        accumulateStatistic();
    }
    processMessages();
    return true;
}

void ThreadBase::startChildThreads()
{
}

void ThreadBase::terminateChildThreads()
{
    for (int i = _childThreadsList.count() - 1; i >= 0; i--)
        terminateChildThread(_childThreadsList[i]);
    for (int i = _childThreadsList.count() - 1; i >= 0; i--)
        delete _childThreadsList[i];
}

void ThreadBase::onDestroyTerminatedChildThread(ThreadBase *thread)
{
    Q_UNUSED(thread)
}

void ThreadBase::destroyTerminatedChildThreads(bool rightNow)
{
    if (rightNow || _nextDestroyTerminatedThreadsTickCount <= DateUtils::getTickCount())
    {
        for (int i = _childThreadsList.count() - 1; i >= 0; i--)
        {
            ThreadBase *thread = _childThreadsList[i];
            if (thread->isFinished())
            {
                onDestroyTerminatedChildThread(thread);
                delete thread;
                _childThreadsList.removeAt(i);
            }
        }
        _nextDestroyTerminatedThreadsTickCount = DateUtils::getNextTickCount(
                    TIMEOUT_COLLECT_TERMINATED_THREADS_MILLISECONDS);
    }
}

int ThreadBase::messagesLeftToProcess()
{
    return _messagesLeftToProcess;
}

bool ThreadBase::processMessage(const MessageBase::Ptr &)
{
    return false;
}

void ThreadBase::processMessages()
{
    const MessagesList messages = _queue.dequeueAll();
    _messagesLeftToProcess = messages.count();

    onProcessMessagesStarted();

    for (int i = 0; i < messages.count(); i++)
    {
        _messagesLeftToProcess--;
        MessageBase::Ptr message = messages[i];
        processMessage(message);

        accumulateStatistic();
    }

    onProcessMessagesFinished();

    accumulateStatistic();
}

void ThreadBase::waitEvents()
{
    waitEvents(_timeout);
}

void ThreadBase::waitEvents(uint timeout)
{
    // ожидание голосования
    // результаты положительного голосования обрабатыватся внутри функции
    // и вызываются соответствующие слоты
    // остальные результаты отдаются на обработку снаружи
    auto pollResult = _polling.poll(timeout);

    // если произошла ошибка
    if (pollResult < 0)
    {
        processError("Ошибка ожидания событий", errno);
    }

    // если выход по таймауту или нужно вызывать Idle
    if (0 == pollResult || _nextCallIdle < DateUtils::getTickCount())
    {
        onIdle();
        _nextCallIdle = DateUtils::getNextTickCount(_timeout);
    }
    _aliveCheckedUtc = QDateTime::currentDateTimeUtc();
}

QString ThreadBase::errorString(int errorNo)
{
#ifdef Q_OS_WIN
    if (0 == errorNo)
        errorNo = GetLastError();

    LPSTR messageBuffer = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, errorNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    QTextCodec *codec = QTextCodec::codecForName("CP1251");
    QString result = codec->toUnicode(messageBuffer).trimmed();

    LocalFree(messageBuffer);

    return result;
#endif

#ifdef Q_OS_LINUX
    if (0 == errorNo)
        errorNo = errno;
    QString result = QString(strerror(errorNo));
    return result;
#endif
}

void ThreadBase::processError(const QString &errorTitle, int errorNo)
{
    MESSAGE_TEMPLATE(99999, 0, Error, "%s: %d - %s");
    if (errorNo < 0)
        errorNo = errno;

    writeLog(Message99999, STRLOG(errorTitle), errorNo, strerror(errorNo));
}

void ThreadBase::setLogThread(ThreadBase *thread,
                              int level)
{
    _logThread = thread;
    _logLevel = level;
}

int ThreadBase::logLevel()
{
    return _logLevel;
}

void ThreadBase::setLogLevel(int level)
{
    _logLevel = level;
}

ThreadBase *ThreadBase::logThread()
{
    return _logThread;
}

void ThreadBase::writeLog(MessageLog::Ptr message, IMessageSubscriber *receiver)
{
    if (!message)
        return;
    if (message->level() > _logLevel)
        return;

    if (_logThread)
        receiver = _logThread;
    if (!receiver)
        receiver = _parentThread;
    if (!receiver)
        return;
    receiver->postMessage(message);
}

void ThreadBase::writeLog(IMessageSubscriber *receiver, const MessageLogTemplate messageTemplate, ...)
{
    if (messageTemplate.level > _logLevel)
        return;

    va_list args;
    va_start(args, messageTemplate);
    char resultPtr[1024];
    memset(resultPtr, 0, sizeof(resultPtr));
    auto resultCount = vsnprintf(resultPtr, sizeof(resultPtr), STRLOG(messageTemplate.text), args);
    va_end(args);

    QString message;
    if (resultCount > 0)
    {
        message = QString(resultPtr);
    }
    else
    {
        message = "Не удалось выполнить форматирование строки";
    }
    auto messageLog = std::make_shared<MessageLog>(message,
                                                   messageTemplate.number,
                                                   messageTemplate.level,
                                                   messageTemplate.messageType);
    writeLog(messageLog, receiver);
}

void ThreadBase::writeLog(const MessageLogTemplate messageTemplate, ...)
{
    if (messageTemplate.level > _logLevel || !_logThread)
        return;

    va_list args;
    va_start(args, messageTemplate);
    char resultPtr[1024];
    memset(resultPtr, 0, sizeof(resultPtr));
    auto resultCount = vsnprintf(resultPtr, sizeof(resultPtr), STRLOG(messageTemplate.text), args);
    va_end(args);

    QString messageText;
    if (resultCount > 0)
    {
        messageText = QString(resultPtr);
    }
    else
    {
        messageText = "Не удалось выполнить форматирование строки";
    }
    auto message = std::make_shared<MessageLog>(messageText,
                                                messageTemplate.number,
                                                messageTemplate.level,
                                                messageTemplate.messageType);
    _logThread->postMessage(message);
}

ThreadBase *ThreadBase::registerMessageSubscribers(const QString &messageName,
                                                   const MessageSubscribersVector &subscribers)
{
    _subscribers[messageName] = subscribers;
    return this;
}

ThreadBase *ThreadBase::unregisterMessageSubscribers(const QString &messageName)
{
    if (_subscribers.contains(messageName))
        _subscribers.remove(messageName);
    return this;
}

MessageSubscribersVector ThreadBase::messageSubscribers(const QString &messageName)
{
    if (_subscribers.contains(messageName))
        return _subscribers[messageName];
    return {};
}

QEventLoop *ThreadBase::eventLoop() const
{
    return _eventLoop;
}

ThreadBase *ThreadBase::startTimer(const QString &timerName,
                                   const int &delayMsecs,
                                   const bool multiShot)
{
    auto thread = new ThreadTimer(this, timerName, delayMsecs, multiShot);
    registerAndStartChildThread(thread);
    return thread;
}

bool ThreadBase::stopTimer(const QString &timerName)
{
    bool result = false;

    for (int i = 0; i < _childThreadsList.count(); i++)
    {
        auto thread = dynamic_cast<ThreadTimer*>(_childThreadsList.at(i));
        if (!thread)
            continue;

        if (thread->timerName() == timerName)
        {
            thread->postTerminateEvent();
            result = true;
        }
    }
    return result;
}

int ThreadBase::stopAllTimers()
{
    int result = 0;

    for (int i = 0; i < _childThreadsList.count(); i++)
    {
        auto thread = dynamic_cast<ThreadTimer*>(_childThreadsList.at(i));
        if (!thread)
            continue;

        thread->postTerminateEvent();
        result++;
    }
    return result;
}

void ThreadBase::signalEventWakeUp()
{
    if (isFinished())
        return;

    switch (_threadRunMode) {
    case ThreadRunMode::Polling:
        _pollerThread.sendSignalWakeUp();
        break;
    case ThreadRunMode::EventLoop:
        emit signalWakeUp();
        break;
    }
}

void ThreadBase::signalEventTerminate()
{
    if (isFinished())
        return;

    switch (_threadRunMode) {
    case ThreadRunMode::Polling:
        _pollerThread.sendSignalTerminate();
        break;
    case ThreadRunMode::EventLoop:
        emit signalTerminate();
        break;
    }
}

void ThreadBase::signalEventChildThreadTerminated()
{
    if (isFinished())
        return;

    switch (_threadRunMode) {
    case ThreadRunMode::Polling:
        _pollerThread.sendSignalTerminateChildThreads();
        break;
    case ThreadRunMode::EventLoop:
        emit signalTerminateChildThreads();
        break;
    }
}

uint ThreadBase::timeout() const
{
    return _timeout;
}

void ThreadBase::setTimeout(uint timeout)
{
    _timeout = timeout;
}

void ThreadBase::slotTerminateThread()
{
    if (ThreadRunMode::EventLoop == _threadRunMode && _eventLoop)
        _eventLoop->exit();

    terminateThread();
}

void ThreadBase::slotTerminateChildThreads()
{
    destroyTerminatedChildThreads(true);
}

void ThreadBase::slotWakeUpThread()
{
    if (threadRunMode() == ThreadRunMode::EventLoop)
    {
        msleep(0);
    }
    processMessages();
}

void ThreadBase::slotTimerEventLoop()
{
    onAfterWaitEvents();
}


}}
