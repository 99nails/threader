#pragma once

#include "ThreadsCommon.h"
#include "PollingsLinux.h"
#include "PollingsWindows.h"
#include "../threader_global.h"

#include <QObject>

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Threader {

namespace Threads {

enum class ThreadSignals {
    SignalTerminate = 1,
    SignalWakeUp = 2,
    SignalTerminateChildThreads = 3
};

class THREADERSHARED_EXPORT PollerThread : public PollerBase
{
    Q_OBJECT
public:
    explicit PollerThread();
    ~PollerThread() override;

    void sendSignalTerminate();
    void sendSignalTerminateChildThreads();
    void sendSignalWakeUp();

#ifdef Q_OS_WIN
    bool process(Descriptor eventToProcess) override;
#endif
#ifdef Q_OS_LINUX
    bool process(const pollfd &event) override;
#endif
private:
#ifdef Q_OS_LINUX
   Descriptor _pipeDescriptors[2];

   bool receiveSignal(uint8_t &signal);
#endif
#ifdef Q_OS_WIN
   Descriptor _eventTerminate;
   Descriptor _eventTerminateChildThreads;
   Descriptor _eventWakeUp;
#endif

   void sendSignal(ThreadSignals signal);

signals:
   void signalTerminateThread();
   void signalTerminateChildThreads();
   void signalWakeUpThread();
};

}}
