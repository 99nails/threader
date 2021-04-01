#include "MessageLog.h"

namespace Threader {

namespace Threads {

#define MESSAGE_NAME_LOG_DEFINITION "Message.Log"

const QString MessageLog::MESSAGE_NAME = MESSAGE_NAME_LOG_DEFINITION;

MessageLog::MessageLog(const QString& text, int number, int level, MessageType messageType)
    : MessageString(text, MessageLog::MESSAGE_NAME)
    , _number(number)
    , _level(level)
    , _messageType(messageType)
{
}

MessageLog::MessageLog(const MessageLogTemplate *messageTemplate, ...)
    : MessageString(QString(), MessageLog::MESSAGE_NAME)
    , _number(messageTemplate->number)
    , _level(messageTemplate->level)
    , _messageType(messageTemplate->messageType)
{
    char resultPtr[1024];
    memset(resultPtr, 0, sizeof(resultPtr));

    va_list args;
    va_start(args, messageTemplate);

    auto resultCount = vsnprintf(resultPtr, sizeof(resultPtr),
                                 messageTemplate->text.toUtf8().constData(), args);
    va_end(args);

    if (resultCount > 0)
        setText(QString(resultPtr));
    else
        setText(messageTemplate->text);
}

MessageLog::MessageLog(const MessageLogTemplate messageTemplate)
    : MessageString(QString(), MessageLog::MESSAGE_NAME)
    , _number(messageTemplate.number)
    , _level(messageTemplate.level)
    , _messageType(messageTemplate.messageType)
{
    setText(messageTemplate.text);
}

MessageType MessageLog::messageType()
{
    return _messageType;
}

QString MessageLog::messageTypeString()
{
    return MessagesTypesNames[_messageType];
}

int MessageLog::number() const
{
    return _number;
}

int MessageLog::level() const
{
    return _level;
}

QString MessageLog::toString(int messageFlags)
{
    QString format;
    QString result = "";
    if (MessageToStringFlags::Date & messageFlags) {
        format = "dd.MM.yyyy";
        result += created().toString(format);
    }

    if (MessageToStringFlags::Time & messageFlags) {
        format = "hh:mm:ss.zzz";
        if (!result.isEmpty())
            result += " ";
        result += created().toString(format);
    }

    if (!result.isEmpty())
        result = '[' + result + ']';

    if (MessageToStringFlags::Type & messageFlags) {
        if (!result.isEmpty())
            result += " ";
        result += messageTypeString() + ':';
    }

    if (MessageToStringFlags::Number & messageFlags) {
        if (!result.isEmpty())
            result += " ";

        result += QString(" %1 ").arg(number(), 5, 10, QLatin1Char('0'));
    }

    if (MessageToStringFlags::Text & messageFlags) {
        if (!result.isEmpty())
            result += " ";
        result += text();
    }

    return result;
}

}}
