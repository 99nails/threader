#include <QDate>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>

#include <csignal>
#include <execinfo.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <iostream>
#include <QCoreApplication>

#include "../Threads/ThreadLogs.h"

#ifndef STACK_LEVEL
#define STACK_LEVEL 25000
#endif

pid_t gettid()
{
    return pid_t(syscall( __NR_gettid ));
}

void posix_death_signal(int signum);

static QString threadInfo;

void setThreadInfo(const QString & info)
{
    threadInfo = info;
}

QString get_signal_string(int signum);

#define make_signal_name_num(SIG) {#SIG, SIG}

#pragma pack(push, 4)
struct
{
    const char *signame;
    int signum;
}

signal_name_num_list[] =
{
    make_signal_name_num(SIGSEGV),
    make_signal_name_num(SIGABRT),
    make_signal_name_num(SIGILL),
    make_signal_name_num(SIGKILL),
    make_signal_name_num(SIGSTOP),
    make_signal_name_num(SIGBUS),
    make_signal_name_num(SIGSYS),
    make_signal_name_num(SIGFPE)
};
#pragma pack(pop)

void setPosixHandler()
{
//    for (auto & signal_name_num : signal_name_num_list)
//        signal(signal_name_num.signum, posix_death_signal);
    for (auto & i : signal_name_num_list)
        signal(i.signum, posix_death_signal);
}

using namespace Threader::Threads;

void posix_death_signal(int signum)
{
    if(signum == SIGABRT)
    {
        signal(signum, nullptr);
        QCoreApplication::exit(3);
    }
    void *buffer[STACK_LEVEL];
    char **strings;
    int num = backtrace(buffer, STACK_LEVEL);
    strings = backtrace_symbols(buffer, num);
    QString FullName;
    try
    {
        auto appName = QCoreApplication::applicationName();
        auto path = QCoreApplication::applicationDirPath();
        FullName = QCoreApplication::applicationDirPath() + QString("/Log/") +
                           QCoreApplication::applicationName() +
                           QDate::currentDate().toString(QString("-yy.MM.dd")) +
                           QString(".Error");
        path += appName;
    }
    catch(...)
    {
        FullName  = "dump.Error";
    }
    QFile ef(FullName);
    if(ef.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        auto ts = new QTextStream(&ef);
        ts->setCodec(QTextCodec::codecForName("UFT-8"));
        *ts << endl; *ts << endl;
        *ts << QDateTime::currentDateTimeUtc().toString("HH:mm:ss.zzz") << endl;
//        *ts << QString("Version: %1").arg(VERSION_STRING) << endl;
        *ts << get_signal_string(signum) << endl;
        *ts << QString("Thread Id: ") << syscall(SYS_gettid) << endl;
        *ts << threadInfo << endl;
        if(strings != Q_NULLPTR)
        {
            for (int j = 0; j < num; j++)
                *ts << QString(strings[j]) << endl;
            free(strings);
        }
        delete ts;
        ef.close();
    }

    if (ThreadBase::logThread())
    {
        auto logThread = dynamic_cast<ThreadLogs*>(ThreadBase::logThread());
        if(logThread)
            logThread->flush();
    }


    signal(signum, nullptr);
    kill(gettid(), signum);
    QCoreApplication::exit(3);
}

QString get_signal_string(int signum)
{
    QString tmp("Signal: ");
    //for (auto & signal_name_num : signal_name_num_list)
    for (auto & i : signal_name_num_list)
    {
        if (signum == i.signum)
        {
            tmp += i.signame;
            return tmp;
        }
    }
    tmp += QString("%1").arg(signum);
    return tmp;
}
