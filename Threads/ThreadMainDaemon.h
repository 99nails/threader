#pragma once

#include "SettingsBase.h"
#include "ThreadBase.h"

#include "../threader_global.h"

#include <QObject>

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT ThreadMainDaemon : public ThreadBase
{
    Q_OBJECT
public:
    explicit ThreadMainDaemon(const QString &threadName = "",
                              const QString &serviceName = "",
                              const QString &applicationName = "",
                              const SettingsBundle::Ptr &settings = nullptr);
    ~ThreadMainDaemon() override;

    QString serviceName() const;

    QString applicationName() const;

    static bool runConsole();
    static void setRunConsole(bool runConsole);

protected:
    void onThreadStarted() override;
    void onThreadFinishing() override;
    void onThreadFinished() override;
    void onIdle() override;
    bool accumulateStatistic() override;

    SettingsBundle::Ptr settings();
private:
    void snapshotThreads(bool rightNow = false);

    QString _serviceName;
    QString _applicationName;
    SettingsBundle::Ptr _settings;

    qint64 _nextBuildStatisticTickCount;
    qint64 _nextBuildCommonStatisticTickCount;
    QString _threadStatisticFileName;

    static bool _runConsole;

};

}
}
