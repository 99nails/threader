#include "MessageQueue.h"

namespace Threader {

namespace Frames {


const QString MessageQueue::MESSAGE_NAME = "Message.Queue";


MessageQueue::MessageQueue(const QueueDataFrames *queue, const QString &alias)
    : MessageBase(MESSAGE_NAME)
    , _queue(queue)
    , _alias(alias)
{
}

const QueueDataFrames *MessageQueue::queue() const
{
    return _queue;
}

QString MessageQueue::alias() const
{
    return _alias;
}



}}
