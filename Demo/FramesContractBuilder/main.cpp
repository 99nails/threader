#include "DataFramesCommon.h"
#include "DataFramesContractBuilder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QTextCodec>

#include <iostream>

const QString PARAM_NAME_SHOW_HELP   = "--help";
const QString PARAM_NAME_SOURCE      = "--source:";
const QString PARAM_NAME_OUTPUT      = "--output:";
const QString PARAM_NAME_PROJECT_NAME= "--project:";
const QString PARAM_NAMESPACE        = "--namespace:";
const QString DEFAULT_NAMESPACE      = "Threader::Frames";

const QString ERROR_PREFIX                = "\r\nОшибка построения контракта: ";
const QString ERROR_SOURCE_FILE_NOT_FOUND = "Файл описаний фреймов [%1] не найден!\r\n";
const QString ERROR_OUTPUT_PATH_NOT_FOUND = "Путь для сохранения контракта [%1] не существует!\r\n";
const QString ERROR_INVALID_PROJECT_NAME  = "Имя проекта [%1] не может быть преобразовано в идентификатор\r\n";

const QString MESSAGE_DEFINITIONS_READ                    = "Прочитано определений фреймов %1 шт.\r\n";
const QString MESSAGE_DEFINITIONS_HEADERS_WRITTEN         = "Заголовки контракта сохранены в файл [%1]\r\n";
const QString MESSAGE_DEFINITIONS_IMPLAMENTATIONS_WRITTEN = "Реализация контракта сохранены в файл [%1]\r\n";

bool parameterShowHelp = false;
QString parameterFile = "./Definitions.xml";
QString parameterOutputPath = "./";
QString parameterProjectName = "";
QString parameterProjectNamespace = DEFAULT_NAMESPACE;

using namespace Threader::Frames;

void writeDosString(const QString &text)
{
    QTextCodec *codec = QTextCodec::codecForName("cp866");
//    QTextCodec *codec = QTextCodec::codecForName("CP-1251");
    if (codec)
    {
        std::cout << text.toStdString();
//        QByteArray dosString = codec->fromUnicode(text.toUtf8());
//        std::cout << dosString.constData();
    }
}

void showHelp() {
    writeDosString("Построение контракта фреймов протокола обмена.\r\n");
    writeDosString("Формат командной строки:\r\n");
    writeDosString("FramesContractBuilder [--help] [--source:<файл>] [--output:<путь>] [--project:<проект>]\r\n");
    writeDosString("\t--help            \t- Вывод формата командной строки.\r\n");
    writeDosString("\t--source:<файл>   \t- Имя исходного файла описаний фреймов. По умолчанию используется файл ./Definitions.xml.\r\n");
    writeDosString("\t--output:<путь>   \t- Установка каталога для создания файлов контракта. По умолчанию используется текущий каталог.\r\n");
    writeDosString("\t--project:<проект>\t- Установка имени проекта для установки префикса при создании файлов контракта. По умолчанию используется FramesDefinitions.\r\n");
    writeDosString("\t--namespace:<namespace>\t- Установка пространства имен. По умолчанию используется SystemT::Shturman::Common::DataFrames\r\n");
}

void parseArguments(QCoreApplication &application)
{
    for (int i = 0; i < application.arguments().count(); i++)
    {
        QString parameter = application.arguments().at(i);

        if (parameter.compare(PARAM_NAME_SHOW_HELP, Qt::CaseInsensitive) == 0)
        {
            parameterShowHelp = true;
        }
        else if (parameter.startsWith(PARAM_NAME_SOURCE, Qt::CaseInsensitive))
        {
            parameterFile = parameter.remove(0, QString(PARAM_NAME_SOURCE).length());
        }
        else if (parameter.startsWith(PARAM_NAME_OUTPUT, Qt::CaseInsensitive))
        {
            parameterOutputPath = parameter.remove(0, QString(PARAM_NAME_OUTPUT).length());
        }
        else if (parameter.startsWith(PARAM_NAME_PROJECT_NAME, Qt::CaseInsensitive))
        {
            parameterProjectName = parameter.remove(0, QString(PARAM_NAME_PROJECT_NAME).length());
        }
        else if (parameter.startsWith(PARAM_NAMESPACE, Qt::CaseInsensitive))
        {
            parameterProjectNamespace = parameter.remove(0, QString(PARAM_NAMESPACE).length());
        }
    }
    Q_UNUSED(application)
}

