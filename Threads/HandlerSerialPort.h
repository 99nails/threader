#pragma once

#include "../threader_global.h"

#include "HandlerBase.h"

#include <QObject>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT HandlerSerialPort : public HandlerBase
{
    Q_OBJECT
public:
    explicit HandlerSerialPort(const QString &deviceName);

    bool open() override;

    void close() override;

    int inputBytesAvailable() override;

    int outputBytesAvailable() override;

#ifdef Q_OS_WIN
    DescriptorsVector *events() override;

    bool process(Descriptor eventToProcess) override;

    int read(char *data, int count) override;

    int write(const char *data, int count) override;
#endif

#ifdef Q_OS_LINUX
    void assign(pollfd &event) override;

    bool process(const pollfd &event) override;
#endif

    uint baudRate() const;
    bool enableParity() const;
    bool twoStopBits() const;
    uint dataSize() const;

    void setBaudRate(const uint &baudRate);
    void setEnableParity(bool enableParity);
    void setTwoStopBits(bool twoStopBits);
    void setDataSize(const uint &dataSize);

#ifdef Q_OS_WIN
    bool useOverlappedIO() const;
    void setUseOverlappedIO(bool useOverlappedIO);
#endif

private:
    uint _baudRate;
    bool _enableParity;
    bool _twoStopBits;
    uint _dataSize;

#ifdef Q_OS_WIN
    bool _useOverlappedIO;
    OVERLAPPED _overlapped;
    DWORD _mask;

    void initializeEvent();
    void registerEvent();
    void unregisterEvent();
    bool processEvent();
#endif
};


}}
