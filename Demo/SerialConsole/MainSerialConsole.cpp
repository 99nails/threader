#include "DaemonApplication.h"
#include "ThreadMainSerialConsole.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextCodec>


int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    return AppMain<ThreadMainSerialConsole>(argc, argv, "Serial_Console");
}
