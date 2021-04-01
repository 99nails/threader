#include "WriterLogs.h"

#include "LogMessagesTemplates.h"
#include "../Utils/DateUtils.h"

#include <QCoreApplication>
#include <QDir>
#include <QTextCodec>

#include <iostream>


namespace Threader {

namespace Threads {

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


WriterLog::WriterLog(bool doConsoleOutput,
                     const QString &pathToLog,
                     int logLevel)
    : _logBuffer("")
    , _doConsoleOutput(doConsoleOutput)
    , _logLevel(logLevel)
{
    // инициализация нулевого времени
    _currentLogFileDate.setDate(2014, 3, 21);
    // префикс имени файла протокола
    _logFilePrefix = QCoreApplication::applicationName();
    // чтение настроек протоколирования

    // формирование пути к файлам
    QDir path = QDir(pathToLog);
    _pathToLog = path.absolutePath();
#ifdef Q_OS_WIN
    _consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    _codec = QTextCodec::codecForName("cp866");
#endif
}

WriterLog::~WriterLog()
{
#ifdef Q_OS_WIN
    if (_codec)
    {
        _codec = nullptr;
    }
#endif
}

QString WriterLog::getLogFileName(const QString &pathToLog,
                                  const QString &applicationName,
                                  const QDate &logFileDate)
{
    return  pathToLog + QDir::separator() + applicationName +
            logFileDate.toString("-yy.MM.dd") + ".log";
}

QString WriterLog::logCheckPath()
{
    QDir logsDir(_pathToLog);
    if (!logsDir.exists())
    {
        logsDir.mkpath(".");
    }

    return _pathToLog;
}

void WriterLog::logPrint(const MessageLog::Ptr &message)
{
    QString messageString = message->toString() + "\r\n";

#ifdef Q_OS_LINUX
    QString colorString;
    switch (message->messageType()) {
    case Warning:
        colorString = ANSI_COLOR_YELLOW;
        break;
    case Error:
        colorString = ANSI_COLOR_RED;
        break;
    case Debug:
        colorString = ANSI_COLOR_BLUE;
        break;
    default:
        colorString = ANSI_COLOR_RESET;
        break;
    }
    std::cout << colorString.toStdString() << messageString.toStdString();
#endif
#ifdef Q_OS_WIN
    uint8_t color;
    switch (message->messageType()) {
    case Message:
        color = 15;
        break;
    case Warning:
        color = 14;
        break;
    case Error:
        color = 12;
        break;
    case Debug:
        color = 6;
        break;
    default:
        color = 15;
    }
    if (INVALID_HANDLE_VALUE != _consoleHandle)
        SetConsoleTextAttribute(_consoleHandle, color);

    if (_codec)
    {
        QByteArray dosString = _codec->fromUnicode(messageString.toUtf8());
        std::cout << dosString.constData();
    }
#endif
}

void WriterLog::logRecreateFile()
{
    // закрытие файла, если файл открыт
    if (_currentLogFile.isOpen())
        _currentLogFile.close();

    // создание подкаталога
    auto pathToLog = logCheckPath();

    // формирование имени файла
    auto logFileName = getLogFileName(pathToLog, QCoreApplication::applicationName(), _currentLogFileDate);

    // установка нового имени
    _currentLogFile.setFileName(logFileName);
    // создание и открытие файла
    _currentLogFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);
}

void WriterLog::logFlushBuffer()
{
    // если нечего сбрасывать или файл закрыт, то курить бамбук
    if (_logBuffer.isEmpty() || !_currentLogFile.isOpen())
        return;

    // перевод в UTF8
    auto bufferUtf8 = _logBuffer.toUtf8();
    qint64 bufferSize = bufferUtf8.size();
    // запись накопленных данных
    qint64 writtenCount = _currentLogFile.write(bufferUtf8);

    // обработка
    if (bufferSize > writtenCount)
    {
        // удаление из буфера записанных данных
        _logBuffer.remove(0, int(writtenCount));
        // повторное открытие файла
        logRecreateFile();
        // запись оставшихся данных
        _currentLogFile.write(_logBuffer.toUtf8());

        doMessageLog(std::make_shared<MessageLog>(Message3));
    }
    // сброс буфера файла на диск
    _currentLogFile.flush();
    // очистка накопленных данных
    _logBuffer.clear();
}

void WriterLog::doMessageLog(const MessageLog::Ptr &message)
{
    if (!message)
        return;

    if (message->level() > _logLevel)
        return;

    // при установленном выводе на консоль
    if (_doConsoleOutput)
        // вывод на консоль
        logPrint(message);

    // получение даты сообщения протоколирования
    QDate currentMessageDate = message->created().date();

    // если дата сообщения не совпадает с датой предыдущего сообщения
    if (_currentLogFileDate != currentMessageDate)
    {
        // сброс накопленного буфера
        logFlushBuffer();

        // запоминание новой даты
        _currentLogFileDate = currentMessageDate;

        // формирование нового файла или открытие существующего файла
        logRecreateFile();
    }

    // накопление сообщений в буфер
    _logBuffer.append(message->toString());
    _logBuffer.append("\r\n");
}

void WriterLog::doMessageRewriteFile(const MessageWriteToFile::Ptr &message)
{
    if (!message)
        return;

    // создание подкаталога
    QString pathToLog = logCheckPath();

    // формирование имени файла
    QString fileName = pathToLog + QDir::separator() + message->fileName();
    // установка имени файла
    QFile file(fileName);

    QFile::OpenMode openMode = (message->writeMode() == MessageWriteToFile::WriteMode::Truncate)
            ? QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate
            : QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append;
    // создание и открытие файла
    if (file.open(openMode))
    {
        if (message->writeMode() == MessageWriteToFile::WriteMode::Append)
            file.seek(file.size());
        // запись данных
        file.write(message->text().toUtf8());
        // закрытие файла
        file.flush();
        file.close();
        //std::cout << message->text().toStdString();
    }
}

int WriterLog::getLevel()
{
    return _logLevel;
}

void WriterLog::setLevel(const int &level)
{
    _logLevel = level;
}

}}
