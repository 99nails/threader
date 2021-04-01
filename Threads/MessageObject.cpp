#include "MessageObject.h"

namespace Threader {

namespace Threads {


MessageObject::MessageObject(const QString &name,
                             QObject *object,
                             bool ownsObject)
    : MessageBase(name)
    , _object(object)
    , _ownsObject(ownsObject)
{
}

MessageObject::~MessageObject()
{
    if (_ownsObject && _object)
        delete _object;
}

QObject *MessageObject::object() const
{
    return _object;
}

bool MessageObject::ownsObject() const
{
    return _ownsObject;
}

void MessageObject::setOwnsObject(bool ownsObject)
{
    _ownsObject = ownsObject;
}

}}
