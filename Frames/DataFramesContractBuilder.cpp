#include "DataFramesContractBuilder.h"
#include "DataAtoms.h"

#include "../ThirdParty/mustache/mustache.h"

#include <QList>
#include <QStringList>
#include <QVariantHash>
#include <QTextStream>


namespace Threader {

namespace Frames {


QString toNameCase(const QString fromString) {
    QString result = fromString;
    bool first = true;
    for (int i = 0; i < result.length(); i++) {
        if (result[i].isLetterOrNumber()) {
            result[i] = (first) ? result[i].toTitleCase() : result[i].toLower();
            first = false;
        } else {
            first = true;
        }
    }
    return result;
}

DataFramesContractBuilder::DataFramesContractBuilder(DataFramesDefinitions *definitions,
                                                     const QString &projectName,
                                                     const QString &output,
                                                     const QString &projNamespace)
    : _definitions(definitions),
      _projectName(projectName),
      _projNamespace(projNamespace),
      _headerFileName(output + projectName + "DataFramesFactory.h"),
//      _helperHeaderName(output + projectName + "DataFramesHelpers.h"),
      _implementationFileName(output + projectName + "DataFramesFactory.cpp"),
//      _helperImplementationName(output + projectName + "DataFramesHelpers.cpp"),
      _headerContent(""),
      _implementationContent("")
{
    _cppTypes[TYPE_NAME_BOOLEAN.toLower()] = "bool";
    _cppTypes[TYPE_NAME_UINT8.toLower()] = "uint8_t";
    _cppTypes[TYPE_NAME_INT8.toLower()] = "int8_t";
    _cppTypes[TYPE_NAME_UINT16.toLower()] = "uint16_t";
    _cppTypes[TYPE_NAME_INT16.toLower()] = "int16_t";
    _cppTypes[TYPE_NAME_UINT32.toLower()] = "uint32_t";
    _cppTypes[TYPE_NAME_INT32.toLower()] = "int32_t";
    _cppTypes[TYPE_NAME_UINT64.toLower()] = "uint64_t";
    _cppTypes[TYPE_NAME_INT64.toLower()] = "int64_t";
    _cppTypes[TYPE_NAME_SINGLE.toLower()] = "float";
    _cppTypes[TYPE_NAME_DOUBLE.toLower()] = "double";
    _cppTypes[TYPE_NAME_DATETIME.toLower()] = "QDateTime";
    _cppTypes[TYPE_NAME_STRING.toLower()] = "QString";
    _cppTypes[TYPE_NAME_BYTEARRAY.toLower()] = "QByteArray";
    _cppTypes[TYPE_NAME_GUID.toLower()] = "QUuid";

    _atomsTypes[TYPE_NAME_BOOLEAN.toLower()] = "BooleanDataAtom";
    _atomsTypes[TYPE_NAME_UINT8.toLower()] = "UInt8DataAtom";
    _atomsTypes[TYPE_NAME_INT8.toLower()] = "Int8DataAtom";
    _atomsTypes[TYPE_NAME_UINT16.toLower()] = "UInt16DataAtom";
    _atomsTypes[TYPE_NAME_INT16.toLower()] = "Int16DataAtom";
    _atomsTypes[TYPE_NAME_UINT32.toLower()] = "UInt32DataAtom";
    _atomsTypes[TYPE_NAME_INT32.toLower()] = "Int32DataAtom";
    _atomsTypes[TYPE_NAME_UINT64.toLower()] = "UInt64DataAtom";
    _atomsTypes[TYPE_NAME_INT64.toLower()] = "Int64DataAtom";
    _atomsTypes[TYPE_NAME_SINGLE.toLower()] = "FloatDataAtom";
    _atomsTypes[TYPE_NAME_DOUBLE.toLower()] = "DoubleDataAtom";
    _atomsTypes[TYPE_NAME_DATETIME.toLower()] = "DateTimeDataAtom";
    _atomsTypes[TYPE_NAME_STRING.toLower()] = "StringDataAtom";
    _atomsTypes[TYPE_NAME_BYTEARRAY.toLower()] = "ByteArrayDataAtom";
    _atomsTypes[TYPE_NAME_GUID.toLower()] = "GuidDataAtom";
}

DataFramesContractBuilder::~DataFramesContractBuilder()
{
    delete _definitions;
}

QString DataFramesContractBuilder::buildHeaderFramesConstantsDefinitions(const QStringList &framesNames)
{
    QString result = "";

    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < framesNames.count(); i++) {
        QString frameName = framesNames.at(i);
        if (frameName.contains('*', Qt::CaseInsensitive))
            continue;

        templates[FRAME_NAME_UPPER] = frameName.toUpper();
        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_FRAME_CONSTANT, &context);
    }
    return result;
}

