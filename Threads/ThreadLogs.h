#pragma once

#include <QDate>
#include <QFile>
#include <QObject>

#include "MessageWriteToFile.h"

#include "MessageBase.h"
#include "MessageLog.h"
#include "ThreadBase.h"
#include "WriterLogs.h"

#include "../threader_global.h"


namespace Threader {

namespace Threads {

/**
 * @brief ThreadLogsAndFilesWriter - Класс записи в файл сообщений протоколирования,
 * файлов очередей и слепков состояния приложения. Поддерживается буферизация данных.
 */
class THREADERSHARED_EXPORT ThreadLogs : public ThreadBase,
        public WriterLog
{
    Q_OBJECT
public:
    explicit ThreadLogs(bool doConsoleOutput,
                        IMessageSubscriber *subscriber = nullptr);

    void flush();

    static void setLogLevel(const int &level);

protected:
    void onThreadStarted() override;
    void onThreadFinishing() override;
    void onThreadFinished() override;
    void onIdle() override;
    bool processMessage(const MessageBase::Ptr &message) override;

private:
    void logFlushBuffer() override;
    void logCheckAndFlushBuffer();

    qint64 _nextWriteFileTickCount;
    IMessageSubscriber *_subscriber;
};


}}
