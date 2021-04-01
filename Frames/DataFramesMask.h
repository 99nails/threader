#pragma once

#include "../threader_global.h"

#include "DataFramesCommon.h"

#include <QHash>
#include <QList>
#include <QRegExp>

namespace Threader {

namespace Frames {


typedef struct _dataFramesDefStruct {
    explicit _dataFramesDefStruct(bool _isAllowed = false,
                                  int _priority = 1,
                                  bool _isBroadcast = false)
        : isAllowed(_isAllowed)
        , priority(_priority)
        , isBroadcast(_isBroadcast)
    {
    }

    bool isAllowed;
    int priority;
    bool isBroadcast;
} DataFrameDefinitionStruct;


using  DataFramesNamesHash = QHash<QString, bool>;

using DataFramesNamesMask = QList<QRegExp>;

/**
 * @DataFramesMask позволяет определять разрешения для использования фреймов
 * на основе имен фреймов
 * Класс автоматичеки обновляет список быстрого доступа к маскам
 * но основе определений фреймов
 */
class THREADERSHARED_EXPORT DataFramesMask
{
public:
    explicit DataFramesMask(const QString &mask = "",
                            DataFramesDefinitions *definitions = Q_NULLPTR);

    bool allow(QString &name);

private:
    DataFramesNamesHash _knownNames;
    DataFramesNamesMask _masksAllowed;
    DataFramesNamesMask _masksProhibited;

    bool allowFrameByMasks(QString &name);
};

using DataFramesMasksHash = QHash<QString, DataFramesMask>;


}}
