#pragma once

#include <QObject>

#include "ThreadMainDaemon.h"

using namespace Threader::Threads;

class ThreadMainSimpleServer : public ThreadMainDaemon
{
    Q_OBJECT
public:
    explicit ThreadMainSimpleServer();

protected:
    void startChildThreads() override;

private:
    ThreadBase *_threadListen;
};
