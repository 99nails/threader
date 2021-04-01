#pragma once

#include "../threader_global.h"

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QUuid>
#include <QMap>
#include <QList>
#include <QHash>
#include <QTime>
#include <QVariant>
#include <QDateTime>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QtDebug>

//#include <functional>
#include <memory>


namespace Threader {

namespace Frames {


const QString TYPE_NAME_BOOLEAN    ("Boolean");
const QString TYPE_NAME_INT8       ("Int8");
const QString TYPE_NAME_UINT8      ("UInt8");
const QString TYPE_NAME_INT16      ("Int16");
const QString TYPE_NAME_UINT16     ("UInt16");
const QString TYPE_NAME_INT32      ("Int32");
const QString TYPE_NAME_UINT32     ("UInt32");
const QString TYPE_NAME_INT64      ("Int64");
const QString TYPE_NAME_UINT64     ("UInt64");
const QString TYPE_NAME_SINGLE     ("Single");
const QString TYPE_NAME_DOUBLE     ("Double");
const QString TYPE_NAME_DATETIME   ("DateTime");
const QString TYPE_NAME_STRING     ("String");
const QString TYPE_NAME_GUID       ("Guid");
const QString TYPE_NAME_BYTEARRAY  ("ByteArray");

using FrameSizeType = uint32_t;

/**
 * @brief DataAtomDefinition - Класс хранения описания атома
 */

class THREADERSHARED_EXPORT DataAtomDefinition
{
public:
    using  Ptr = std::shared_ptr<DataAtomDefinition>;

public:
    explicit DataAtomDefinition(const QString &name,
                                const QString &atomType,
                                const bool &isScalar,
                                const QString &defaultValue);
    ~DataAtomDefinition(){}

    QString name() const;
    QString atomType() const;
    bool isScalar() const;

private:
    QString _name;
    QString _atomType;
    bool _isScalar;
    QString _defaultValue;
};

using DataAtomsDefinitions = QList<DataAtomDefinition::Ptr>;

/**
 * @brief DataFrameDefinition класс хранения описания фрейма
 */
class THREADERSHARED_EXPORT DataFrameDefinition
{
public:
    using  Ptr = std::shared_ptr<DataFrameDefinition>;

public:
    explicit DataFrameDefinition(
            const QString &name,
            const bool isArray,
            const unsigned int capacity,
            const bool needsTicket,
            const unsigned char priority,
            const DataAtomsDefinitions& atoms = DataAtomsDefinitions());
    virtual ~DataFrameDefinition() = default;

    void clear();

    const QString name() const;
    bool isArray() const;
    unsigned int capacity() const;
    bool needsTicket() const;
    unsigned char priority() const;

    bool hasAddressDirection() const;
    void setHasAddressDirection(bool hasAddressDirection);

    void appendAtom(const DataAtomDefinition::Ptr& atom);

    const DataAtomsDefinitions &atoms() const;
    int indexOfAtomByName(const QString &name);

    virtual QString toString() const;

private:
    QString _name;
    bool _isArray;
    unsigned int _capacity;
    bool _needsTicket;
    unsigned char _priority;
    bool _hasAddressDirection;
    DataAtomsDefinitions _atoms;
};

using DataFramesDefinitionsList = QList<DataFrameDefinition*>;


/**
 * @brief DataFramesDefinitions класс хранилища описаний фреймов
 */
class THREADERSHARED_EXPORT DataFramesDefinitions
{
public:
    /**
     * @brief KeyExtractor - функтор для того, чтоб из описания фрейма доставать
     * ключ, по которому фрейм может быть найден в списке. Принимает на вход
     * константный указатель на DataFrameDefinition, отдаёт QString с ключом.
     */
//    typedef std::function<QString(const DataFrameDefinition::Ptr&)> KeyExtractor;

    using  Ptr = std::shared_ptr<DataFramesDefinitions>;

public:
    explicit DataFramesDefinitions();
    virtual ~DataFramesDefinitions();

    void clear();

    void readFromReader(QXmlStreamReader &xml);

    bool readFromFile(const QString &fileName,
                     const bool &clearList = true);

    bool readFromResource(const QString &resourceName,
                          const bool &clearList = true);


    QList<QString> keys(bool sorted = false);

    int count();

    /**
     * @return описание фрейма соответствующее ключу @a key или nullptr, если
     * данный ключ хранилищу не известен
     */
    DataFrameDefinition::Ptr definitionByKey(const QString &key) const;

    QString fileName() const;

    QString errorString() const;

protected:
    virtual void parseDataFramesAttributes(QXmlStreamAttributes attributes);
    virtual DataFrameDefinition::Ptr parseDataFrameAttributes(QXmlStreamAttributes attributes);
    virtual DataAtomDefinition::Ptr parseDataAtomAttributes(QXmlStreamAttributes attributes);
    virtual void appendDefinition(const QString &name,
				  DataFrameDefinition::Ptr definition);

private:
    QString _version;
    QHash<QString, DataFrameDefinition::Ptr> _framesHash;
    QString _fileName;
    QString _error;
};

class THREADERSHARED_EXPORT FrameDefinitionByName
{
public:
    explicit FrameDefinitionByName(const QString& name) : _name(name){}

    bool operator()(const DataFrameDefinition::Ptr& dataFrame)
    {
        return dataFrame->name().compare(_name) == 0;
    }

private:
    QString _name;
};

}}
