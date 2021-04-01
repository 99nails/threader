#include "PollerThread.h"

namespace Threader {

namespace Threads {

PollerThread::PollerThread()
    : PollerBase()
{
#ifdef Q_OS_LINUX
    memset(_pipeDescriptors, 0, sizeof (_pipeDescriptors));
    if (::pipe2(_pipeDescriptors, O_NONBLOCK) == 0)
    {
        setDescriptor(_pipeDescriptors[0]);
        setEvents(POLLIN);
    }
#endif

#ifdef Q_OS_WIN
    _eventTerminate = CreateEvent(nullptr, true, false, nullptr);
    _eventTerminateChildThreads = CreateEvent(nullptr, true, false, nullptr);
    _eventWakeUp = CreateEvent(nullptr, true, false, nullptr);
    assign({_eventTerminate, _eventTerminateChildThreads, _eventWakeUp});
#endif
}

PollerThread::~PollerThread()
{
#ifdef Q_OS_LINUX
    close(_pipeDescriptors[0]);
    close(_pipeDescriptors[1]);
#endif
}

void PollerThread::sendSignalTerminate()
{
    sendSignal(ThreadSignals::SignalTerminate);
}

void PollerThread::sendSignalTerminateChildThreads()
{
    sendSignal(ThreadSignals::SignalTerminateChildThreads);
}

void PollerThread::sendSignalWakeUp()
{
    sendSignal(ThreadSignals::SignalWakeUp);
}

#ifdef Q_OS_WIN
bool PollerThread::process(Descriptor eventToProcess)
{
    bool result = hasEvent(eventToProcess);
    if (result)
    {
        ResetEvent(eventToProcess);

        if (eventToProcess == _eventTerminate)
        {
            emit signalTerminateThread();
        }
        else if (eventToProcess == _eventTerminateChildThreads)
        {
            emit signalTerminateChildThreads();
        }
        else if (eventToProcess == _eventWakeUp) {
            emit signalWakeUpThread();
        }

    }
    return result;
}
#endif

#ifdef Q_OS_LINUX
bool PollerThread::process(const pollfd &event)
{
    // если событие чужое
    if (event.fd != descriptor())
        return false;

    // если не надо читать из пайпа
    if (!(event.revents & POLLIN))
        return false;

    uint8_t signal;
    if (!receiveSignal(signal))
        return false;

    switch (static_cast<ThreadSignals>(signal)) {
    case ThreadSignals::SignalTerminate:
        emit signalTerminateThread();
        break;
    case ThreadSignals::SignalWakeUp:
        emit signalWakeUpThread();
        break;
    case ThreadSignals::SignalTerminateChildThreads:
        emit signalTerminateChildThreads();
        break;
    }

    return true;
}

bool PollerThread::receiveSignal(uint8_t &signal)
{
    return read(_pipeDescriptors[0], &signal, sizeof(signal));
}
#endif


void PollerThread::sendSignal(ThreadSignals signal)
{
#ifdef Q_OS_LINUX
    // приведение к 8-ми битовому значению, т.к. sizeof(ThreadSignals) == 4
    uint8_t signalInt = static_cast<uint8_t>(signal);
    auto res = write(_pipeDescriptors[1], &signalInt, sizeof(signalInt));
    Q_UNUSED(res)
#endif
#ifdef Q_OS_WIN
    switch (signal) {
    case ThreadSignals::SignalTerminate:
        SetEvent(_eventTerminate);
        break;
    case ThreadSignals::SignalTerminateChildThreads:
        SetEvent(_eventTerminateChildThreads);
        break;
    case ThreadSignals::SignalWakeUp:
        SetEvent(_eventWakeUp);
        break;
    }
#endif
}

}}
