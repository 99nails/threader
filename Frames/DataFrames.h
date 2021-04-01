#pragma once

#include "../threader_global.h"

#include "DataFramesCommon.h"
#include "DataAtoms.h"
#include "DataFrameRawData.h"

#include "../Utils/DataStream.h"

#include <QList>
#include <memory>
#include <typeindex>


namespace Threader {

namespace Frames {


using namespace Threader::Utils;



class THREADERSHARED_EXPORT DataFrame
{
public:
    using Ptr = std::shared_ptr<DataFrame>;
public:
    explicit DataFrame(DataFrameDefinition::Ptr definition,
                       AbstractDataAtoms *atoms);
    virtual ~DataFrame();

    DataFrameDefinition::Ptr definition() const;
    QString name() const;
    bool needsTicket() const;
    AbstractDataAtoms *atoms() const;

    virtual bool read(DataStream &stream);
    virtual bool read(DataStream &stream, uint32_t capacity);
    virtual bool read(DataStream *stream);
    virtual bool write(DataStream &stream) const;
    virtual bool write(DataStream *stream) const;

    uint32_t capacity() const;
    void setCapacity(const uint32_t &capacity);
    uint32_t calcCapacityFromAtoms(uint32_t totalSize) const;

    uint32_t size() const;

    uint32_t currentIndex() const;
    void setCurrentIndex(const uint32_t &currentIndex);

    AbstractDataAtom *atomByName(const QString& name) const;

    template <class AtomType>
    AtomType *atomByName(const QString& name) const;

    template<class AtomType>
    AtomType* getAtom(const QString& name) const;

    template <class AtomType>
    AtomType* getAtom() const;

    QString toString() const;

    uint32_t packetId() const;
    void setPacketId(const uint32_t &packetId);

    DataFrameRawData::Ptr dataFrameRawData();

    template <class AtomType, class ValueType>
    bool setAtomValue(const QString &atomName, const ValueType &value)
    {
        AtomType *atom = static_cast<AtomType*>(this->atomByName(atomName));
        if (nullptr != atom)
        {
            atom->setValue(value);
            return true;
        }
        return false;
    }

    template<class ValueType>
    typename std::enable_if<sizeof(BaseDataAtom<ValueType>) != 0, ValueType>::type
    getAtomValue(const QString &atomName, ValueType defaultValue = {})
    {
        if (auto atom = dynamic_cast<BaseDataAtom<ValueType>*>(atomByName(atomName)))
            return atom->value();
        return defaultValue;
    }

    template <class AtomType, class ValueType>
    ValueType getAtomValue(const QString &atomName, ValueType defaultValue = {})
    {
        if (auto atom = dynamic_cast<AtomType*>(atomByName(atomName)))
                return atom->value();
        return defaultValue;
    }

    template <typename AtomType>
    QList<AtomType> getAtomValues(const QString &atomName)
    {
        if (auto atom = dynamic_cast<BaseDataAtom<AtomType>*>(atomByName(atomName)))
                return atom->values();
        return QList<AtomType>{};
    }

protected:
    virtual QString atomsToString() const;

private:
    DataFrameDefinition::Ptr _definition;
    AbstractDataAtoms *_atoms;

    uint32_t _capacity;
    uint32_t _currentIndex;    
    uint32_t _packetId;

};

template<class AtomType>
AtomType *DataFrame::atomByName(const QString& name) const
{
    return dynamic_cast<AtomType*>(atomByName(name));
}

template<class AtomType>
AtomType *DataFrame::getAtom(const QString &name) const
{
    return dynamic_cast<AtomType*>(atomByName(name));
}

template<class AtomType>
AtomType *DataFrame::getAtom() const
{
    AbstractDataAtom *atom;
    for (int i = 0; i < atoms()->count(); i++)
    {
        atom = atoms()->at(i);

        // Дальше начинается маленькая плюсовая магия.
        // Принципиально важно брать тип AtomType и сравнивать его с типом
        // разыменованного atom'а
        // Если сравнивать тип указателя на AtomType с типом atom'а, то чуда не
        // случится, потому что у atom'а всегда будет тип AbstractDataAtom*,
        // не смотря на всю виртуальность.
        if (std::type_index(typeid(AtomType)) == std::type_index(typeid(*atom)))
            return static_cast<AtomType*>(atom);
    }
    return nullptr;
}

using  DataFramesList = QList<DataFrame::Ptr>;


}}
