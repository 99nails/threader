#include "ThreadSerialPort.h"

#include "../Common/Logs/LogMessagesTemplatesDemo.h"

#include "HandlerSerialPort.h"

#include <QDebug>

ThreadSerialPort::ThreadSerialPort(IMessageSubscriber *parent,
                                   const QString &deviceName)
    : ThreadHandler(parent,
                    new HandlerSerialPort(deviceName),
                    30,
                    nullptr,
                    "Thread.Serial")
{
}

void ThreadSerialPort::onThreadStarted()
{
    writeLog(nullptr, Message2000,
             STRLOG(handler()->deviceName()));
}

void ThreadSerialPort::onThreadFinishing()
{
    writeLog(nullptr, Message2001,
             STRLOG(handler()->deviceName()));
}

void ThreadSerialPort::onThreadFinished()
{
    writeLog(nullptr, Message2002,
             STRLOG(handler()->deviceName()));
}

void ThreadSerialPort::onIdle()
{
}

void ThreadSerialPort::onConnecting()
{
    writeLog(nullptr, Message2003,
             STRLOG(handler()->deviceName()));
}

void ThreadSerialPort::onConnected()
{
    writeLog(nullptr, Message2004,
             STRLOG(handler()->deviceName()));

    handler()->writeString("AT\r\n");
//    outputStream()->write(QString("*ver\r\n"));
//    outputStream()->write(QString("*setdb105v1\r\n"));
//    outputStream()->write(QString("*Zoff\r\n"));
//    outputStream()->write(QString("*w2047\r\n"));
//    outputStream()->write(QString("*debug\r\n"));
}

void ThreadSerialPort::onDisconnected()
{
    writeLog(nullptr, Message2006,
             STRLOG(handler()->deviceName()));
}

void ThreadSerialPort::onError(const int errorCode)
{
    writeLog(nullptr, Message2005,
             STRLOG(handler()->deviceName()),
             errorCode,
             STRLOG(errorString(errorCode)));
}

bool ThreadSerialPort::onPacketReceived(const PacketBase::Ptr &)
{
    return true;
}

bool ThreadSerialPort::onPacketSent(const PacketBase::Ptr &)
{
    return true;
}

void ThreadSerialPort::onReadData(const char *data,
                                  const int &count)
{
    qDebug() << "Получено:" << QString(data);
    Q_UNUSED(count);
}

void ThreadSerialPort::onWriteData(const char *data, const int &)
{
    qDebug() << "Передано:" << QString(data);
}
