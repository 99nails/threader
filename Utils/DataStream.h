#pragma once

#include "../threader_global.h"

#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QUuid>
#include <QTextCodec>

namespace Threader {

namespace Utils {

/**
 * @brief DataStream - Реализует расширение чтения и записи
 * типизированных данных в QByteArray.
 * Значение QByteArray получается по ссылке и управление его
 * жизненным циклом не производится
 */
class THREADERSHARED_EXPORT DataStream
{
public:
    explicit DataStream(QByteArray *data = nullptr);

    void setData(QByteArray *data);

    int position() const;
    void setPosition(int value);
    char* cursor();

    bool beginOfData() const;
    bool endOfData() const;
    int size() const;

    bool read(void *value, int valueSize);

    template<typename T> inline bool read(T *value)
    {
        return read(value, sizeof(*value));
    }

    template<typename T> inline bool read(T &value)
    {
        return read<T>(&value);
    }


    bool write(const void *value, int valueSize);

    bool writeString(const QString &value);

    template<typename T> inline bool write(const T *value)
    {
        return write(value, sizeof(*value));
    }

    template<typename T> inline bool write(const T &value)
    {
        return write<T>(&value);
    }

    template<typename T> inline bool append(const T *value)
    {
        setPosition(size());
        return write<T>(value);
    }

    template<typename T> inline bool append(const T &value)
    {
        setPosition(size());
        return write<T>(value);
    }

    template<typename T> static inline int valueSize(const T* value)
    {
        return sizeof(*value);

        Q_UNUSED(value)
    }

    template<typename T> static inline int valueSize(const T& value)
    {
        return valueSize(&value);
    }

    /**
     * Возвращает размер сериализованной в поток переменной типа T.
     * При невозможности по типу вывести размер (например для строк или
     * массива байт) дожно возвращать отрицательное значение.
     */
    template<typename T> static inline int valueSize()
    {
        return sizeof(T);
    }

private:
    QByteArray *_data;
    int _position;

    int indexOf(char c);
};

template<> inline bool DataStream::read<QByteArray>(QByteArray* value)
{
    uint32_t valueSize;
    if (!read(valueSize))
        return false;

    if (0 == valueSize)
    {
        value->clear();
        return true;
    }

    int tailSize = size() - position();

    if (tailSize <= 0)
    {
        value->clear();
        return false;
    }

    if (tailSize < (int64_t)valueSize)
        valueSize = tailSize;

    value->resize(valueSize);

    memcpy(value->data(), cursor(), valueSize);
    setPosition(position() + valueSize);
    return true;
}

template<> inline bool DataStream::write<QByteArray>(const QByteArray* value)
{
    if (!value)
        return false;

    uint32_t valueSize = ((uint64_t)value->size()) & 0xFFFFFFFF;
    if (!write(&valueSize, sizeof(valueSize)))
        return false;

    return write(value->constData(), valueSize);
}

template<> inline int DataStream::valueSize<QByteArray>(const QByteArray* value)
{
    if (!value)
        return -1;

    auto valueSize = value->size();
    return valueSize + sizeof(valueSize);
}

template<> inline int DataStream::valueSize<QByteArray>()
{
    return -1;
}

template<> inline bool DataStream::write<QString>(const QString *value)
{
    char EndOfString = 0;

    QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
    QByteArray raw = codec->fromUnicode(*value);

    return write(raw.data(), raw.size()) &&
            write(&EndOfString, sizeof(EndOfString));
}

template<> inline bool DataStream::read<QString>(QString *value)
{
    *value = "";
    QByteArray raw;

    // поиск нулевого символа
    int stringTail = indexOf('\0');

    // если 0 не найден, то установка предела чтения до конца всех данных
    if (-1 == stringTail)
        stringTail = size() - 1;

    // вычисление размера
    int size = (stringTail - position() + 1) * int(sizeof(char));

    // если размер не 0
    if (size + 1 > 0) {
        // выделение буфера и обнуление
        char *buffer = new char[size + 1];
        memset(buffer, 0, size_t(size + 1));
        // чтение в буфер с текущей позиции с заданным размером
        if (size > 0)
            memcpy(buffer, cursor(), size_t(size));
        // передача буфера в строку
        raw.append(buffer);

        delete[] buffer;
    } else {
        setPosition(position() + 1);
    }

    setPosition(position() + size);

    QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
    *value = codec->toUnicode(raw);

    return true;
}

template<> inline int DataStream::valueSize<QString>(const QString* value)
{
    return value->toLocal8Bit().size() + int(sizeof(char));
}

template<> inline int DataStream::valueSize<QString>()
{
    return -1;
}

template<> inline bool DataStream::write<QDateTime>(const QDateTime* value)
{
    QDateTime zeroTime;
    double doubleValue =
            double(value->toMSecsSinceEpoch()) / 86400000. + 25569.;
    return write(doubleValue);
}

template<> inline bool DataStream::read<QDateTime>(QDateTime* value)
{
    double doubleValue = 0;
    bool result = read(doubleValue);

    qint64 milliseconds = qint64(((doubleValue - 25569.) * 86400000.));
    *value = QDateTime::fromMSecsSinceEpoch(milliseconds);
    return result;
}

template<> inline int DataStream::valueSize<QDateTime>()
{
    return sizeof(double);
}

template<> inline int DataStream::valueSize<QDateTime>(const QDateTime* value)
{
    Q_UNUSED(value);
    return valueSize<QDateTime>();
}

template<> inline bool DataStream::write<QUuid>(const QUuid* value)
{
    return write(value->toRfc4122());
}

template<> inline bool DataStream::read<QUuid>(QUuid* value)
{
    QByteArray data;
    data.resize(16);
    bool result = read(data);
    if (result)
        *value = QUuid::fromRfc4122(data);
    return result;
}

template<> inline int DataStream::valueSize<QUuid>()
{
    return 16;
}

template<> inline int DataStream::valueSize<QUuid>(const QUuid* value)
{
    Q_UNUSED(value);
    return valueSize<QUuid>();
}


}}
