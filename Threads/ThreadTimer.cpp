#include "ThreadTimer.h"

#include "MessageTimer.h"

namespace Threader
{

namespace Threads
{


const QString ThreadTimer::THREAD_NAME_PREFFIX = "ThreadTimer.";


ThreadTimer::ThreadTimer(IMessageSubscriber *parent,
                         const QString &timerName,
                         const int &delayMsecs,
                         const bool multiShot)
    : ThreadBase(parent, THREAD_NAME_PREFFIX + timerName, ThreadRunMode::Polling)
    , _timerName(timerName)
    , _delay(delayMsecs)
    , _multiShot(multiShot)
    , _shotCount(0)
{
    Q_ASSERT(parent);
    setTimeout(uint(_delay));
}

QString ThreadTimer::timerName() const
{
    return _timerName;
}

int ThreadTimer::delay() const
{
    return _delay;
}

bool ThreadTimer::multiShot() const
{
    return _multiShot;
}

void ThreadTimer::onIdle()
{
    if (!_multiShot)
        terminateThread();
    sendTimerMessage();
}

void ThreadTimer::sendTimerMessage()
{
    if (!parentThread())
        return;
    _shotCount++;
    parentThread()->postMessage(std::make_shared<MessageTimer>(_timerName, _shotCount));
}

}}

