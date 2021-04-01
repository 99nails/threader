#include "DateUtils.h"

#include <fcntl.h>

#ifdef Q_OS_LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <ctime>

namespace Threader {

namespace Utils {

#ifdef Q_OS_WIN
GetTickCount64Prototype* DateUtils::_getTickCount = nullptr;
#endif

qint64 DateUtils::getTickCount()
{
#ifdef Q_OS_LINUX
    timespec ts{0, 0};
    qint64 tickCountMsecs = -1;
    int result = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (0 == result)
    {
        tickCountMsecs = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }
    return tickCountMsecs;
#endif

#ifdef Q_OS_WIN
    if (!_getTickCount)
    {
        HINSTANCE hModule = ::LoadLibraryA("kernel32.dll");
        if (hModule)
        {
            _getTickCount = (GetTickCount64Prototype*)::GetProcAddress((HMODULE)hModule, "GetTickCount64");
        }
    }

    if (_getTickCount)
        return (*_getTickCount)();

    return 0;
#endif
}

qint64 DateUtils::getNextTickCount(uint milliseconds)
{
    qint64 result = getTickCount();
    if (result >= 0)
        result += milliseconds;
    return result;
}

bool DateUtils::setCurrenrDateTime(const QDateTime &dateTime)
{
#ifdef Q_OS_WIN
    SYSTEMTIME systemTime;
    memset(&systemTime, 0, sizeof(systemTime));

    QDate date = dateTime.date();
    QTime time = dateTime.time();

    systemTime.wYear = date.year();
    systemTime.wMonth = date.month();
    systemTime.wDay = date.day();
    systemTime.wDayOfWeek = date.dayOfWeek() % 7;
    systemTime.wHour = time.hour();
    systemTime.wMinute = time.minute();
    systemTime.wSecond = time.second();
    systemTime.wMilliseconds = time.msec();

    auto result = SetLocalTime(&systemTime);
    return result != 0;
#endif
#ifdef Q_OS_LINUX
    return false;
    Q_UNUSED(dateTime)
#endif
}

}}

