#pragma once
#include <QString>
#include<QStringList>
#include<QList>
#include<exception>
#include<limits>
#include<cstring>

#include "../threader_global.h"


namespace Threader {

namespace Utils {

#define BYTES_COUNT 4

typedef union Cardinals {
    quint8 bytes[4];
    unsigned int  values;
} Cardinals;

typedef struct BinarryIpAddress {
    bool wildCards[4];
    Cardinals cardinals;

    bool operator == (const BinarryIpAddress &other) {
        return (std::memcmp(this, &other, sizeof (Cardinals)) == 0);
    }
} BinarryIpAddress;

typedef QList<BinarryIpAddress> TMaskList;

class THREADERSHARED_EXPORT IpMask
{
public:
    /**
     * @brief IpMask
     * @param mask_init_values
     * @param delimiters
     */
    explicit IpMask(const QString &maskValues,
                    const QString &delimiters = " ");

    ~IpMask() = default;
    /**
     * @brief Count
     * @param index
     * @return count of elements in prohibited or allowed list
     */
    int count(int index);
    /**
     * @brief ParseMask
     * @param out_string
     * @return true if parsing was done correct
     *         false if parsing failed
     */
    bool parseMask(QString &outString);
    /**
     * @brief Check
     * @param address
     * @param error_string
     * @return true if address is allowed
     *         false if address is not allowed
     */
    bool checkIpAddress(const QString &address, QString &errorString);

private:
    /**
     * @brief StrToBin
     * @param str_addr
     * @param bin_addr
     * @param use_match
     * @return true if str_addr was successful converted to bin_addr
     *         false in an other variants
     */
    bool strToBin(const QString &strAddr, BinarryIpAddress &binAddr, bool useMatch = false);

private:
    QList<BinarryIpAddress> _prohibitedMask;
    QList<BinarryIpAddress> _allowedMask;
    QString _maskValue;
    QString _delimiters;
    bool _maskParsed;
};

}}
