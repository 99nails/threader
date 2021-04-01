#pragma once

#include "../threader_global.h"


#include "DataFramesCommon.h"
#include "../Utils/DataStream.h"

#include <QVector>
#include <QDateTime>
#include <QUuid>

namespace Threader {

namespace Frames {


using namespace Threader::Utils;


class THREADERSHARED_EXPORT AbstractDataAtom
{
public:
    explicit AbstractDataAtom(const DataAtomDefinition *definition);
    virtual ~AbstractDataAtom();

    virtual AbstractDataAtom *clone(const DataAtomDefinition *definition) const = 0;

    virtual void clear() = 0;
    virtual uint32_t size() = 0;

    virtual bool read(DataStream &stream) = 0;
    virtual bool readItem(DataStream &stream, uint = 0) { return read(stream); }
    virtual bool write(DataStream &stream) = 0;

    virtual uint32_t capacity() const = 0;
    virtual void setCapacity(const uint32_t &capacity) = 0;

    const DataAtomDefinition *definition() const;

    virtual uint32_t currentIndex() const;
    virtual void setCurrentIndex(const uint32_t &currentIndex);

    virtual QString toString() const = 0;
private:
    const DataAtomDefinition *_definition;
    uint32_t _currentIndex;
};

using AbstractDataAtoms = QVector<AbstractDataAtom*>;

template <class ValueType>
class BaseDataAtom : public AbstractDataAtom {
public:
    BaseDataAtom(const DataAtomDefinition *definition);
    virtual ~BaseDataAtom() override;

    AbstractDataAtom *clone(const DataAtomDefinition *definition) const override;

    void clear() override;
    uint32_t size() override;

    virtual void fillData(ValueType &value);
    virtual ValueType value() const;
    virtual QList<ValueType> values() const;
    virtual void setValue(const ValueType &value);
    virtual void setValue(const QList<ValueType> &values);

    bool read(DataStream &stream) override;
    bool readItem(DataStream &stream, uint index = 0) override
    {
        if (definition()->isScalar())
            return read(stream);

        if (_valuesVector.size() < int(index) + 1 && index > 0)
            _valuesVector.resize(index + 1);
        //if (definition()-> <= int(currentIndex()))
          //  return false;
        if (stream.read(_valuesVector[index]))
        {
//            setCapacity(1 + currentIndex());
//            setCurrentIndex(1 + currentIndex());
            return true;
        }
        return false;
    }

    bool write(DataStream &stream) override;

    uint32_t capacity() const override;
    void setCapacity(const uint32_t &capacity) override;

