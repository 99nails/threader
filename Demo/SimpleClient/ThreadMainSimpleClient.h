#pragma once

#include <QObject>

#include "../../Threads/ThreadMainDaemon.h"

using namespace Threader::Threads;

class ThreadMainSimpleClient : public ThreadMainDaemon
{
    Q_OBJECT
public:
    explicit ThreadMainSimpleClient();

protected:
    void startChildThreads() override;

private:
    ThreadBase *_threadClient;
};