QString DataFramesContractBuilder::buildHeaderAtomsConstantsDefinitions(const QStringList &atomsNames)
{
    QString result = "";

    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < atomsNames.count(); i++) {
        templates[TEMPLATE_ATOM_NAME_UPPER] = atomsNames.at(i).toUpper();
        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_ATOM_CONSTANT, &context);
    }
    return result;
}

QString DataFramesContractBuilder::buildHeaderFunctionsDefinitions(const QStringList &framesNames)
{
    QString result = "";

    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < framesNames.count(); i++) {
        QString frameName = framesNames.at(i);
        if (frameName.contains('*', Qt::CaseInsensitive))
            continue;

        DataFrameDefinition::Ptr definition = _definitions->definitionByKey(frameName);
        templates[FRAME_NAME] = toNameCase(frameName);
        templates[TEMPLATE_ARGUMENTS] = buildFunctionsArguments(definition);
        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_FUNCTION_DEFINITION, &context);
    }
    return result;
}

QString DataFramesContractBuilder::buildFunctionsArguments(DataFrameDefinition::Ptr definition)
{
    QString result = "";
    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < definition->atoms().count(); i++) {
        DataAtomDefinition::Ptr atomDefinition = definition->atoms().at(i);

        QString type = atomDefinition->atomType();
        type = (_cppTypes.contains(type)) ? _cppTypes[type] : "UnknownType";
        templates[TEMPLATE_TYPE] = type;

        QString argument = "";
        if (result.length() > 0)
            result += ", ";
        argument += atomDefinition->name();
        if (argument.length() > 0)
            argument[0] = argument[0].toLower();
        templates[TEMPLATE_ARGUMENT] = argument;

        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_FUNCTION_ARGUMENT, &context);
    }
    return result;
}

void DataFramesContractBuilder::addNamespaces(QVariantHash& templates)

{
    QStringList namespss = _projNamespace.split("::");
    if (namespss.count()>0)
    {
        QString NS_OPEN, NS_CLOSE;
//        for (int i = 0; i < namespss.count(); i++)
//        {
//            NS_OPEN += QString(NS_Format).arg(namespss[i]);
//            NS_CLOSE += "}";
//        }
        NS_OPEN += DataFramesUsing;
        NS_OPEN += ConstNSUsing + _projectName + "DataFramesFactory;\r\n";

        templates[TEMPLATE_FRAME_NAMESPACE_OPEN] = NS_OPEN;
//        templates[TEMPLATE_FRAME_NAMESPACE_CLOSE] = NS_CLOSE;
//        templates[TEMPLATE_FRAME_NAMESPACE_OPEN] = "";
        templates[TEMPLATE_FRAME_NAMESPACE_CLOSE] = "";
    }
}

QString DataFramesContractBuilder::buildHeader(const QStringList &framesNames, const QStringList &atomsNames)
{
    QVariantHash templates;
    addNamespaces(templates);
    templates[TEMPLATE_PROJECT_NAME] = _projectName;
    templates[TEMPLATE_PROJECT_NAME_UPPER] = _projectName.toUpper();
    templates[TEMPLATE_FRAMES_NAMES_CONSTANTS] = buildHeaderFramesConstantsDefinitions(framesNames);
    templates[TEMPLATE_ATOMS_NAMES_CONSTANTS] = buildHeaderAtomsConstantsDefinitions(atomsNames);
    templates[TEMPLATE_FUNCTIONS] = buildHeaderFunctionsDefinitions(framesNames);

    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(templates);
    QString header = renderer.render(TEMPLATE_HEADER_FILE, &context);

    return header;
}

QString DataFramesContractBuilder::buildImplementationFramesConstantsDefinitions(const QStringList &framesNames)
{
    QString result = "";

    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < framesNames.count(); i++) {
        QString frameName = framesNames.at(i);
        if (frameName.contains('*', Qt::CaseInsensitive))
            continue;

        templates[TEMPLATE_PROJECT_NAME] = _projectName;
        templates[FRAME_NAME_UPPER] = frameName.toUpper();
        templates[FRAME_NAME] = frameName;
        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_FRAME_CONSTANT_IMPLEMENTATION, &context);
    }
    return result;
}

QString DataFramesContractBuilder::buildImplementationAtomsConstantsDefinitions(const QStringList &atomsNames)
{
    QString result = "";

    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < atomsNames.count(); i++) {
        templates[TEMPLATE_PROJECT_NAME] = _projectName;
        templates[TEMPLATE_ATOM_NAME_UPPER] = atomsNames.at(i).toUpper();
        templates[TEMPLATE_ATOM_NAME] = atomsNames.at(i);
        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_ATOM_CONSTANT_IMPLEMENTATION, &context);
    }
    return result;

}

