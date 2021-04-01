#pragma once

#include "ThreadsCommon.h"
#include "PollingsLinux.h"
#include "PollingsWindows.h"

#include "../threader_global.h"

#include <QObject>
#include <QtGlobal>

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT HandlerBase : public PollerBase
{
    Q_OBJECT
public:
    /**
     * @brief ConnectionState состояние подключения
     */
    enum class ConnectionState
    {
        Disconnected,
        Connecting,
        Connected
    };

    /**
     * @brief HandlerBase Конструктор с именем устройства
     * @param deviceName Имя устройства в системе
     */
    explicit HandlerBase(const QString &deviceName = "",
                         const Descriptor descriptor = INVALID_DESCRIPTOR);

#ifdef Q_OS_WIN
    Descriptor descriptor() const;
    virtual void setDescriptor(const Descriptor &value);
#endif

#ifdef Q_OS_LINUX
    void setDescriptor(const Descriptor &descriptor) override;
#endif

    ConnectionState connectionState();
    virtual bool isConnected();

    QString deviceName() const;
    void setDeviceName(const QString &deviceName);

    bool needsToWrite() const;
    void setNeedsToWrite(const bool needsToWrite);

    virtual bool open() = 0;
    virtual void close() = 0;

    /**
     * @brief inputBytesAvailable - Возврат количества байт во входном буфере
     * @return - Количество байт во входном буфере
     */
    virtual int inputBytesAvailable() = 0;

    /**
     * @brief outputBytesAvailable - Возврат количества свободных байт в выходном буфере
     * @return - Количество байт в выходном буфере
     */
    virtual int outputBytesAvailable() = 0;

    /**
     * @brief read - Чтение данных из устройства
     * @param data - указатель на буфер для чтения данных
     * @param count - размер буфера
     * @return - количество прочитанных данных
     */
#ifdef Q_OS_LINUX
    virtual int read(char *data, int count);
    virtual int write(const char *data, int count);
#endif
#ifdef Q_OS_WIN
    virtual int read(char *data, int count) = 0;
    virtual int write(const char *data, int count) = 0;
#endif    

    QString readString();
    int writeString(const QString &data);

    QByteArray readBytes();
    int writeBytes(const QByteArray &data);

protected:
    void setConnectionState(ConnectionState value);

    void doReadyRead();
    void doReadyWrite();
    void doError(int errorCode);

private:
    QString _deviceName;
    ConnectionState _connectionState;
    bool _needsToWrite;

#ifdef Q_OS_WIN
    Descriptor _descriptor;
#endif

signals:
    void signalOnConnectionStateChanged(HandlerBase *sender,
                                      ConnectionState oldState,
                                      ConnectionState newState);

    void signalOnReadyRead(HandlerBase *sender);

    void signalOnReadyWrite(HandlerBase *sender);

    void signalOnError(HandlerBase *sender, int errorCode);
};

}}
