#pragma once

#include "MessageBase.h"

#include "MessageLog.h"
#include "MessageWriteToFile.h"

#include "../threader_global.h"

#include <QFile>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Threader {

namespace Threads {


class THREADERSHARED_EXPORT WriterLog
{
public:
    explicit WriterLog(bool doConsoleOutput,
                       const QString &pathToLog,
                       int logLevel);
    virtual ~WriterLog();

    static QString getLogFileName(const QString &pathToLog,
                                  const QString &applicationName,
                                  const QDate &logFileDate);
protected:
    QString logCheckPath();
    void logPrint(const MessageLog::Ptr& message);
    void logRecreateFile();
    virtual void logFlushBuffer();
    void doMessageLog(const MessageLog::Ptr& message);
    void doMessageRewriteFile(const MessageWriteToFile::Ptr& message);
    int getLevel();
    void setLevel(const int &level);

protected:
    QFile _currentLogFile;
    QString _logBuffer;

private:
    bool _doConsoleOutput;
    QString _logFilePrefix;
    QDate _currentLogFileDate;
    QString _pathToLog;
    int _logLevel;
#ifdef Q_OS_WIN
    HANDLE _consoleHandle;
    QTextCodec *_codec;
#endif
};


}}
