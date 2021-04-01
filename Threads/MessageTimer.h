#pragma once

#include "MessageBase.h"
#include "../threader_global.h"

namespace Threader {

namespace Threads {

/**
 * @brief MessageTimer - Класс сообщения таймера
 */
class THREADERSHARED_EXPORT MessageTimer : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageTimer>;

public:
    /**
     * @brief MessageTimer - Конструктор
     * @param name - Имя сообщения таймера
     * @param shotCount - Счетчик сообщения
     */
    explicit MessageTimer(const QString &name,
                          const qint64 &shotCount);

    /**
     * @brief shotCount - Получение счетчика сообщения
     * @return  - Счетчик сообщения
     */
    qint64 shotCount() const;

private:
    /**
     * @brief _shotCount - Счетчик сообщения
     */
    qint64 _shotCount;
};

}}
