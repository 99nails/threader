#pragma once

#include "../threader_global.h"

#include "../Threads/ThreadsCommon.h"

#include <QString>
#include <QtGlobal>

namespace Threader {

namespace Utils {

class THREADERSHARED_EXPORT SerialUtils
{
public:
    static Descriptor openSerial(const QString &deviceName,
                                 const uint &baudRate,
                                 const bool &enableParity,
                                 const bool &twoStopBits,
                                 const uint &dataSize);

    static void closeSerial(Descriptor descriptor);

    static int inputBytesAvailable(Descriptor descriptor);

    static int outputBytesAvailable(Descriptor descriptor);

#ifdef Q_OS_WIN
    static bool setWaitMask(Descriptor descriptor, bool needsToWrite);
#endif
};

}}
