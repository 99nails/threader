#include "DataFramesFactory.h"

#include <functional>
#include <memory>

namespace Threader {

namespace Frames {


DataFramesFactory::DataFramesFactory(DataFramesDefinitions *definitions)
    : _definitions(definitions)
    , _isOwningDefinitions(!definitions)
{
}

DataFramesFactory::~DataFramesFactory()
{
    releaseDefinitionsStorage();
    releaseAtomsStorage();
}

bool DataFramesFactory::readFromXml(const QString &definitionsFileName)
{
    initializeAtomsStorage();

    _definitionsFileName = definitionsFileName;
    if(definitionsFileName.isEmpty())
    {
        return false;
    }
    if (!_definitions)
        _definitions = createDefinitionStorage();

    return _definitions->readFromFile(definitionsFileName);
}

bool DataFramesFactory::readFromResource(const QString &resouceName,
                                         const bool &clear)
{
    initializeAtomsStorage();

    _definitionsFileName = resouceName;
    if(resouceName.isEmpty())
    {
        return false;
    }
    if (!_definitions)
        _definitions = createDefinitionStorage();

    return _definitions->readFromResource(resouceName, clear);
}

DataFrame::Ptr DataFramesFactory::readFrame(const DataFrameRawData::Ptr& frameRawData)
{
    QByteArray array(frameRawData->buffer(), frameRawData->length());
    DataStream stream(&array);
    return readFrame(stream, true);
}

DataFrame::Ptr DataFramesFactory::readFrame(DataStream &stream, bool readWithoutLength) const
{
    QString frameName;
    FrameSizeType frameSize{};
    DataFrame::Ptr frame;

    // фиксация позиции начала фрейма
    int frameStart = stream.position();

    // чтение размера фрейма
    if (readWithoutLength || stream.read(frameSize))
    {
        // чтение имени фрейма
        if (stream.read(frameName))
        {
            // построение фрейма по имени
            frame = buildFrame(frameName);
            if (frame)
            {
                if (!frame->read(stream))
                {
                    frame.reset();
                }
            }
            else
            {
                // TODO: чтение неизвестного фрейма
            }
        }
    }

    if (!readWithoutLength)
        // установка на позицию начала следующего фрейма
        stream.setPosition(frameStart + static_cast<int>(frameSize + sizeof(frameSize)));

    return frame;
}

DataFrame::Ptr DataFramesFactory::readFrame(QByteArray &data, bool readWithoutLength) const
{
    DataStream stream(&data);
    return readFrame(stream, readWithoutLength);
}

DataFramesList DataFramesFactory::readFrames(DataStream &stream)
{
    DataFramesList result;

    while (!stream.endOfData())
    {
        DataFrame::Ptr frame = readFrame(stream);

        // если фрейм построился
        if (frame)
        {
            result.append(frame);
        }
        else
        {
            break;
        }
    }
    return result;
}

DataFramesList DataFramesFactory::readFrames(QByteArray *data)
{
    DataStream stream(data);
    return readFrames(stream);
}


AbstractDataAtom *DataFramesFactory::buildAtom()
{
    return Q_NULLPTR;
}

DataFrame::Ptr DataFramesFactory::buildFrame(const QString& key) const
{
    if (!_definitions)
        return DataFrame::Ptr();

    DataFrameDefinition::Ptr definition = _definitions->definitionByKey(key);
    if (definition)
    {
        return buildFrame(definition);
    }

    return DataFrame::Ptr();
}

DataFrame::Ptr DataFramesFactory::buildFrame(DataFrameDefinition::Ptr definition) const
{
    const DataAtomsDefinitions& atomsDefintions(definition->atoms());
    AbstractDataAtom* atom;
    auto atomsVector = new AbstractDataAtoms();
    atomsVector->reserve(atomsDefintions.count());
    for (int i = 0; i < atomsDefintions.count(); i++)
    {
        const DataAtomDefinition::Ptr& atomDefinition(atomsDefintions.at(i));

        if (!atomDefinition)
            continue;

        atom = atomByDefinition(atomDefinition);
        atomsVector->append(atom);
    }
    return std::make_shared<DataFrame>(definition, atomsVector);
}

QString DataFramesFactory::definitionsFileName() const
{
    return _definitionsFileName;
}

DataFramesDefinitions *DataFramesFactory::definitions() const
{
    return _definitions;
}

bool DataFramesFactory::isFramesNeedTicket(QList<DataFrameRawData::Ptr> &list)
{
    // если определения не инициализированы, то выход
    if (!_definitions)
        return false;

    // результат по умолчанию - ложь
    bool result = false;

    // обход определений
    for (int i = 0; i < list.count(); i++)
    {
        DataFrameDefinition::Ptr definition = _definitions->definitionByKey(QString(list[i]->name()));
        if (definition)
        {
            // если найдено хоть одно определение с подтверждением
            result |= definition->needsTicket();
            // необходимо квитирование
            if (result)
                return true;
        }
    }
    return result;
}

void DataFramesFactory::initializeAtomsStorage()
{
    releaseAtomsStorage();

    registerAtom(TYPE_NAME_BOOLEAN, new BooleanDataAtom(nullptr));

    registerAtom(TYPE_NAME_UINT8, new UInt8DataAtom(nullptr));
    registerAtom(TYPE_NAME_INT8, new Int8DataAtom(nullptr));

    registerAtom(TYPE_NAME_UINT16, new UInt16DataAtom(nullptr));
    registerAtom(TYPE_NAME_INT16, new Int16DataAtom(nullptr));

    registerAtom(TYPE_NAME_UINT32, new UInt32DataAtom(nullptr));
    registerAtom(TYPE_NAME_INT32, new Int32DataAtom(nullptr));

    registerAtom(TYPE_NAME_UINT64, new UInt64DataAtom(nullptr));
    registerAtom(TYPE_NAME_INT64, new Int64DataAtom(nullptr));

    registerAtom(TYPE_NAME_SINGLE, new SingleDataAtom(nullptr));
    registerAtom(TYPE_NAME_DOUBLE, new DoubleDataAtom(nullptr));

    registerAtom(TYPE_NAME_STRING, new StringDataAtom(nullptr));
    registerAtom(TYPE_NAME_DATETIME, new DateTimeDataAtom(nullptr));
    registerAtom(TYPE_NAME_GUID, new GuidDataAtom(nullptr));
    registerAtom(TYPE_NAME_BYTEARRAY, new ByteArrayDataAtom(nullptr));
}

void DataFramesFactory::releaseAtomsStorage()
{
    foreach (AbstractDataAtom *atom, _registry) {
        delete atom;
    }
}

DataFramesDefinitions *DataFramesFactory::createDefinitionStorage()
{
    releaseDefinitionsStorage();
    _isOwningDefinitions = true;
    return new DataFramesDefinitions();
}

void DataFramesFactory::releaseDefinitionsStorage()
{
    if (_definitions && _isOwningDefinitions)
    {
        delete _definitions;
        _definitions = nullptr;
    }
}

void DataFramesFactory::init(const QString &definitionsFileName)
{
    initializeAtomsStorage();
    readFromXml(definitionsFileName);
}

void DataFramesFactory::init(DataFramesDefinitions *definitions)
{
    initializeAtomsStorage();
    releaseDefinitionsStorage();
    _definitions = definitions;
    _isOwningDefinitions = false;
}

void DataFramesFactory::registerAtom(const QString &type,
                                     AbstractDataAtom *instance)
{
    _registry.insert(type.toLower(), instance);
}

AbstractDataAtom *DataFramesFactory::atomByDefinition(const DataAtomDefinition::Ptr &definition,
                                                      AbstractDataAtom *defaultValue) const
{
    AbstractDataAtom* atom = _registry.value(definition->atomType(), defaultValue);

    if (atom)
    {
        atom = atom->clone(definition.get());
    }

    return atom;
}


}}