bool checkArguments() {

    if (!QFile::exists(parameterFile)) {
        QString error = QString(ERROR_SOURCE_FILE_NOT_FOUND).arg(parameterFile);
        writeDosString(ERROR_PREFIX + error);
        return false;
    }

    if (parameterOutputPath.length() > 0 && parameterOutputPath.at(parameterOutputPath.length() - 1) != QDir::separator())
        parameterOutputPath +=QDir::separator();

    if (!QDir::current().exists(parameterOutputPath)) {
        QString error = QString(ERROR_OUTPUT_PATH_NOT_FOUND).arg(parameterOutputPath);
        writeDosString(ERROR_PREFIX + error);
        return false;
    }

    QRegExp r("[a-zA-Z]\\w*");
    if (!parameterProjectName.isEmpty() && !r.exactMatch(parameterProjectName)) {
        QString error = QString(ERROR_INVALID_PROJECT_NAME).arg(parameterProjectName);
        writeDosString(ERROR_PREFIX + error);
        return false;
    }

    return true;
}

DataFramesDefinitions *readDefinitions() {
    DataFramesDefinitions *result = new DataFramesDefinitions();
    result->readFromFile(parameterFile);
    QString message = QString(MESSAGE_DEFINITIONS_READ).arg(result->count());
    writeDosString(message);
    return result;
}

bool writeFile(const QString& text, const QString & fileName)
{
    QFile file(fileName);
    bool result = false;
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered))
    {
        QByteArray data = text.toUtf8();
        qint64 writtenCount = file.write(data);
        result = writtenCount == data.size();
        file.close();
    }
    return result;
}

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QCoreApplication app(argc, argv);

    parseArguments(app);

    if (parameterShowHelp) {
        showHelp();
        return 0;
    }


    writeDosString("Построение контракта фреймов протокола обмена.\r\n");
    writeDosString("Для вывода возможных ключей коммандной строки наберите FramesContractBuilder --help\r\n");

    checkArguments();

    DataFramesDefinitions *definitions = readDefinitions();

    DataFramesContractBuilder builder(definitions,
                                      parameterProjectName,
                                      parameterOutputPath,
                                      parameterProjectNamespace);
    builder.build();


    QString fileName = builder.headerFileName();
    QString text = builder.headerContent();
    if (writeFile(text, fileName))
    {
        QString message = QString(MESSAGE_DEFINITIONS_HEADERS_WRITTEN).arg(fileName);
        writeDosString(message);
    } else
        writeDosString(ERROR_PREFIX + "Ошибка записи файла " + fileName);


    fileName = builder.implementationFileName();
    text = builder.implementationContent();
    if (writeFile(text, fileName))
    {
        QString message = QString(MESSAGE_DEFINITIONS_IMPLAMENTATIONS_WRITTEN).arg(fileName);
        writeDosString(message);
    } else
        writeDosString(ERROR_PREFIX + "Ошибка записи файла " + fileName);


//    QFile helperImplementationFile(builder.helperImplementationName());
//    if (helperImplementationFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered)) {
//        QByteArray data = builder.helperImplementation().toUtf8();
//        qint64 writtenCount = helperImplementationFile.write(data);
//        if (writtenCount == data.size()) {
//            QString message = QString(MESSAGE_DEFINITIONS_HEADERS_WRITTEN).arg(builder.helperImplementationName());
//            writeDosString(message.toStdString());
//        }
//        helperImplementationFile.close();
//    }

    return 0;
}
