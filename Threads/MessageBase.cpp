#include "MessageBase.h"

#ifdef Q_OS_LINUX
#include "ThreadBase.h"
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Threader {

namespace Threads {

int MessageBase::_referenceCount(0);

QMutex MessageBase::_mutexReferenceCount;

MessageBase::MessageBase(const QString &name)
    : _created(QDateTime::currentDateTime())
    , _name(name)
    #ifdef Q_OS_LINUX
    , _threadId(ThreadBase::threadId())
    #endif
    #ifdef Q_OS_WIN
    , _threadId(GetCurrentThreadId())
    #endif
{
    incrementReferenceCount();
}

MessageBase::~MessageBase()
{
    decrementReferenceCount();
}

QDateTime MessageBase::created()
{
    return _created;
}

QString MessageBase::name() const
{
    return _name;
}

long MessageBase::threadId() const
{
    return _threadId;
}

int MessageBase::referenceCount()
{
    QMutexLocker locker(&_mutexReferenceCount);
    return _referenceCount;
}

void MessageBase::incrementReferenceCount()
{
    QMutexLocker locker(&_mutexReferenceCount);
    MessageBase::_referenceCount++;
}

void MessageBase::decrementReferenceCount()
{
    QMutexLocker locker(&_mutexReferenceCount);
    _referenceCount--;
}

void MessageBase::setCreated(const QDateTime &created)
{
    _created = created;
}

}}
