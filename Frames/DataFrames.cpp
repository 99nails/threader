#include "DataFrames.h"

namespace Threader {

namespace Frames {


DataFrame::DataFrame(DataFrameDefinition::Ptr definition,
                     AbstractDataAtoms *atoms)
    : _definition(definition)
    , _atoms(atoms)
    , _capacity(1)
    , _currentIndex(0)
    , _packetId(0)
{
}

DataFrame::~DataFrame()
{
    if (_atoms) {
        foreach (AbstractDataAtom* atom, *_atoms) {
            delete atom;
        }
        delete _atoms;
    }
}

DataFrameDefinition::Ptr DataFrame::definition() const
{
    return _definition;
}

QString DataFrame::name() const
{
    if (!_definition)
        return "";
    return _definition->name();
}

bool DataFrame::needsTicket() const
{
    if (!_definition)
        return false;
    return _definition->needsTicket();
}

AbstractDataAtoms *DataFrame::atoms() const
{
    return _atoms;
}

bool DataFrame::read(DataStream &stream)
{
    bool result = true;
    uint32_t capacity = 1;
    if (_definition->isArray())
    {
        result = stream.read(&capacity, sizeof(capacity));
        if (result && capacity > uint32_t(stream.size() - stream.position()))
            return false;
    }
    if (!result)
        return false;

    setCapacity(capacity);

    // чтение атомов
    for (int i = 0; i < _atoms->count(); i++)
    {
        AbstractDataAtom *atom = _atoms->at(i);
        result = (nullptr != atom);
        if (result)
            result = atom->read(stream);
        if (!result)
            break;
    }

    return result;
}

bool DataFrame::read(DataStream &stream, uint32_t capacity)
{
    bool result = true;
    for (int i = 0; i < _atoms->count(); i++)
    {
        AbstractDataAtom *atom = _atoms->at(i);
        result = (nullptr != atom);
        if (result)
        {
            if (!atom->definition()->isScalar())
            {
                auto savedPos = i;
                for (uint c = 0; c < capacity; c++)
                {
                    i = savedPos;
                    for(int j = savedPos;j < _atoms->count() &&
                         !_atoms->at(i)->definition()->isScalar();j++,i++)
                    {
                        if (_atoms->at(j)->readItem(stream, c))
                            continue;
                        return false;
                    }
                }
                continue;
            }
            else
                result = atom->read(stream);
        }
        else
            break;
    }
    return result;
}

bool DataFrame::read(DataStream *stream)
{
    return read(*stream);
}

bool DataFrame::write(DataStream &stream) const
{
    bool result;
    uint32_t size32 = size();
    //    if (size32 > 0xFFFF)
    //        return false;
    FrameSizeType frameSize = size32;
    QString frameName = (QString)name();
    result = stream.write(frameSize) &&
            stream.write(frameName);
    if (result && definition()->isArray())

        result = stream.write(_capacity);
    if (result) {
        for (int i = 0; i < atoms()->count(); i++) {
            AbstractDataAtom *atom = _atoms->at(i);
            result = (nullptr != atom);
            if (result)
                result = atom->write(stream);
            if (!result)
                break;
        }
    }
    return result;
}

bool DataFrame::write(DataStream *stream) const
{
    if (!stream)
        return false;
    return write(*stream);
}

uint32_t DataFrame::capacity() const
{
    return _capacity;
}

void DataFrame::setCapacity(const uint32_t &capacity)
{
    // проверка значения
    uint32_t newSize = capacity;
    if (newSize < 1)
        newSize = 1;
    // установка значения
    _capacity = newSize;
    foreach (AbstractDataAtom *atom, *_atoms)
        if (atom && atom->definition())
            atom->setCapacity(atom->definition()->isScalar() ? 1 : _capacity);
}

uint32_t DataFrame::calcCapacityFromAtoms(uint32_t totalSize) const
{
    uint32_t capacity = 0, totalSizeScalar = 0, totalSizeArray = 0;
    for (auto atom : *_atoms)
    {
        if (atom && atom->definition())
        {
            if (atom->definition()->isScalar())
                totalSizeScalar += atom->size();
            else
                totalSizeArray += atom->size();
        }
    }
    if (totalSizeArray)
        capacity = (totalSize - totalSizeScalar)/totalSizeArray;//FIX: command?
    return capacity;
}

uint32_t DataFrame::size() const
{
    uint32_t result = name().length() + sizeof(char);
    if (_definition->isArray())
            result += sizeof(_capacity);
    foreach (AbstractDataAtom *atom, *_atoms)
        result += atom->size();
    return result;
}

uint32_t DataFrame::currentIndex() const
{
    return _currentIndex;
}

void DataFrame::setCurrentIndex(const uint32_t &currentIndex)
{
    _currentIndex = currentIndex;
    foreach (AbstractDataAtom *atom, *_atoms)
        if (nullptr != atom)
            atom->setCurrentIndex(_currentIndex);
}

AbstractDataAtom *DataFrame::atomByName(const QString &name) const
{
    AbstractDataAtom *atom;
    for (int i = 0; i < atoms()->count(); i++) {
        atom = atoms()->at(i);
        if (0 == atom->definition()->name().compare(name, Qt::CaseInsensitive))
            return atom;
    }
    return nullptr;
}

QString DataFrame::toString() const
{
    return QString("%1: %2").arg(_definition->toString(), atomsToString());
}

QString DataFrame::atomsToString() const
{
    QString res;
    for(AbstractDataAtom* atom : *atoms())
        res.append(" <").append(atom->toString()).append(">");
    return res;
}

uint32_t DataFrame::packetId() const
{
    return _packetId;
}

void DataFrame::setPacketId(const uint32_t &packetId)
{
    _packetId = packetId;
}

DataFrameRawData::Ptr DataFrame::dataFrameRawData()
{
    DataFrameRawData::Ptr result = nullptr;
    // сериализация фрейма DataFrame в поток
    QByteArray array;
    DataStream stream(&array);
    write(&stream);

    // восстановление фрейма DataFrameRawData
    DataFrameRawDataList list;
    DataFrameRawData::parse(array, list);

    // если восстановление прошло удачно, то возврат фрейма DataFrameRawData
    if (list.count() > 0)
    {
        result = list.at(0);
        result->setDefinition(_definition);
    }
    return result;
}

}}
