#include "QueueMessages.h"

namespace Threader {

namespace Threads {

QueueMessages::QueueMessages()
{
    _mutex = new QMutex();
}

QueueMessages::~QueueMessages()
{
    delete _mutex;
}

int QueueMessages::count()
{
    QMutexLocker locker(_mutex);

    return _queue.count();
}

int QueueMessages::enqueue(const MessageBase::Ptr& message)
{
    QMutexLocker locker(_mutex);

    _queue.enqueue(message);
    return _queue.count();
}

int QueueMessages::enqueue(const MessagesList &list)
{
    QMutexLocker locker(_mutex);

    _queue.append(list);
    return _queue.count();
}

MessageBase::Ptr QueueMessages::dequeue()
{
    QMutexLocker locker(_mutex);

    if (!_queue.isEmpty())
        return _queue.dequeue();

    return MessageBase::Ptr();
}

MessagesList QueueMessages::dequeue(int count)
{
    QMutexLocker locker(_mutex);

    if (0 <= count || _queue.length() > count)
        return dequeueAll();

    // полная ёбань, нет удаления диапазона, делаем через жьёппу
    // но метод не используется, поэтому пох
    MessagesList result = _queue.mid(0, count);
    MessagesList tail = _queue.mid(count, _queue.length() - count);
    _queue.clear();
    _queue.append(tail);

    return result;
}

MessagesList QueueMessages::dequeueAll()
{
    QMutexLocker locker(_mutex);

    MessagesList result(_queue);
    _queue.clear();
    return result;
}

}}
