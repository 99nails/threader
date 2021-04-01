#pragma once

#include "../threader_global.h"

#include "MessageBase.h"
#include "ThreadBase.h"

namespace Threader{

namespace Threads {


class THREADERSHARED_EXPORT MessageThread : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageThread>;

public:
    static const QString MESSAGE_NAME;

    explicit MessageThread(ThreadBase *thread);

    ThreadBase *thread() const;

private:
    ThreadBase *_thread;

};

}}
