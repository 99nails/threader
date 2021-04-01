#include "SettingsBase.h"

#include <QFileInfo>
#include <QTextCodec>

namespace Threader
{

namespace Threads
{

bool SettingsBundle::readBundle()
{
    return true;
}

bool SettingsBundle::writeBundle()
{
    return true;
}

QString SettingsBundle::fileName() const
{
    return _fileName;
}

void SettingsBundle::setFileName(const QString &fileName)
{
    _fileName = fileName;
}

bool SettingsBundle::read(QSettings &settings)
{
    bool result = true;
    for (auto s: _settingsBundle)
    {
        bool r = s->read(settings);
        if (!r)
            result = false;
    }
    return result;
}

bool SettingsBundle::write(QSettings &settings)
{
    bool result = true;
    for (auto s: _settingsBundle)
    {
        if (!s->write(settings))
            result = false;
    }
    return result;
}

SettingsBase::Ptr SettingsBundle::registerSettings(const SettingsBase::Ptr &settings)
{
    _settingsBundle.append(settings);
    return settings;
}

}}
