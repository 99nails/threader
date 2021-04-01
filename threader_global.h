#pragma once

#include <QtCore/qglobal.h>

#if defined(THREADER_LIBRARY)
#  define THREADERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define THREADERSHARED_EXPORT Q_DECL_IMPORT
#endif

