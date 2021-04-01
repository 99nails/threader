#include "ThreadLogs.h"

#include "LogMessagesTemplates.h"

#include "../Utils/DateUtils.h"

#include <QDir>

namespace Threader {

namespace Threads {


using namespace Threader::Utils;


class MessageLogLevel : public MessageBase
{
public:
    static const QString MESSAGE_NAME;

    explicit MessageLogLevel(const int &level)
        : MessageBase(MESSAGE_NAME)
        , _level(level)
    {
    }

    int level() const
    {
        return _level;
    }

private:
    int _level;
};

const QString MessageLogLevel::MESSAGE_NAME = "Message.Log.Level";


#define THREAD_NAME_LOG "Thread.Log"
#define PATH_TO_LOG "Log"
#define LOG_BUFFER_MAXIMIUM_SIZE (1024 * 32)
#define TIMEOUT_BUFFER_FLUSH_MILLISECONDS 1000


ThreadLogs::ThreadLogs(bool doConsoleOutput,
                       IMessageSubscriber *subscriber)
    : ThreadBase(nullptr, THREAD_NAME_LOG)
    , WriterLog(doConsoleOutput, "./Log/", 9)
    , _nextWriteFileTickCount(DateUtils::getNextTickCount(TIMEOUT_BUFFER_FLUSH_MILLISECONDS))
    , _subscriber(subscriber)
{
    // регистрация в качестве обработчика сообщений протоколирования
    setLogThread(this, getLevel());
    setTimeout(1000);
}

void ThreadLogs::flush()
{
    logFlushBuffer();
}

void ThreadLogs::setLogLevel(const int &level)
{
    auto *logThread = dynamic_cast<ThreadLogs*>(ThreadBase::logThread());
    if (logThread)
        logThread->postMessage(std::make_shared<MessageLogLevel>(level));
}

void ThreadLogs::onThreadStarted()
{
    doMessageLog(std::make_shared<MessageLog>(Message0));
}

void ThreadLogs::onThreadFinishing()
{
    doMessageLog(std::make_shared<MessageLog>(Message1));
}

void ThreadLogs::onThreadFinished()
{
    doMessageLog(std::make_shared<MessageLog>(Message2));

    // сброс накопленных данных на диск
    logFlushBuffer();

    // освобождение файла
    _currentLogFile.close();
}

void ThreadLogs::onIdle()
{
    logCheckAndFlushBuffer();
}

bool ThreadLogs::processMessage(const MessageBase::Ptr &message)
{
    QString messageName = message->name();
    if (MessageLog::MESSAGE_NAME == messageName)
    {
        auto messageLog = std::dynamic_pointer_cast<MessageLog>(message);
        if (messageLog)
        {
            doMessageLog(messageLog);
            if (_subscriber && getLevel() >= messageLog->level())
                _subscriber->postMessage(messageLog);
        }
    }
    else if (MessageWriteToFile::MESSAGE_NAME_WRITE_TO_FILE == messageName)
    {
        doMessageRewriteFile(std::dynamic_pointer_cast<MessageWriteToFile>(message));
    }
    else if (MessageLogLevel::MESSAGE_NAME == messageName)
    {
        auto messageLogLevel = std::dynamic_pointer_cast<MessageLogLevel>(message);
        if (messageLogLevel)
            setLevel(messageLogLevel->level());
    }

    logCheckAndFlushBuffer();

    return true;
}

void ThreadLogs::logFlushBuffer()
{
    WriterLog::logFlushBuffer();
    // вычисление следующего времени сброса накопленных данных
    _nextWriteFileTickCount = DateUtils::getNextTickCount(TIMEOUT_BUFFER_FLUSH_MILLISECONDS);
}

void ThreadLogs::logCheckAndFlushBuffer()
{
    if (_logBuffer.isEmpty())
        return;

    if (DateUtils::getTickCount() < _nextWriteFileTickCount)
        return;

    logFlushBuffer();
}

}}
