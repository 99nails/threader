#include "DaemonApplication.h"
#include "ThreadMainUdp.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    for (int i = 0; i< argc; i++)
    {
        QString parameter = QString(argv[i]).toLower();
        if ("--listener" == parameter)
        {
            ThreadMainUdp::setSocketType(HandlerUdpSocket::SocketType::Listener);
        }
        else if ("--sender" == parameter)
        {
            ThreadMainUdp::setSocketType(HandlerUdpSocket::SocketType::Sender);
        }
    }

    return AppMain<ThreadMainUdp>(argc, argv, "Serial_Console");
}