    QString toString() const override;

protected:
    QVector<ValueType> _valuesVector;
};

template <class ValueType>
BaseDataAtom<ValueType>::BaseDataAtom(const DataAtomDefinition *definition)
    : AbstractDataAtom(definition)
{
    _valuesVector.resize(1);
}

template <class ValueType>
BaseDataAtom<ValueType>::~BaseDataAtom()
{
}


template <class ValueType>
AbstractDataAtom *BaseDataAtom<ValueType>::clone(const DataAtomDefinition *definition) const
{
    return new BaseDataAtom<ValueType>(definition);
}

template <class ValueType>
void BaseDataAtom<ValueType>::clear()
{
    int size = capacity();
    _valuesVector.clear();
    _valuesVector.resize(size);
}

template <class ValueType>
uint32_t BaseDataAtom<ValueType>::size()
{
    int size = 0;
    for(uint32_t i = 0; i < capacity(); ++i)
    {
        size += DataStream::valueSize(_valuesVector.at(i));
    }
    return uint32_t(size);
}


template <class ValueType>
void BaseDataAtom<ValueType>::fillData(ValueType &value)
{
    _valuesVector.fill(value);
}

template <class ValueType>
ValueType BaseDataAtom<ValueType>::value() const
{
    return _valuesVector.at(currentIndex());
}

template<class ValueType>
QList<ValueType> BaseDataAtom<ValueType>::values() const
{
    return _valuesVector.toList();
}

template <class ValueType>
void BaseDataAtom<ValueType>::setValue(const ValueType &value)
{
    _valuesVector.replace(currentIndex(), value);
}

template<class ValueType>
void BaseDataAtom<ValueType>::setValue(const QList<ValueType> &values)
{
    for(int i = 0; i < values.size() && i < int(capacity()); ++i)
    {
        _valuesVector[i] = values.at(i);
    }
}

template <class ValueType>
bool BaseDataAtom<ValueType>::read(DataStream &stream)
{
    clear();
    for(int i = 0; i < _valuesVector.size(); ++i)
    {
        if (!stream.read(_valuesVector[i]))
        {
            return false;
        }
    }
    return true;
}

template <class ValueType>
bool BaseDataAtom<ValueType>::write(DataStream &stream)
{
    for(int i = 0; i < _valuesVector.size(); ++i)
    {
        if (!stream.write(_valuesVector[i]))
        {
            return false;
        }
    }
    return true;
}

template <class ValueType>
uint32_t BaseDataAtom<ValueType>::capacity() const
{
    return _valuesVector.size();
}

template <class ValueType>
void BaseDataAtom<ValueType>::setCapacity(const uint32_t &capacity)
{
    uint32_t newSize = capacity;
    if (0 == newSize)
        newSize = 1;
    _valuesVector.resize(newSize);
    if (currentIndex() >= newSize)
        setCurrentIndex(newSize - 1);
}

template<typename ValueType>
inline QString atomToString(const BaseDataAtom<ValueType>& atom)
{
    return QString("%1: %2")
            .arg(atom.definition()->name())
            .arg(atom.value());
}

template<class ValueType>
QString BaseDataAtom<ValueType>::toString() const
{
    return atomToString<ValueType>(*this);
}

typedef struct _guid {
    uint32_t D1;
    uint16_t D2;
    uint16_t D3;
    uchar D4[8];
} GUID;

typedef BaseDataAtom<bool> BooleanDataAtom;

typedef BaseDataAtom<uint8_t> UInt8DataAtom;
typedef BaseDataAtom<int8_t> Int8DataAtom;

typedef BaseDataAtom<uint16_t> UInt16DataAtom;
typedef BaseDataAtom<int16_t> Int16DataAtom;

typedef BaseDataAtom<uint32_t> UInt32DataAtom;
typedef BaseDataAtom<int32_t> Int32DataAtom;

typedef BaseDataAtom<uint64_t> UInt64DataAtom;
typedef BaseDataAtom<int64_t> Int64DataAtom;

typedef BaseDataAtom<float> SingleDataAtom;
typedef BaseDataAtom<double> DoubleDataAtom;

typedef BaseDataAtom<QUuid> GuidDataAtom;
typedef BaseDataAtom<QDateTime> DateTimeDataAtom;


class THREADERSHARED_EXPORT StringDataAtom : public BaseDataAtom<QString> {
public:
    explicit StringDataAtom(const DataAtomDefinition *definition);
    AbstractDataAtom *clone(const DataAtomDefinition *definition) const override;
    uint32_t size() override;
};


class THREADERSHARED_EXPORT ByteArrayDataAtom : public BaseDataAtom<QByteArray>
{
public:
    explicit ByteArrayDataAtom(const DataAtomDefinition* definition);
    ByteArrayDataAtom *clone(const DataAtomDefinition *definition) const override;

    void setCapacity(const uint32_t &capacity) override;
};


template<>
inline QString atomToString<QDateTime>(const DateTimeDataAtom& atom)
{
    return QString("%1: %2")
            .arg(atom.definition()->name())
            .arg(atom.value().toString("HH:mm:ss.zzz"));
}

template<>
inline QString atomToString<QByteArray>(const BaseDataAtom<QByteArray>& atom)
{
    return QString("%1: %2")
            .arg(atom.definition()->name())
            .arg(QString(atom.value().toHex()));

}

template<>
inline QString atomToString<QUuid>(const GuidDataAtom& atom)
{
    return QString("%1: %2")
            .arg(atom.definition()->name())
            .arg(QString(atom.value().toByteArray()));
}

}}
