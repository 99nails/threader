#include "IpMask.h"

#include "../threader_global.h"


namespace Threader {

namespace Utils {

IpMask::IpMask(const QString &maskValues,
               const QString &delimiters)
    : _maskValue(maskValues)
    , _delimiters(delimiters)
    , _maskParsed(false)
{
    QString outString;
    parseMask(outString);
}

int IpMask::count(int index) {
    int result =  -1;
    switch(index) {
    case 0:
            result = this->_prohibitedMask.length();
        break;
    case 1:
            result = this->_allowedMask.length();
        break;
    }
    return result;
}

bool IpMask::parseMask(QString &outString)
{
    bool result = false;
    BinarryIpAddress binAddr;
    QStringList list = this->_maskValue.split(this->_delimiters, QString::SkipEmptyParts);
    foreach(QString line, list) {
        if(line.split(".",QString::SkipEmptyParts).length() > 4) {
            outString = "Invalid Ip address " + line;
            return result;
        }
        if (line.indexOf("!") == 0) {
            line.remove(0,1);
            if (this->strToBin(line, binAddr, true)) {
                this->_prohibitedMask.append(binAddr);
            }
            else {
                outString = "Недопустимый формат маски запрещённых адресов:   "  + line;
                return result;
            }
        }
        else {
            if (this->strToBin(line, binAddr, true)) {
                this->_allowedMask.append(binAddr);
            }
            else {
                outString = "Недопустимый формат маски разрешённых адресов:   "  + line;
                return result;
            }
        }
    }

    binAddr.wildCards[0] = false;
    binAddr.wildCards[1] = true;
    binAddr.wildCards[2] = true;
    binAddr.wildCards[3] = true;
    binAddr.cardinals.bytes[0] = 127;
    binAddr.cardinals.bytes[1] = 255;
    binAddr.cardinals.bytes[2] = 255;
    binAddr.cardinals.bytes[3] = 255;

    if (this->count(1) == 0)
        this->_allowedMask.append(binAddr);

    binAddr.wildCards[0] = true;
    binAddr.wildCards[1] = true;
    binAddr.wildCards[2] = true;
    binAddr.wildCards[3] = true;
    binAddr.cardinals.values = 0xFFFFFFFF;

    if (this->_prohibitedMask.contains(binAddr))
        outString = "Запрещены все IP адреса";
    else {
        result = true;
        this->_maskParsed = true;
    }

    return result;
}

bool IpMask::checkIpAddress(const QString &address, QString &errorString)
{
    bool result = false;
    bool notMatch;
    BinarryIpAddress binAddr;

    if (!this->_maskParsed) {
        errorString = "IP table was not created";
        return result;
    }
    if (!this->strToBin(address, binAddr)) {
        errorString = "Invalid IP address: " + address;
        return result;
    }

    foreach(BinarryIpAddress listed_address, this->_prohibitedMask) {
        notMatch = false;
        if ((binAddr.cardinals.values & listed_address.cardinals.values) == binAddr.cardinals.values) {
            for(int i = 0; i < BYTES_COUNT; ++i) {
                quint8 tst = binAddr.cardinals.bytes[i] | listed_address.cardinals.bytes[i];
                if ((tst != binAddr.cardinals.bytes[i]) && (!listed_address.wildCards[i])) {
                    notMatch = true;
                    break;
                }
            }
            if (!notMatch) {
                return result;
            }
        }
    }

    foreach(BinarryIpAddress listed_address, this->_allowedMask) {
        notMatch = false;
        if ((binAddr.cardinals.values & listed_address.cardinals.values ) == binAddr.cardinals.values) {
            for(int i = 0; i < BYTES_COUNT; ++i) {
                quint8 tst = binAddr.cardinals.bytes[i] | listed_address.cardinals.bytes[i];
                if ((tst != binAddr.cardinals.bytes[i]) && (!listed_address.wildCards[i])) {
                    notMatch = true;
                    break;
                }
            }
            if (!notMatch) {
                result = true;
                break;
            }
        }
    }
    return result;
}

bool IpMask::strToBin(const QString &strAddr, BinarryIpAddress &binAddr, bool useMatch)
{
    bool result = false;
    QString stringIp = strAddr;
    QString sbyte;
    bool ok;
    int position;
    int start = 0;
    for (int i = 0; i < BYTES_COUNT; ++i) {
        if (start > stringIp.length())
            return false;
        position = stringIp.indexOf('.', start);
        if (position == -1) {
            if (useMatch && stringIp[start] == '*') {
                for (int j = i; j < 4; ++j) {
                    binAddr.wildCards[j] = true;
                    binAddr.cardinals.bytes[j] = 0xff;
                }
                result = true;
                return result;
            }
            else if (i == 3) {
                sbyte = stringIp.mid(start, stringIp.length() - start);
                 int gbyte = sbyte.toInt(&ok, 10);
                 if (ok &&(gbyte >= std::numeric_limits<quint8>::min()) && (gbyte <= std::numeric_limits<quint8>::max())) {
                     binAddr.cardinals.bytes[i] = gbyte & 0xff;
                     binAddr.wildCards[i] = false;
                 }
                 else
                     return result;
            }
            else {
                return false;
            }
        }
        else {
            if (useMatch && stringIp[start] == '*') {
                binAddr.wildCards[i] = true;
                binAddr.cardinals.bytes[i] = 0xff;
            }
            else {
                sbyte = stringIp.mid(start, position - start);
                 int gbyte = sbyte.toInt(&ok, 10);
                 if (ok &&(gbyte >= std::numeric_limits<quint8>::min()) && (gbyte <= std::numeric_limits<quint8>::max())) {
                     binAddr.cardinals.bytes[i] = gbyte & 0xff;
                     binAddr.wildCards[i] = false;
                 }
                 else
                    return false;
            }
        }
        start = position + 1;
    }
    result = true;
    return result;
}

}}
