#pragma once

#include "../threader_global.h"

#include "DataFramesCommon.h"
#include "DataFrameRawData.h"
#include "DataAtoms.h"
#include "DataFrames.h"

#include "../Utils/DataStream.h"

namespace Threader {

namespace Frames {


using namespace Threader::Utils;


using AtomsRegistry = QHash<QString, AbstractDataAtom*>;


class THREADERSHARED_EXPORT DataFramesFactory
{
public:
    /**
     * @brief Создание пустой фабрики.
     * @note Для инициализации используйте вызов init()
     */
    explicit DataFramesFactory(DataFramesDefinitions *definitions = nullptr);

    virtual ~DataFramesFactory();

    /**
     * @brief Инициализация фабрики.
     * @param definitionsFileName - путь к файлу описания фреймов
     */
    virtual void init(const QString &definitionsFileName);
    virtual void init(DataFramesDefinitions *definitions);

    bool readFromXml(const QString &definitionsFileName);
    bool readFromResource(const QString &resouceName,
                          const bool &clear = true);

    DataFrame::Ptr readFrame(const DataFrameRawData::Ptr& frameRawData);
    virtual DataFramesList readFrames(DataStream &stream);
    DataFramesList readFrames(QByteArray *data);

    DataFrame::Ptr buildFrame(const QString &key) const;
    DataFrame::Ptr buildFrame(DataFrameDefinition::Ptr definition) const;

    QString definitionsFileName() const;
    DataFramesDefinitions *definitions() const;

    bool isFramesNeedTicket(QList<DataFrameRawData::Ptr> &list);

    virtual DataFrame::Ptr readFrame(DataStream &stream, bool readWithoutLength = false) const;
    virtual DataFrame::Ptr readFrame(QByteArray &data, bool readWithoutLength = false) const;
protected:
    virtual void initializeAtomsStorage();
    virtual void releaseAtomsStorage();

    void registerAtom(const QString& type, AbstractDataAtom* instance);

    AbstractDataAtom* atomByDefinition(const DataAtomDefinition::Ptr& definition,
                                       AbstractDataAtom* defaultValue = nullptr) const;

    virtual DataFramesDefinitions* createDefinitionStorage();

    virtual void releaseDefinitionsStorage();

    template <class AtomType, class ValueType>
    bool setAtomValue(const QString &atomName, DataFrame::Ptr frame, const ValueType &value)
    {
        AtomType *atom = static_cast<AtomType*>(frame->atomByName(atomName));
        if (nullptr != atom)
        {
            atom->setValue(value);
            return true;
        }
        return false;
    }
private:
    AbstractDataAtom *buildAtom();

    QString _definitionsFileName;
    DataFramesDefinitions *_definitions;
    bool _isOwningDefinitions;
    AtomsRegistry _registry;
};

}}
