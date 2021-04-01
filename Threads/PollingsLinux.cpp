#include "PollingsLinux.h"

#ifdef Q_OS_LINUX

#include "../Utils/DateUtils.h"
//#include <QDateTime>

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


PollerBase::PollerBase(const int &descriptor, const short &events)
    : QObject(nullptr)
    , _descriptor(descriptor)
    , _events(events)
{
}

int PollerBase::descriptor() const
{
    return _descriptor;
}

void PollerBase::setDescriptor(const Descriptor &descriptor)
{
    _descriptor = descriptor;
}

short int PollerBase::events()
{
    return _events;
}

void PollerBase::setEvents(short int events)
{
    _events = events;
}

void PollerBase::assign(pollfd &event)
{
    event.fd = _descriptor;
    event.events = _events;
    event.revents = 0;
}


bool PollerBase::process(const pollfd &event)
{
    bool result = event.fd == _descriptor;
    if (result)
    {
        emit signalOnPollEvent(this, event);
    }
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
    int index = indexOfDescriptor(poller->descriptor());

    // если дескриптор найден
    if (index >= 0)
    {
        // замена голосующего
        _pollers[index] = poller;
    }
    else
    {
        // добавление нового голосущего
        _pollers.append(poller);
    }

    // получение количества голосующих
    int count = _pollers.count();

    // поддержание размерности массива структур с данными голосующих
    _pollStructArray.resize(count);

    return count;
}

int Polling::unregisterPoller(PollerBase *poller)
{
    // удаление голосующего
    _pollers.removeAll(poller);

    // получение количества голосующих
    int count = _pollers.count();

    // поддержание размерности массива структур с данными голосующих
    _pollStructArray.resize(count);

    return count;
}

int Polling::poll(const uint32_t &timeout)
{
    // получение количества голосующих
    int count = _pollStructArray.count();

    // проверка существования голосующих
    if (0 == count)
        return 0;

    // формирование ссылки на массив структур голосования
    pollfd *pollArrayPointer = _pollStructArray.data();

    // заполнение массива структур с данными голосующих
    for (int i = 0; i < count; i++)
        _pollers.at(i)->assign(pollArrayPointer[i]);

    int result;
    // голосование
    do
    {
        // запоминание начала ожидания
        qint64 pollStart = DateUtils::getTickCount();
        // ожидание
        result = ::poll(pollArrayPointer,
                        static_cast<uint32_t>(count),
                        static_cast<int32_t>(timeout));
        // вычисление времени в ожидании
        _waitCount += DateUtils::getTickCount() - pollStart;
        // может прийти сигнал и тогда ошибка будет EINTR
        // в этом случае ожидание продолжается
    } while (result < 0 && EINTR == errno);

    // обработка результатов
    if (result != 0)
        for (int i = 0; i < count; i++)
        {
            // если произошло событие
            if (pollArrayPointer[i].revents != 0)
                // обработка события голосующим
                _pollers.at(i)->process(pollArrayPointer[i]);
        }

    return result;
}

qint64 Polling::waitCount() const
{
    return _waitCount;
}

int Polling::indexOfDescriptor(const int &descriptor)
{
    for (int i = 0; i < _pollers.count(); i++)
        if (_pollers.at(i)->descriptor() == descriptor)
            return i;
    return -1;
}

}}

#endif
