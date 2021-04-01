#include "DataFramesCommon.h"

namespace Threader {

namespace Frames {


DataAtomDefinition::DataAtomDefinition(const QString &name,
                                       const QString &atomType,
                                       const bool &isScalar,
                                       const QString &defaultValue)
    : _name(name)
    , _atomType(atomType.toLower())
    , _isScalar(isScalar)
    , _defaultValue(defaultValue)
{
}

QString DataAtomDefinition::name() const
{
    return _name;
}

QString DataAtomDefinition::atomType() const
{
    return _atomType;
}

bool DataAtomDefinition::isScalar() const
{
    return _isScalar;
}


DataFrameDefinition::DataFrameDefinition(const QString &name,
                                         const bool isArray,
                                         const unsigned int capacity,
                                         const bool needsTicket,
                                         const unsigned char priority,
                                         const DataAtomsDefinitions &atoms)
    : _name(name)
    , _isArray(isArray)
    , _capacity(capacity)
    , _needsTicket(needsTicket)
    , _priority(priority)
    , _hasAddressDirection(false)
    , _atoms(atoms)
{
}

void DataFrameDefinition::clear()
{
    _atoms.clear();
}

const QString DataFrameDefinition::name() const
{
    return _name;
}

bool DataFrameDefinition::isArray() const
{
    return _isArray;
}

unsigned int DataFrameDefinition::capacity() const
{
    return _capacity;
}

bool DataFrameDefinition::needsTicket() const
{
    return _needsTicket;
}

unsigned char DataFrameDefinition::priority() const
{
    return _priority;
}

bool DataFrameDefinition::hasAddressDirection() const
{
    return _hasAddressDirection;
}

void DataFrameDefinition::setHasAddressDirection(bool hasAddressDirection)
{
    _hasAddressDirection = hasAddressDirection;
}

void DataFrameDefinition::appendAtom(const DataAtomDefinition::Ptr& atom)
{
    _atoms.append(atom);
}

const DataAtomsDefinitions &DataFrameDefinition::atoms() const
{
    return _atoms;
}

int DataFrameDefinition::indexOfAtomByName(const QString &name)
{
    for (int i = 0; i < _atoms.count(); i++)
    {
        if (_atoms.at(i)->name().toUpper() == name.toUpper())
            return i;
    }
    return -1;
}

QString DataFrameDefinition::toString() const
{
    return name();
}

const QString NODE_NAME_DATA_FRAMES = "DataFrames";
const QString NODE_NAME_DATA_FRAME = "DataFrame";
const QString NODE_NAME_DATA_ATOM = "DataAtom";

const QString ATTR_NAME_VERSION = "Version";
const QString ATTR_NAME_NAME = "Name";
const QString ATTR_NAME_NEEDS_TICKET = "NeedsTicket";
const QString ATTR_NAME_IS_ARRAY = "IsArray";
const QString ATTR_NAME_PRIORITY = "Priority";
const QString ATTR_NAME_CAPACITY = "Capacity";

const QString ATTR_NAME_SCALAR = "Scalar";
const QString ATTR_NAME_ATOM_TYPE = "Type";
const QString ATTR_NAME_DEFAULT_VALUE = "Default";

//ГС для Timeout
const QString ATTR_NAME_TIMEOUT = "Timeout";

DataFramesDefinitions::DataFramesDefinitions()
    : _version(QString())
    , _fileName(QString())
    , _error(QString())
{
}

DataFramesDefinitions::~DataFramesDefinitions()
= default;

void DataFramesDefinitions::clear()
{
    _framesHash.clear();
}

void DataFramesDefinitions::parseDataFramesAttributes(QXmlStreamAttributes attributes)
{
    if (attributes.hasAttribute(ATTR_NAME_VERSION))
        _version = attributes.value(ATTR_NAME_VERSION).toString();
}

DataFrameDefinition::Ptr DataFramesDefinitions::parseDataFrameAttributes(QXmlStreamAttributes attributes)
{
    QString name;
    bool needsTicket = false;
    bool isArray = false;
    unsigned int priority = 1;
    unsigned int capacity = 1;

    if (attributes.hasAttribute(ATTR_NAME_NAME))
        name = attributes.value(ATTR_NAME_NAME).toString();

    if (attributes.hasAttribute(ATTR_NAME_NEEDS_TICKET))
    {
        needsTicket = (attributes.value(ATTR_NAME_NEEDS_TICKET)
                       .compare(QString("true"), Qt::CaseInsensitive) == 0);
    }

    if (attributes.hasAttribute(ATTR_NAME_IS_ARRAY))
    {
        isArray = (attributes.value(ATTR_NAME_IS_ARRAY)
                   .compare(QString("true"), Qt::CaseInsensitive) == 0);
    }

    if (attributes.hasAttribute(ATTR_NAME_PRIORITY))
    {
        priority = attributes.value(ATTR_NAME_PRIORITY).toUInt();
        if (priority < 1)
        {
            priority = 1;
        }
    }

    if (attributes.hasAttribute(ATTR_NAME_CAPACITY))
    {
        capacity = attributes.value(ATTR_NAME_CAPACITY).toUInt();
        if (capacity < 1)
        {
            capacity = 1;
        }
    }

    DataFrameDefinition::Ptr frame = std::make_shared<DataFrameDefinition>(
                name, isArray, capacity, needsTicket,
                priority, DataAtomsDefinitions());
    return frame;
}

DataAtomDefinition::Ptr DataFramesDefinitions::parseDataAtomAttributes(QXmlStreamAttributes attributes)
{
    QString name;
    QString atomType = "Undefined";
    bool isScalar = false;
    QString defaultValue;

    if (attributes.hasAttribute(ATTR_NAME_NAME))
        name = attributes.value(ATTR_NAME_NAME).toString();

    if (attributes.hasAttribute(ATTR_NAME_SCALAR))
        isScalar = (attributes.value(ATTR_NAME_SCALAR)
                    .compare(QString("true"), Qt::CaseInsensitive) == 0);

    if (attributes.hasAttribute(ATTR_NAME_ATOM_TYPE))
        atomType = attributes.value(ATTR_NAME_ATOM_TYPE).toString();

    if (attributes.hasAttribute(ATTR_NAME_DEFAULT_VALUE))
        defaultValue = attributes.value(ATTR_NAME_DEFAULT_VALUE).toString();

    DataAtomDefinition::Ptr atom = std::make_shared<DataAtomDefinition>(
                name, atomType, isScalar, defaultValue);

    return atom;
}
void DataFramesDefinitions::appendDefinition(const QString &name,
                                             DataFrameDefinition::Ptr definition)
{
    _framesHash.insert(name.toUpper(), definition);
}

QString DataFramesDefinitions::errorString() const
{
    return _error;
}

QString DataFramesDefinitions::fileName() const
{
    return _fileName;
}

void DataFramesDefinitions::readFromReader(QXmlStreamReader &xml)
{
    QXmlStreamAttributes attributes;

    DataFrameDefinition::Ptr dataFrame;

    // проход по узлам
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        // если начало документа, то продолжаем
        if (token == QXmlStreamReader::StartDocument)
            continue;

        // если начало узла
        if (token == QXmlStreamReader::StartElement) {
            // чтение тега <DataFrames>
            if (xml.name().compare(NODE_NAME_DATA_FRAMES, Qt::CaseInsensitive) == 0) {
                attributes = xml.attributes();
                parseDataFramesAttributes(attributes);
            }
            // чтение тега <DataFrame>
            if (xml.name().compare(NODE_NAME_DATA_FRAME, Qt::CaseInsensitive) == 0) {
                attributes = xml.attributes();
                dataFrame = parseDataFrameAttributes(attributes);
                // _framesHash.insert(functor(dataFrame).toUpper(), dataFrame);
                appendDefinition(dataFrame->name().toUpper(), dataFrame);
            }
            // чтение тега <DataAtom>
            if (xml.name().compare(NODE_NAME_DATA_ATOM, Qt::CaseInsensitive) == 0) {
                attributes = xml.attributes();
                dataFrame->appendAtom(parseDataAtomAttributes(attributes));
            }
        }
    }
}

bool DataFramesDefinitions::readFromFile(const QString &fileName,
                                        const bool &clearList)
{
    if (clearList)
        clear();

    _fileName = fileName;
    // открытие файла
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        _error = file.errorString();
        return false;
    }

    // инициализация чтения XML
    QXmlStreamReader xml(&file);
    readFromReader(xml);

    return true;
}

bool DataFramesDefinitions::readFromResource(const QString &resourceName,
                                             const bool &clearList)
{
    _fileName = resourceName;

    QFile file(resourceName);
    if (!file.exists())
        return false;

    if (clearList)
        clear();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        _error = file.errorString();
        return false;
    }

    // инициализация чтения XML
    QXmlStreamReader xml(&file);
    readFromReader(xml);

    return true;
}

QList<QString> DataFramesDefinitions::keys(bool sorted)
{
    QList<QString> result = _framesHash.keys();
    if (sorted)
//#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
//        qSort(result.begin(), result.end());
//#else
        std::sort(result.begin(), result.end());
//#endif
    return result;
}

int DataFramesDefinitions::count()
{
    return _framesHash.count();
}

DataFrameDefinition::Ptr DataFramesDefinitions::definitionByKey(const QString &key) const
{
    QString keyUpper = key.toUpper();
    if (_framesHash.contains(keyUpper))
    {
        return _framesHash.value(keyUpper, DataFrameDefinition::Ptr());
    }
    return DataFrameDefinition::Ptr();
}


}}