QString DataFramesContractBuilder::buildImplementationAssignAtoms(DataFrameDefinition::Ptr definition)
{
    QString result = "";
    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < definition->atoms().count(); i++) {
        DataAtomDefinition::Ptr atomDefinition = definition->atoms().at(i);

        QString type = atomDefinition->atomType();
        type = (_atomsTypes.contains(type)) ? _atomsTypes[type] : "UnknownType";
        templates[TEMPLATE_ATOM_TYPE] = type;

        QString argument = atomDefinition->name();
        if (argument.length() > 0)
            argument[0] = argument[0].toLower();
        templates[TEMPLATE_ATOM_NAME] = argument;
        templates[TEMPLATE_ATOM_NAME_UPPER] = atomDefinition->name().toUpper();

        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_ASSIGN_ATOM, &context);
    }
    return result;
}

QString DataFramesContractBuilder::buildFunctionsImplementation(const QStringList &framesNames)
{
    QString result = "";

    QVariantHash templates;
    Mustache::Renderer renderer;

    for (int i = 0; i < framesNames.count(); i++) {
        QString frameName = framesNames.at(i);
        if (frameName.contains('*', Qt::CaseInsensitive))
            continue;

        DataFrameDefinition::Ptr definition = _definitions->definitionByKey(frameName);

        templates[TEMPLATE_PROJECT_NAME] = _projectName;
        templates[FRAME_NAME] = toNameCase(frameName);
        templates[FRAME_NAME_UPPER] = frameName.toUpper();
        templates[TEMPLATE_ARGUMENTS] = buildFunctionsArguments(definition);
        templates[TEMPLATE_ASSIGN_ATOMS] = buildImplementationAssignAtoms(definition);

        Mustache::QtVariantContext context(templates);
        result += renderer.render(TEMPLATE_FUNCTION_IMPLEMENTATION, &context);
    }
    return result;
}

QString DataFramesContractBuilder::buildImplementation(const QStringList &framesNames, const QStringList &atomsNames)
{
    QVariantHash templates;
    addNamespaces(templates);
    templates[TEMPLATE_PROJECT_NAME] = _projectName;
    templates[TEMPLATE_PROJECT_NAME_UPPER] = _projectName.toUpper();
    templates[TEMPLATE_FRAMES_NAMES_CONSTANTS] = buildImplementationFramesConstantsDefinitions(framesNames);
    templates[TEMPLATE_ATOMS_NAMES_CONSTANTS] = buildImplementationAtomsConstantsDefinitions(atomsNames);
    templates[TEMPLATE_FUNCTIONS] = buildFunctionsImplementation(framesNames);

    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(templates);
    QString result = renderer.render(TEMPLATE_IMPLEMENTATION_FILE, &context);

    return result;
}

bool DataFramesContractBuilder::build(bool verbose)
{
    if (nullptr == _definitions)
        return false;

    // получение и сортировка имен фреймов
    QStringList framesNames = QStringList(_definitions->keys());
    if (framesNames.count() < 1)
        return false;
    framesNames.sort(Qt::CaseInsensitive);

    // получение и сортировка имен атомов
    QStringList atomsNames;
    for (int i = 0; i < framesNames.count(); i++) {

        DataFrameDefinition::Ptr frameDefinition = _definitions->definitionByKey(framesNames.at(i));
        if (nullptr == frameDefinition)
            continue;

        for (int j = 0; j < frameDefinition->atoms().count(); j++) {
            DataAtomDefinition::Ptr atomDefinition = frameDefinition->atoms().at(j);
            if (nullptr == atomDefinition)
                continue;

            QString atomName = atomDefinition->name();
            if (!atomsNames.contains(atomName, Qt::CaseInsensitive))
                atomsNames << atomName;
        }
    }
    atomsNames.sort(Qt::CaseInsensitive);


    _headerContent = buildHeader(framesNames, atomsNames).replace("&amp;amp;", "&");
    _implementationContent = buildImplementation(framesNames, atomsNames).replace("&quot;", "\"").replace("&amp;amp;", "&").replace("&amp;lt;", "<").replace("&amp;gt;", ">");

    if (verbose)
    {
        QTextStream output(stdout);

        output << "\r\n" + _headerFileName + ":\r\n\r\n";
        output << _headerContent;


        output << "\r\n" + _implementationFileName + ":\r\n\r\n";
        output << _implementationContent;
    }

    return true;
}

QString DataFramesContractBuilder::projectName() const
{
    return _projectName;
}

QString DataFramesContractBuilder::headerFileName() const
{
    return _headerFileName;
}

QString DataFramesContractBuilder::implementationFileName() const
{
    return _implementationFileName;
}

QString DataFramesContractBuilder::headerContent() const
{
    return _headerContent;
}

QString DataFramesContractBuilder::implementationContent() const
{
    return _implementationContent;
}

}}
