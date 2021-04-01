#include "MessageTimer.h"

namespace Threader {

namespace Threads {


MessageTimer::MessageTimer(const QString &name,
                           const qint64 &shotCount)
    : MessageBase(name)
    , _shotCount(shotCount)
{
}

qint64 MessageTimer::shotCount() const
{
    return _shotCount;
}

}}
