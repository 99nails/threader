#pragma once

#include "../threader_global.h"

#include <QDateTime>
#include <QVector>

namespace Threader {

namespace Utils {


struct TrafficHistory
{
    TrafficHistory(const QDateTime &received = QDateTime(),
                   const quint64 &count = 0)
        : Received(received.toMSecsSinceEpoch())
        , Count(count)
    {
    }

    quint64 Received;
    quint64 Count;
};

using TrafficHistoryList = QVector<TrafficHistory>;


class THREADERSHARED_EXPORT TrafficCounter
{
public:
    explicit TrafficCounter(int secondsToSore = 5);
    void clear();
    quint64 append(const QDateTime &dateTime,
                   const quint64 &count);
    quint64 actualize();
    quint64 currentSpeed() const;

    int secondsToStore() const;
    void setSecondsToStore(int secondsToStore);

    quint64 total() const;

private:
    int _secondsToStore;
    quint64 _total;
    quint64 _currentSpeed;
    TrafficHistoryList _historyList;
};

}}
