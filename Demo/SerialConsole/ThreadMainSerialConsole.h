#pragma once

#include "ThreadMainDaemon.h"

#include <QObject>

using namespace Threader::Threads;

class ThreadMainSerialConsole : public ThreadMainDaemon
{
    Q_OBJECT
public:
    explicit ThreadMainSerialConsole();

protected:
    void startChildThreads() override;

private:
    ThreadBase *_threadSerialPort;
};

