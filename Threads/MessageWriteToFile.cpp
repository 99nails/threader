#include "MessageWriteToFile.h"

namespace Threader {

namespace Threads {

const QString MessageWriteToFile::MESSAGE_NAME_WRITE_TO_FILE = "Message.Write.To.File";

MessageWriteToFile::MessageWriteToFile(const QString &fileName,
                                       const QString &data,
                                       const WriteMode writeMode)
    : MessageString(data, MESSAGE_NAME_WRITE_TO_FILE)
    , _fileName(fileName)
    , _writeMode(writeMode)
{
}

QString MessageWriteToFile::fileName() const
{
    return _fileName;
}

MessageWriteToFile::WriteMode MessageWriteToFile::writeMode() const
{
    return _writeMode;
}

}}
