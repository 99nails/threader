#include "DataStream.h"

#include <QByteArray>

namespace Threader {

namespace Utils {


DataStream::DataStream(QByteArray *data)
    : _data(data)
    , _position(0)
{
}

void DataStream::setData(QByteArray *data)
{
    _data = data;
    _position = 0;
}

int DataStream::position() const
{
    return _position;
}

void DataStream::setPosition(int value)
{
    int dataCount = 0;
    _position = -1;

    if (nullptr != _data) {
        _position = value;
        dataCount = _data->count();
        if (_position > dataCount)
        {
            _position = dataCount;
        }
        else if (_position < 0)
        {
            _position = 0;
        }
    }
}

char *DataStream::cursor()
{
    if (nullptr == _data)
        return nullptr;
    return (_data->data()) + position();
}

bool DataStream::beginOfData() const
{
    if (nullptr == _data)
        return true;
    return _position <= 0;
}

bool DataStream::endOfData() const
{
    if (nullptr == _data)
        return true;
    int dataSize = size();
    bool result = (_position >= dataSize);
    return result;
}

int DataStream::size() const
{
    if (nullptr == _data)
        return -1;
    return _data->count();
}

int DataStream::indexOf(char c)
{
    if (nullptr == _data)
        return -1;
    int pos = position();
    int result = _data->indexOf(c, pos);
    return result;
}

bool DataStream::read(void *value, int valueSize)
{
    if (position() + valueSize > size())
        setPosition(size());
    if (endOfData()) {
        return false;
    }
    memcpy(value, _data->data() + _position, size_t(valueSize));
    setPosition(position() + valueSize);
    return true;
}

bool DataStream::write(const void *value, int valueSize)
{
    if (nullptr == _data)
        return false;
    if (position() + valueSize > size())
        _data->resize(position() + valueSize);
    memcpy(cursor(), value, size_t(valueSize));
    setPosition(position() + valueSize);
    return true;
}

bool DataStream::writeString(const QString &value)
{
    QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
    if (!codec)
        return false;
    QByteArray raw = codec->fromUnicode(value);
    return write(raw.data(), raw.size());
}

}}
