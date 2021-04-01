#pragma once

#include "MessageBase.h"
#include "../threader_global.h"

namespace Threader {

namespace Threads {

/**
 * @brief MessageString - Класс сообщения, содержащего строковые данные
 */
class THREADERSHARED_EXPORT MessageString : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageString>;

public:

    /**
     * @brief MESSAGE_NAME_STRING - Имя строкового сообщения по умолчанию
     */
    static const QString MESSAGE_NAME_STRING;

    /**
     * @brief MessageString - Конструтор сообщения
     * @param text - Текст сообщения
     * @param name - Имя сообщения
     */
    explicit MessageString(const QString &text = QString(),
                           const QString &name = MESSAGE_NAME_STRING);

    /**
     * @brief text - Получение текста сообщения
     * @return - Текст сообщения
     */
    QString text();
protected:
    /**
     * @brief setText - Установка текста сообщения
     * @param text - Текст сообщения
     */
    void setText(const QString &text);

private:
    /**
     * @brief _text - Текст сообщения
     */
    QString _text;
};

}}
