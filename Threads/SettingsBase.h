#pragma once

#include "../threader_global.h"

#include <QHash>
#include <QSettings>
#include <QVector>

#include <memory>

namespace Threader
{

namespace Threads
{

enum class SettingsLocation
{
    HomeDir,
    NearExecutable,
    DirectReference
};

class THREADERSHARED_EXPORT SettingsBase
{
public:
    using Ptr = std::shared_ptr<SettingsBase>;

public:
    virtual ~SettingsBase() = default;
    virtual bool read(QSettings &settings) = 0;
    virtual bool write(QSettings &settings) = 0;
};


using SettingsBaseVector = QVector<SettingsBase::Ptr>;


using SettingsBaseHash = QHash<QString, SettingsBase::Ptr>;


class THREADERSHARED_EXPORT SettingsBundle : public SettingsBase
{
public:
    using Ptr = std::shared_ptr<SettingsBundle>;

public:
    virtual bool readBundle();
    virtual bool writeBundle();

    QString fileName() const;

protected:
    void setFileName(const QString &fileName);

    bool read(QSettings &settings) override;

    bool write(QSettings &settings) override;

    SettingsBase::Ptr registerSettings(const SettingsBase::Ptr &settings);

private:
    SettingsBaseVector _settingsBundle;
    QString _fileName = QString();
};

}}
