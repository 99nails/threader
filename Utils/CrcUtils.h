#pragma once

#include "../threader_global.h"

#include <QCryptographicHash>
#include <QtGlobal>

#ifdef Q_OS_WIN
#include <cstdint>
using u_int16_t = uint16_t;
#endif

#ifdef Q_OS_LINUX
#include <sys/types.h>
#endif


namespace Threader {

namespace Utils {

class THREADERSHARED_EXPORT CrcUtils
{
public:
    static unsigned char Crc8(const unsigned char *pcBlock,
                              unsigned char len);

    static uint16_t Crc16(u_int16_t start,
                          const unsigned char *data,
                          const int count);

    static uint16_t Crc16(uint8_t *data,
                          uint16_t length);

    static uint32_t Crc32(uint8_t *data, uint32_t length);

    static QByteArray fileChecksum(const QString &fileName,
                                   QCryptographicHash::Algorithm hashAlgorithm);

};

}}
