#pragma once

#include "../threader_global.h"

#include "QueueDataFrames.h"

#include "../Threads/MessageBase.h"


namespace Threader {

namespace Frames {


using namespace Threader::Threads;


class THREADERSHARED_EXPORT MessageQueue : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageQueue>;

public:

    static const QString MESSAGE_NAME;

    explicit MessageQueue(const QueueDataFrames *queue, const QString &alias = "");

    const QueueDataFrames *queue() const;

    QString alias() const;

private:
    const QueueDataFrames *_queue;
    const QString _alias;
};


}}
