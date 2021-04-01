#include "DataFramesMask.h"


namespace Threader {

namespace Frames {


DataFramesMask::DataFramesMask(const QString &mask, DataFramesDefinitions *definitions)
{
    // формирование списка определений из строки настроек
    QStringList masksList = mask.split(' ', QString::SkipEmptyParts, Qt::CaseInsensitive);
//    qSort(masksList.begin(), masksList.end());
    std::sort(masksList.begin(), masksList.end());

    // формирование рарешенных и запрещенных масок в виде регулярных выражений
    for (int i = 0; i < masksList.count(); i++)
    {
        // имена фреймов принимаются в верхнем регистре
        QString item = masksList[i].toUpper();
        if (item.isEmpty())
            continue;

        // символ '!' определяет исключающую маску
        if (item.at(0) == '!')
        {
            // удаление '!'
            item.remove(0, 1);
            QRegExp prohibitedRegExp(item);

            // установка режима разбора Wildcard
            prohibitedRegExp.setPatternSyntax(QRegExp::Wildcard);

            // добавление маски в список исключений
            _masksProhibited.append(prohibitedRegExp);
        }
        else
        {
            QRegExp allowedRegExp(item);

            // установка режима разбора Wildcard
            allowedRegExp.setPatternSyntax(QRegExp::Wildcard);

            // добавление маски в список разрешений
            _masksAllowed.append(allowedRegExp);
        }
    }

    // очистка списка предопределенных имен фреймов
    _knownNames.clear();

    // список определений фреймов не может быть неопределен
    if (!definitions)
        return;

    // получение списка имен фреймов
    QList<QString> keys = definitions->keys();


    for (int i = 0; i < keys.count(); i++)
    {
        // имена фреймов принимаются в верхнем регистре
        QString frameName = keys[i].toUpper();

        // если определение фрейма содержит символ '*', фрейм является служебным
        // и не должен использоваться при обмене
        if (!frameName.contains("*"))
        {
            //_knownNames[frameName] =
            allowFrameByMasks(frameName);
        }
    }
}

bool DataFramesMask::allow(QString &name)
{
    // работа ведется с верхним регистром
    QString upperName = name.toUpper();

    // если имя фрейма содержится в списке известных фреймов
    if (_knownNames.contains(upperName))
    {
        // разрешение фрейма берется из списка известных фреймов
        return _knownNames[upperName];
    }
    // разрешение фрейма определяется по считанным маскам из настроек
    return allowFrameByMasks(upperName);
}

bool DataFramesMask::allowFrameByMasks(QString &name)
{
    // по умолчанию - ответ - запрещено
    bool  result = false;
    bool prohibited = false;
    bool allowed = false;

    // работа ведется с именами в верхнем регистре
    QString upperName = name.toUpper();

    // приоритет - поиск по запрещенным маскам
    for (int i = 0; i < _masksProhibited.count(); i++)
    {
        prohibited = _masksProhibited.at(i).exactMatch(upperName);
        if (prohibited)
            break;
    }

    // если нет явного запрета
    if (!prohibited)
    {
        // поиск явного разрешения
        for (int i = 0; i < _masksAllowed.count(); i++)
        {
            allowed = _masksAllowed.at(i).exactMatch(upperName);
            if (allowed)
                break;
        }
    }

    // фрейм будет разрешен, если не входит в запрещенные
    // и явно указан в разрешениях
    result = allowed;

    // запоминание в известных фреймах
    if (!_knownNames.contains(upperName))
        _knownNames[upperName] = result;

    return result;
}


}}
