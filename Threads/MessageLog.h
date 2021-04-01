#pragma once

#include "MessageString.h"
#include "../threader_global.h"

#include <QtCore/qglobal.h>

namespace Threader {

namespace Threads {

/**
 * @brief MessageType - Тип сообщений (Сообщение, Предупреждение, Ошибка, Отладка)
 */
enum MessageType
{
    Message,
    Warning,
    Error,
    Debug
};

/**
 * @brief MessageToStringFlags - флаги отображения данных при форматировании сообщения
 * протоколирования в строку
 */
enum MessageToStringFlags
{
    Date   = 1,
    Time   = 2,
    Type   = 4,
    Number = 8,
    Text   = 16
};

/**
 * @brief MessagesTypesNames
 */
const QString MessagesTypesNames[MessageType::Debug + 1] = {"MSG", "WRN", "ERR", "DBG"};

#pragma pack(push, 4)
struct MessageLogTemplate {
    int         number;
    int         level;
    MessageType messageType;
    QString     text;
};
#pragma pack(pop)

/**
 * Макрос для задания шаблона сообщения.
 * @param NUMBER - номер сообщения
 * @param LEVEL  - уровень сообщения
 * @param TYPE   - тип сообщения
 * @param TEXT   - текст сообщения
 */
#define MESSAGE_TEMPLATE(NUMBER, LEVEL, TYPE, TEXT)                                                \
    const MessageLogTemplate Message##NUMBER = {NUMBER, LEVEL, TYPE, TEXT}

/**
 * Макрос для протоколирования идентификаторов
 * @param GUID - идентификатор
 */
#define GUIDLOG(GUID) GUID.toString().toUtf8().constData()
/**
 * Макрос для протоколирования строкового значения
 * @param TEXT - строковое (QString) значение
 */
#define STRLOG(TEXT) TEXT.toUtf8().constData()
/**
 * Макрос для протоколирования двух строковых значений
 * @param TEXT1 - первое строковое (QString) значение
 * @param TEXT2 - второе строковое (QString) значение
 */
#define STRLOG2(TEXT1, TEXT2) TEXT1.toUtf8().constData(), TEXT2.toUtf8().constData()
/**
 * Макрос для протоколирования трех строковых значений
 * @param TEXT1 - первое строковое (QString) значение
 * @param TEXT2 - второе строковое (QString) значение
 * @param TEXT3 - третье строковое (QString) значение
 */
#define STRLOG3(TEXT1, TEXT2, TEXT3)                                                               \
    TEXT1.toUtf8().constData(), TEXT2.toUtf8().constData(),                              \
                        TEXT3.toUtf8().constData()
/**
 * Макрос для протоколирования даты и времени по заданному формату
 * @param DATETIME - значение даты и времени
 * @param FORMAT - форрматирование
 */
#define DTFLOG(DATETIME, FORMAT) DATETIME.toString(FORMAT).toUtf8().constData()
/**
 * Макрос для протоколирования даты и времени
 * @param DATETIME - значение даты и времени
 */
#define DTLOG(DATETIME) DTFLOG(DATETIME, "dd.MM.yyyy HH:mm:ss.zzz")

class THREADERSHARED_EXPORT MessageLog : public MessageString
{
public:
    using Ptr = std::shared_ptr<MessageLog>;

public:
    static const QString MESSAGE_NAME;

    explicit MessageLog(const QString &text,
                        int number,
                        int level,
                        MessageType messageType = Message);

    explicit MessageLog(const MessageLogTemplate *messageTemplate, ...);

    explicit MessageLog(const MessageLogTemplate messageTemplate);

    MessageType messageType();
    QString     messageTypeString();
    int         number() const;
    int         level() const;

    QString toString(int messageFlags = MessageToStringFlags::Time | MessageToStringFlags::Type |
            MessageToStringFlags::Number | MessageToStringFlags::Text);

protected:
    int         _number;
    int         _level;
    MessageType _messageType;
};

using VectorMessagesLog = QVector<MessageLog::Ptr>;

}}
