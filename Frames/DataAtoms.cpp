#include "DataAtoms.h"


namespace Threader {

namespace Frames {


AbstractDataAtom::AbstractDataAtom(const DataAtomDefinition *definition)
    : _definition(definition)
    , _currentIndex(0)
{
}

AbstractDataAtom::~AbstractDataAtom()
{
}

const DataAtomDefinition *AbstractDataAtom::definition() const
{
    return _definition;
}

uint32_t AbstractDataAtom::currentIndex() const
{
    return _currentIndex;
}

void AbstractDataAtom::setCurrentIndex(const uint32_t &currentIndex)
{
    _currentIndex = currentIndex;
    if (capacity() <= _currentIndex)
        _currentIndex = capacity() - 1;
}

StringDataAtom::StringDataAtom(const DataAtomDefinition *definition)
    : BaseDataAtom<QString>(definition)
{
    clear();
}

AbstractDataAtom *StringDataAtom::clone(const DataAtomDefinition *definition) const
{
    return new StringDataAtom(definition);
}

uint32_t StringDataAtom::size()
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < capacity(); i++)
        result += (_valuesVector.at(i).length() + 1) * sizeof(char);
    return result;
}

ByteArrayDataAtom::ByteArrayDataAtom(const DataAtomDefinition *definition)
    : BaseDataAtom(definition)
{
}

ByteArrayDataAtom *ByteArrayDataAtom::clone(const DataAtomDefinition *definition) const
{
    return new ByteArrayDataAtom(definition);
}

void ByteArrayDataAtom::setCapacity(const uint32_t &capacity)
{
    // тут нельзя менять ёмкость, тут всегда один элемент
    Q_UNUSED(capacity);
}


}}
