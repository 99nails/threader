#pragma once

#include <QDateTime>
#ifdef Q_OS_LINUX
#include <linux/rtc.h>
#endif

#ifdef Q_OS_WIN

#include <windows.h>
#include <time.h>

#endif

#include "../threader_global.h"


namespace Threader {

namespace Utils {

#ifdef Q_OS_WIN
typedef ULONGLONG (WINAPI GetTickCount64Prototype)(VOID);
#endif

class THREADERSHARED_EXPORT DateUtils {
public:
    DateUtils() = default;

    static qint64 getTickCount();
    static qint64 getNextTickCount(uint milliseconds);
    static bool setCurrenrDateTime(const QDateTime &dateTime);

private:
#ifdef Q_OS_WIN
    static GetTickCount64Prototype* _getTickCount;
#endif
};

}}
