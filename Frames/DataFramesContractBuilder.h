#pragma once

#include "DataFramesCommon.h"
#include "DataFramesLiterals.h"

#include "../threader_global.h"

#include <QString>
#include <QHash>

namespace Threader {

namespace Frames {


class THREADERSHARED_EXPORT DataFramesContractBuilder
{
public:
    explicit DataFramesContractBuilder(DataFramesDefinitions *definitions,
                                       const QString &projectName,
                                       const QString &output,
                                       const QString &projNamespace);

    virtual ~DataFramesContractBuilder();

    bool build(bool verbose = false);

    QString projectName() const;

    QString headerFileName() const;

    QString implementationFileName() const;

    QString headerContent() const;

    QString implementationContent() const;

    void addNamespaces(QVariantHash &templates);

private:
    DataFramesDefinitions *_definitions;
    QString _projectName;
    QString _projNamespace;
    QString _headerFileName;
    QString _implementationFileName;
    QString _headerContent;
    QString _implementationContent;
    QHash<QString, QString> _cppTypes;
    QHash<QString, QString> _atomsTypes;

    QString buildHeaderFramesConstantsDefinitions(const QStringList &framesNames);
    QString buildHeaderAtomsConstantsDefinitions(const QStringList &atomsNames);
    QString buildHeaderFunctionsDefinitions(const QStringList &framesNames);
    QString buildFunctionsArguments(DataFrameDefinition::Ptr definition);
    QString buildHeader(const QStringList &framesNames, const QStringList &atomsNames);
    QString buildImplementationFramesConstantsDefinitions(const QStringList &framesNames);
    QString buildImplementationAtomsConstantsDefinitions(const QStringList &atomsNames);
    QString buildImplementationAssignAtoms(DataFrameDefinition::Ptr definition);
    QString buildFunctionsImplementation(const QStringList &framesNames);
    QString buildImplementation(const QStringList &framesNames, const QStringList &atomsNames);
};
}}
