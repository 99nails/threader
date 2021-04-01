#pragma once

#include <QtGlobal>

#ifdef Q_OS_LINUX

using Descriptor = int;

#define INVALID_DESCRIPTOR -1

#endif

#ifdef Q_OS_WIN

#include <sdkddkver.h>
#include <winsock2.h>
#include <windows.h>

using Descriptor = HANDLE;

#define INVALID_DESCRIPTOR INVALID_HANDLE_VALUE

#endif


