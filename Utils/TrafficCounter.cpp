#include "TrafficCounter.h"

namespace Threader {

namespace Utils {

TrafficCounter::TrafficCounter(int secondsToSore)
    : _secondsToStore(secondsToSore)
    , _total(0)
{    
}

void TrafficCounter::clear()
{
    _total = 0;
    _historyList.clear();
}

quint64 TrafficCounter::append(const QDateTime &dateTime,
                            const quint64 &count)
{
    _total += count;
    _historyList.append(TrafficHistory(dateTime, count));

    return actualize();
}

quint64 TrafficCounter::currentSpeed() const
{
    return _currentSpeed;
}

int TrafficCounter::secondsToStore() const
{
    return _secondsToStore;
}

void TrafficCounter::setSecondsToStore(int secondsToStore)
{
    _secondsToStore = secondsToStore;
}

quint64 TrafficCounter::total() const
{
    return _total;
}

quint64 TrafficCounter::actualize()
{
    quint64 limit = QDateTime::currentDateTime().addSecs(-_secondsToStore).toMSecsSinceEpoch();
    
    int removeCount = 0;
    quint64 trafficCount = 0;
    for (int i = 0; i < _historyList.count(); i++)
    {
        if (_historyList.at(i).Received < limit)
        {
            removeCount++;
        }
        else
        {
            trafficCount += _historyList.at(i).Count;
        }
    }

    if (removeCount)
        _historyList.remove(0, removeCount);

    _currentSpeed = trafficCount / _secondsToStore;

    return _currentSpeed;
}

}}
