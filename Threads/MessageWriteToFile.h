#pragma once

#include "MessageString.h"
#include "../threader_global.h"

namespace Threader {

namespace Threads {

class THREADERSHARED_EXPORT MessageWriteToFile : public MessageString
{
public:
    enum class WriteMode {Truncate, Append};

    using Ptr = std::shared_ptr<MessageWriteToFile>;

public:
    static const QString MESSAGE_NAME_WRITE_TO_FILE;

    explicit MessageWriteToFile(const QString &fileName,
                                const QString &data,
                                const WriteMode writeMode = WriteMode::Append);
    QString fileName() const;

    WriteMode writeMode() const;

private:
    QString _fileName;
    WriteMode _writeMode;
};

}}
