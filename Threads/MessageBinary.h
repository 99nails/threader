#pragma once

#include "MessageBase.h"
#include "../threader_global.h"

#include <QByteArray>

namespace Threader {

namespace Threads {


class THREADERSHARED_EXPORT MessageBinary : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageBinary>;

public:

    static const QString BINARY_MESSAGE_NAME;

    explicit MessageBinary(const char *data, int size);
    explicit MessageBinary(const QString &name, const char *data, int size);
    explicit MessageBinary(const QString &name, const QByteArray *data);
    explicit MessageBinary(const QString &name, const QByteArray &data);

    const QByteArray *data() const;

protected:
    void setData(QByteArray *data);

private:
    QByteArray _data;

};

}}
