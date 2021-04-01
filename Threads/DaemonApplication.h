#pragma once

#include "ThreadLogs.h"
#include "../ThirdParty/qtservice/qtservice.h"

#include "../Utils/PosixHandler.h"

#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QFileInfo>

#include <iostream>
#include <memory>

using namespace Threader::Threads;

#define COMMAND_LINE_PARAMETER_CONSOLE "--console"

void readStr()
{
    std::string ch;
    std::cout << "Press Enter to terminate program...\r\n";
    std::getline(std::cin, ch);
    QCoreApplication::quit();
}


class Service : public QtService<QCoreApplication>
{
public:
    explicit Service(int argc, char *argv[], QString &name, ThreadBase *mainThread)
        : QtService<QCoreApplication>(argc, argv, name)
        , _mainThread(mainThread)
    {
        QFileInfo fileInfo(argv[0]);
        _applicationPath = fileInfo.absolutePath();
    }

protected:
    void start() override
    {
        // установка текущего каталога на месторасположение приложения
        QDir::setCurrent(_applicationPath);

        // запуск подсистемы протоколирования
        _logThread = new ThreadLogs(false);
        _logThread->start();

        // запуск основного потока
        _mainThread->start();
    }

    void stop() override
    {
        // остановка основной подсистемы
        if (_mainThread->isRunning())
        {
            // отправка сигнала остановки
            _mainThread->postTerminateEvent();
            // ожидание завершения потока
            while (!_mainThread->isFinished())
            {
                _mainThread->postTerminateEvent();
                QThread::msleep(10);
            }
        }

        // остановка подсистемы протоколирования
        if (_logThread->isRunning())
        {
            // отправка сигнала остановки
            _logThread->postTerminateEvent();
            // ожидание завершения потока
            while (_logThread->isRunning())
            {
                _logThread->postTerminateEvent();
                QThread::msleep(10);
            }
        }
    }

private:
    QString _applicationPath;
    ThreadLogs *_logThread;
    ThreadBase *_mainThread;
};


template<class AppMainThread>
int AppMain(int argc, char *argv[], const QString &serviceName)
{
    if (argc < 1)
        return 0;

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

#ifdef Q_OS_LINUX
    setPosixHandler();
#endif

    QtConcurrent::run(readStr);

    bool runConsole = false;
    for (int i = 0; i < argc; i++)
    {
        QString argument = QString(argv[i]).toLower();
        if (COMMAND_LINE_PARAMETER_CONSOLE == argument)
            runConsole = true;
    }

    AppMainThread::setRunConsole(runConsole);

    // консольный режим
    if (runConsole)
    {
        // создание класса приложения до запуска потоков
        QCoreApplication a(argc, argv);

        // установка текущего каталога на месторасположение приложения
        QFileInfo fileInfo(argv[0]);
        QString path = fileInfo.absolutePath();
        QDir::setCurrent(path);

        // запуск подсистемы протоколирования
        ThreadLogs logThread(true);
        logThread.start();

        // запуск основного потока
        AppMainThread mainThread;
        mainThread.start();

        auto result = a.exec();

        // остановка основной подсистемы
        if (mainThread.isRunning())
        {
            // отправка сигнала остановки
            mainThread.postTerminateEvent();
            // ожидание завершения потока
            while (!mainThread.isFinished())
            {
                mainThread.postTerminateEvent();
                QThread::msleep(10);
            }
        }

        // остановка подсистемы протоколирования
        if (logThread.isRunning())
        {
            // отправка сигнала остановки
            logThread.postTerminateEvent();
            // ожидание завершения потока
            while (logThread.isRunning())
            {
                logThread.postTerminateEvent();
                QThread::msleep(10);
            }
        }
        return result;
    }
    else
    {
        QString _serviceName = serviceName;
        AppMainThread *mainThread = new AppMainThread();
        Service service(argc, argv, _serviceName, mainThread);
        return service.exec();
    }
}
