#include "MessageDataFrames.h"

namespace Threader {

namespace Frames {


const QString MessageDataFrame::MESSAGE_NAME_DATA_FRAME  = "Message.DataFrame";

MessageDataFrame::MessageDataFrame(const DataFrame::Ptr &frame)
                    : MessageBase(MESSAGE_NAME_DATA_FRAME)
                    , _frame(frame)
{
}

DataFrame::Ptr MessageDataFrame::frame() const
{
    return _frame;
}


const QString MessageDataFrames::MESSAGE_NAME_DATAFRAMES = "Message.DataFrames";

MessageDataFrames::MessageDataFrames(const QString &alias,
                                     const DataFrameRawDataList &list)
                    : MessageBase(MESSAGE_NAME_DATAFRAMES)
                    , _alias(alias)
                    , _list(list)
{
}

MessageDataFrames::MessageDataFrames(const QString &alias,
                                     const DataFrameRawData::Ptr &frame)
                    : MessageBase(MESSAGE_NAME_DATAFRAMES)
                    , _alias(alias)
{
    _list.append(frame);
}

QString MessageDataFrames::alias() const
{
    return _alias;
}

DataFrameRawDataList MessageDataFrames::list()
{
    return _list;
}

}}

