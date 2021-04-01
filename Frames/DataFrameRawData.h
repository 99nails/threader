#pragma once

#include "../threader_global.h"

#include "DataFramesCommon.h"

#include "../Threads/MessageBase.h"

#include <QByteArray>
#include <QList>
#include <QMutex>

#include <memory>

namespace Threader {

namespace Frames {


using namespace Threader::Threads;


/**
 * @brief DataFrameRawData - Класс хранения данных фрейма в памяти
 * Используется для минимизации потребления памяти
 * Для доступа к типизированным данным фрейма применяется класс DataFrame
 */

class THREADERSHARED_EXPORT DataFrameRawData
{
public:
    using Ptr = std::shared_ptr<DataFrameRawData>;

public:
    /**
     * @brief DataFrameRawData - Конструктор класса из области памяти
     * @param data - Указатель на данные для хранения
     * @param length - Длина блока хранимых данных
     */
    explicit DataFrameRawData(const char *data, const FrameSizeType &length);

    /**
     * @brief ~DataFrameRawData - Деструктор класса
     */
    ~DataFrameRawData();

    /**
     * @brief clone - Клонирование фрейма
     * @return
     */
    Ptr clone();

    /**
     * @brief valid - Признак корректного разбора данных фрейма
     * @return - Признак корректного разбора данных фрейма
     */
    bool isValid() const;

    /**
     * @brief buffer - Получение указателя на буфер хранения данных фрейма
     * @return - Буфер хранения данных фрейма
     */
    char *buffer() const;

    /**
     * @brief name - Получение указателя на имя фрейма
     * @return - Имя фрейма
     */
    char *name() const;

    /**
     * @brief data - Получение указателя на область данных фрейма
     * @return - Область данных фрейма
     */
    char *data() const;

    /**
     * @brief length - Получение размера фрейма
     * @return - Размер фрейма
     */
    FrameSizeType length() const;

    /**
     * @brief streamSize - Получение размера фрейма в потоке
     * @return - Размер фрейма в потоке
     */
    int streamSize() const;

    /**
     * @brief appendByteArray - Добавление данных фрейма в хвост массива
     * @param array - Массив для добавления
     * @param clear - Признак очистки массива
     */
    void appendByteArray(QByteArray &array, bool clear = true);

    /**
     * @brief dataLength - Получение длины области данных фрейма
     * @return - Длина области данных фрейма
     */
    int dataLength() const;

    /**
     * @brief nameString Получение строки имени фрейма в обертке QString
     * @return имя фрейма
     */
    QString toString() const;

    /**
     * @brief priority - Получение приоритета фремйа
     * @return - Приоритет фрейма
     */
    uint8_t priority() const;

    /**
     * @brief packetId - Получение идентификатора связанного пакета
     * @return  - Идентификатор связанного пакета
     */
    int packetId() const;

    /**
     * @brief setPacketId - Установка идентификатора связанного пакета
     * @param packetId - Идентификатор связанного пакета
     */
    void setPacketId(int packetId);

    /**
     * @brief definition - Получение описания фрейма
     * @return - Описание фрейма
     */
    DataFrameDefinition::Ptr definition();

    /**
     * @brief setDefinition - Установка описания фрейма
     * @param definition - Описание фрейма
     */
    void setDefinition(DataFrameDefinition::Ptr definition);

    /**
     * @brief parse - Разбор данных пакета
     * @param source - Область данных пакета
     * @param result - Список фреймов с данными
     */
    static void parse(QByteArray &source, QList<DataFrameRawData::Ptr> &result);

    static bool loadFromFile(const QString &fileName,
                             const DataFramesDefinitions *definitions,
                             QList<DataFrameRawData::Ptr> &list);

    static int referenceCount();

private:
    bool _isValid;
    char *_buffer;
    char *_name;
    char *_data;
    FrameSizeType _length;
    FrameSizeType _dataLength;

    QString _nameString;
    int _packetId;

    DataFrameDefinition::Ptr _definition;
    /**
     * @brief _referenceCount - Счетчик экземпляров
     */
    static QMutex _mutexReferenceCount;
    static int _referenceCount;
    void incrementReferenceCount();
    void decrementReferenceCount();
};

using DataFrameRawDataList = QList<DataFrameRawData::Ptr>;


class MessageDataFramesRaw : public MessageBase
{
public:
    using Ptr = std::shared_ptr<MessageDataFramesRaw>;

public:
    static const QString MESSAGE_NAME;

    explicit MessageDataFramesRaw(const DataFrameRawDataList &frames,
                                  const QString alias = "");

private:
    DataFrameRawDataList _frames;
    QString _alias;
};

}}
