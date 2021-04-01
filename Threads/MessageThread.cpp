#include "MessageThread.h"

namespace Threader{

namespace Threads {


const QString MessageThread::MESSAGE_NAME = "Message.Thread";


MessageThread::MessageThread(ThreadBase *thread)
    : MessageBase(MESSAGE_NAME)
    , _thread(thread)
{
}

ThreadBase *MessageThread::thread() const
{
    return _thread;
}


}}
