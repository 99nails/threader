#pragma once

#include "MessageBase.h"
#include "../threader_global.h"

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT MessageObject : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageObject>;

public:
    explicit MessageObject(const QString &name,
                           QObject *object,
                           bool ownsObject = false);
    ~MessageObject() override;

    QObject *object() const;

    bool ownsObject() const;
    void setOwnsObject(bool ownsObject);

private:
    QObject *_object;
    bool _ownsObject;
};


}}
