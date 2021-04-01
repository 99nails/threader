#include "MessageString.h"

namespace Threader {

namespace Threads {

const QString MessageString::MESSAGE_NAME_STRING = "Message.String";

MessageString::MessageString(const QString& text, const QString &name)
    : MessageBase(name)
    , _text(text)
{
}

QString MessageString::text()
{
    return _text;
}

void MessageString::setText(const QString &text)
{
    _text = text;
}

}}
