#include "PollingsWindows.h"

#ifdef Q_OS_WIN

#include "../Utils/DateUtils.h"

namespace Threader {

namespace Threads {

using namespace Threader::Utils;

PollerBase::PollerBase(const DescriptorsVector &events)
    : QObject(nullptr)
    , _events(events)
{
}

PollerBase::~PollerBase()
{
    for (auto descriptor : _events)
        CloseHandle(descriptor);
}

bool PollerBase::hasEvent(Descriptor event)
{
    for (auto descriptor : _events)
        if (descriptor == event)
            return true;
    return false;
}

DescriptorsVector *PollerBase::events()
{
    return &_events;
}

void PollerBase::assign(const DescriptorsVector &events)
{
    _events.clear();
    _events.append(events);
}

bool PollerBase::process(Descriptor eventToProcess)
{
    bool result = hasEvent(eventToProcess);
    if (result)
        emit signalOnPollEvent(this);
    return result;
}

Polling::Polling()
    : _waitCount(0)
{
}

int Polling::pollersCount()
{
    return _pollers.count();
}

int Polling::registerPoller(PollerBase *poller)
{
    // проверка существования голосующего с добавляемым дескриптором
    int index = _pollers.indexOf(poller);

    // если голосующий найден
    if (index < 1)
    {
        // добавление нового голосущего
        _pollers.append(poller);
    }

    // получение количества голосующих
    int count = _pollers.count();

    return count;
}

int Polling::unregisterPoller(PollerBase *poller)
{
    // удаление голосующего
    _pollers.removeAll(poller);

    // получение количества голосующих
    int count = _pollers.count();

    return count;
}

int Polling::poll(const uint32_t &timeout)
{
    // заполнение массива с данными голосующих
    DescriptorsVector pollArray;
    for (auto poller: _pollers)
        pollArray.append(*poller->events());
    auto pollArrayCount = static_cast<uint32_t>(pollArray.count());

    // запоминание начала ожидания событий
    auto waitStarted = DateUtils::getTickCount();

    // ожидание голосования
    auto waitResult = WaitForMultipleObjects(pollArrayCount,
                                             pollArray.constData(),
                                             false,
                                             timeout);

    // фиксация общего времени ожидания событий
    waitStarted = DateUtils::getTickCount() - waitStarted;
    _waitCount = _waitCount + waitStarted;

    // выход по таймауту
    if (WAIT_TIMEOUT == waitResult)
        return 0;

    // выход по ошибке
    if (WAIT_FAILED == waitResult)
        return -1;

    if (pollArrayCount > waitResult)
    {
        // получение дескриптора события
        int eventIndex = waitResult - WAIT_OBJECT_0;
        Descriptor eventToProcess = pollArray.at(eventIndex);
        // сброс события раньше был здесь
        ResetEvent(eventToProcess);
        // обработка события в голосующем
        for (auto poller : _pollers)
            if (poller->hasEvent(pollArray.at(eventIndex)))
                poller->process(eventToProcess);

        // сброс события теперь здесь после обработки события
        // ResetEvent(eventToProcess);
    }

    return 1;
}

qint64 Polling::waitCount() const
{
    return _waitCount;
}

}}

#endif
