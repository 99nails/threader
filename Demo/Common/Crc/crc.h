#pragma once

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <cstdint>
using u_int16_t = uint16_t;
#endif

#ifdef Q_OS_LINUX
#include <sys/types.h>
#endif


unsigned char crc8(const unsigned char *pcBlock, unsigned char len);
uint16_t crc16(u_int16_t start, const unsigned char *data, int count);



