#include "MessageBinary.h"

namespace Threader {

namespace Threads {

const QString MessageBinary::BINARY_MESSAGE_NAME = "Message.Binary";

MessageBinary::MessageBinary(const char *data, int size)
    : MessageBase (QString(BINARY_MESSAGE_NAME))
{
    _data.append(data, size);
}

MessageBinary::MessageBinary(const QString &name,
                             const char *data, int size)
    : MessageBase(name)
{
    _data.append(data, size);
}

MessageBinary::MessageBinary(const QString &name,
                             const QByteArray *data)
    : MessageBase(name)
    , _data(*data)
{}

MessageBinary::MessageBinary(const QString& name,
                             const QByteArray &data)
    : MessageBase(name)
    , _data(data)
{}


const QByteArray *MessageBinary::data() const
{
    return &_data;
}

void MessageBinary::setData(QByteArray *data)
{
    _data.append(*data);
}

}}
