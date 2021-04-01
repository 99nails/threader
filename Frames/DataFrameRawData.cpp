#include "DataFrameRawData.h"

#include <QFile>


#define PRIORITY_DEFAULT 1


namespace Threader {

namespace Frames {


int DataFrameRawData::_referenceCount(0);

QMutex DataFrameRawData::_mutexReferenceCount;

DataFrameRawData::DataFrameRawData(const char *data,
                                   const FrameSizeType &length)
    : _nameString("")
    , _packetId(-1)
    , _definition(nullptr)
{
    incrementReferenceCount();
    // если данных нет или первый символ 0, то фрейм ошибочен и должен игнорироваться
    if (!data || 0 == length || 32 >= data[0])
    {
        _buffer = nullptr;
        _data = nullptr;
        _name = nullptr;
        _length = 0;
        _dataLength = 0;
        _isValid = false;
        return;
    }

    // весь фрейм состоит из заголовка
    // данных нет
    _data = nullptr;
    _dataLength = 0;
    _isValid = true;

    // запоминание общей длины
    _length = length;

    // выделение буфера для хранения с увеличением размена на 1
    // для гараньтированного завершения на 0
    _buffer = new char[length + 1];

    // подготовка и перенос данных во внутреннюю память
    memset(_buffer, 0, static_cast<size_t>(length + 1));
    memcpy(_buffer, data, static_cast<size_t>(length));

    // имя фрейма в начале блока данных
    _name = _buffer;

    // данные начинаются после окончания заголовка нулевым символом
    _data = _name + 1;

    FrameSizeType zeroPosition = 0;
    for (uint i = 0; i < length; i++)
    {
        if (0 == _buffer[i])
        {
            zeroPosition = i;
            break;
        }
    }

    // если 0 есть в блоке разбираемых данных
    // и таким образом распознан заголовок
    // можно описать короче, но так НАГЛЯДНЕЕ
    if (zeroPosition > 0)
    {
        // если данные есть
        if (zeroPosition + 1 < _length)
        {
            _data = _buffer + zeroPosition + 1;
            _dataLength = _length - zeroPosition - 1;
            _nameString = QString(_name);
        }
    }
}

DataFrameRawData::~DataFrameRawData()
{    
    delete [] _buffer;
    _buffer = nullptr;
    decrementReferenceCount();
}

DataFrameRawData::Ptr DataFrameRawData::clone()
{
    DataFrameRawData::Ptr result = std::make_shared<DataFrameRawData>(_buffer, _length);
    result->setDefinition(this->definition());
    return result;
}

bool DataFrameRawData::isValid() const
{
    return _isValid;
}

char *DataFrameRawData::buffer() const
{
    return _buffer;
}

char *DataFrameRawData::name() const
{
    return _name;
}

char *DataFrameRawData::data() const
{
    return _data;
}

FrameSizeType DataFrameRawData::length() const
{
    return _length;
}

int DataFrameRawData::streamSize() const
{
    return _length + static_cast<int>(sizeof(uint16_t));
}

void DataFrameRawData::appendByteArray(QByteArray &array, bool clear)
{
    // FIXME через жопу, надо передумать и переделать
    if (clear)
        array.clear();
    // запись размера
    array.append((char*)&_length, sizeof(_length));
    // запись данных
    array.append(_buffer, _length);
}

int DataFrameRawData::dataLength() const
{
    return _dataLength;
}

QString DataFrameRawData::toString() const
{
    return _nameString;
}

uint8_t DataFrameRawData::priority() const
{
    if (_definition)
        return _definition->priority();
    return PRIORITY_DEFAULT;
}

int DataFrameRawData::packetId() const
{
    return _packetId;
}

void DataFrameRawData::setPacketId(int packetId)
{
    _packetId = packetId;
}

DataFrameDefinition::Ptr DataFrameRawData::definition()
{
    return _definition;
}

void DataFrameRawData::setDefinition(DataFrameDefinition::Ptr definition)
{
    _definition = definition;
}


void DataFrameRawData::parse(QByteArray &source, QList<DataFrameRawData::Ptr> &result)
{
    // очистка списка результатов
    result.clear();
    // получение размера разбираемых данных
    auto sourceSize = static_cast<uint32_t>(source.size());

    // проверка размера разбираемых данных
    if (sourceSize <= 0)
        return;

    // получение указателя на начало разбираемых данных
    char *sourcePointer = const_cast<char*>(source.constData());

    // переменная размера фрейма
    FrameSizeType frameLength = 0;
    // текущая позиция в буфере при разборе данных
    FrameSizeType position = 0;

    // пока из данных возможно получить длину фрейма
    while (position + sizeof(frameLength) <= sourceSize)
    {
        // получение длины фрейма
        memcpy(&frameLength, sourcePointer + position, sizeof(frameLength));
        // сдвиг на размер длины фрейма
        position += sizeof(frameLength);

        // если есть возможность прочитать фрейм длины frameLength из данных
        if (position + frameLength <= sourceSize)
        {
            // формирование неразобранных данных фрейма
            DataFrameRawData::Ptr dataFrameRawData = std::make_shared<DataFrameRawData>(sourcePointer + position, frameLength);
            // проверка корректности разбора данных фрейма
            if (dataFrameRawData->isValid())
                result.append(dataFrameRawData);
            // сдвиг на длину фрейма
            position += frameLength;
        }
        else
            // выход при невозможности прочитать фрейм длины frameLength из данных
            break;
    }
}

bool DataFrameRawData::loadFromFile(const QString &fileName,
                                    const DataFramesDefinitions *definitions,
                                    QList<DataFrameRawData::Ptr> &list)
{
    // очистка списка фреймов
    list.clear();

    // открытие файла
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        // чтение файла
        QByteArray fileContent = file.readAll();
        file.close();

        // построение списка фреймов
        parse(fileContent, list);

        if (definitions)
        {
            for (int i = 0; i < list.count(); i++)
            {
                const DataFrameRawData::Ptr& frame = list.at(i);
                frame->setDefinition(definitions->definitionByKey(frame->toString()));
            }
        }

        return true;
    }

    return false;
}

int DataFrameRawData::referenceCount()
{
    QMutexLocker locker(&_mutexReferenceCount);
    return _referenceCount;
}

void DataFrameRawData::incrementReferenceCount()
{
    QMutexLocker locker(&_mutexReferenceCount);
    _referenceCount++;
}

void DataFrameRawData::decrementReferenceCount()
{
    QMutexLocker locker(&_mutexReferenceCount);
    _referenceCount--;
}


const QString MessageDataFramesRaw::MESSAGE_NAME = "Message.DataFramesRaw";

MessageDataFramesRaw::MessageDataFramesRaw(const DataFrameRawDataList &frames,
                                           const QString alias)
    : MessageBase(MESSAGE_NAME)
    , _alias(alias)
{
    _frames.append(frames);
}

}}
