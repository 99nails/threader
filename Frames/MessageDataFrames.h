#pragma once

#include "../threader_global.h"

#include "../Threads/MessageBase.h"
#include "DataFrameRawData.h"
#include "DataFrames.h"

#include <QList>

namespace Threader {

namespace Frames {


using namespace Threader::Threads;


/**
 * @brief The MessageDataFrame class - Сообщение с передачей указателя на фрейм
 */
class THREADERSHARED_EXPORT MessageDataFrame : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageDataFrame>;
    static const QString MESSAGE_NAME_DATA_FRAME;

public:
    explicit MessageDataFrame(const DataFrame::Ptr &frame);

    DataFrame::Ptr frame() const;

private:
    DataFrame::Ptr _frame;
};

/**
 * @brief The MessageDataFrames class - Сообщение со списком фреймов
 */
class THREADERSHARED_EXPORT MessageDataFrames : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageDataFrames>;

public:
    static const QString MESSAGE_NAME_DATAFRAMES;

    explicit MessageDataFrames(const QString &alias, const DataFrameRawDataList &list);

    explicit MessageDataFrames(const QString &alias, const DataFrameRawData::Ptr &frame);

    virtual ~MessageDataFrames() = default;

    QString alias() const;

    DataFrameRawDataList list();

private:
    QString _alias;
    QList<DataFrameRawData::Ptr> _list;
};

}}

