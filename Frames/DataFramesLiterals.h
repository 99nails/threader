#pragma once
#include <QString>

namespace {

inline QString MarkUpVar2(const QString & name)
{ return  "{{" + name + "}}"; }
//inline QString MarkUpVar(const QString & name)
//{ return  "{" + name + "}"; }

//inline QString operator ""_cbs(const char *name, size_t)
//{
//    return MarkUpVar2(name);
//}

#define TEMP_VAR_DECL(_TEMP_VARNAME, _TEMP_VALUE) \
const QString CBS_##_TEMP_VARNAME = MarkUpVar2("##_TEMP_VALUE");\
    \
const QString _TEMP_VARNAME = "##_TEMP_VALUE";

const QString TEMPLATE_PROJECT_NAME = QString("ProjectName");
const QString TEMPLATE_PROJECT_NAME_UPPER = QString("ProjectNameUpper");
const QString FRAME_NAME = QString("FrameName");
const QString FRAME_NAME_UPPER = QString("FrameNameUpper");
const QString TEMPLATE_FRAME_NAMESPACE_OPEN = QString("FrameNamespaceOpen");
const QString TEMPLATE_FRAME_NAMESPACE_CLOSE = QString("FrameNamespaceClose");
const QString TEMPLATE_FRAMES_NAMES_CONSTANTS = QString("FramesNamesConstants");

const QString TEMPLATE_ATOM_NAME_UPPER = QString("AtomNameUpper");
const QString TEMPLATE_ATOM_NAME = QString("AtomName");
const QString TEMPLATE_ATOM_TYPE = "AtomType";
const QString CBS_TEMPLATE_ATOM_TYPE = MarkUpVar2(TEMPLATE_ATOM_TYPE);
//TEMP_VAR_DECL(TEMPLATE_ATOM_TYPE, AtomType)

const QString TEMPLATE_ATOMS_NAMES_CONSTANTS = QString("AtomsNamesConstants");

const QString TEMPLATE_ARGUMENT = QString("Argument");
const QString TEMPLATE_TYPE = QString("Type");
const QString TEMPLATE_ARGUMENTS = QString("Arguments");
const QString TEMPLATE_GET_ARGUMENTS = QString("getArguments");
const QString TEMPLATE_SET_ARGUMENTS = QString("setArguments");
const QString TEMPLATE_ASSIGN_ATOMS = QString("AssignAtoms");
const QString TEMPLATE_FUNCTIONS = QString("Functions");

const QString TEMPLATE_FRAME_CONSTANT = QString("extern const char FRAME_NAME_{{" + FRAME_NAME_UPPER +
                                                "}}[];\r\n");

const QString TEMPLATE_ATOM_CONSTANT = QString("extern const char ATOM_NAME_{{" + TEMPLATE_ATOM_NAME_UPPER +
                                               "}}[];\r\n");

const QString TEMPLATE_FUNCTION_ARGUMENT = QString("\r\n\t\tconst {{" + TEMPLATE_TYPE + "}} &{{" + TEMPLATE_ARGUMENT + "}}");

const QString TEMPLATE_FUNCTION_DEFINITION = QString("\tDataFrame::Ptr build{{" + FRAME_NAME + "}}Frame({{" + TEMPLATE_ARGUMENTS + "}});\r\n\r\n");

const QString TEMPLATE_HEADER_FILE = QString(
            "// Этот файл создан автоматически.\r\n\r\n"
            "#pragma once\r\n"
            "#include \"../../Threader/Frames/DataFramesFactory.h\"\r\n"
            "#include <math.h>\r\n\r\n"
            "namespace {{" + TEMPLATE_PROJECT_NAME +  "}}DataFramesFactory"
            "{\r\n"
            "{{" + TEMPLATE_FRAMES_NAMES_CONSTANTS + "}}\r\n"
            "{{" + TEMPLATE_ATOMS_NAMES_CONSTANTS +  "}}\r\n"
            "}\r\n\r\n"
            "{{" + TEMPLATE_FRAME_NAMESPACE_OPEN + "}}\r\n"
            "class {{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactoryImpl : public DataFramesFactory\r\n"
            "{\r\n"
            "public:\r\n"
            "\texplicit {{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactoryImpl(DataFramesDefinitions *definitions = nullptr);\r\n\r\n"
            "\t~{{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactoryImpl() override;\r\n\r\n"
            "\tstatic {{" + TEMPLATE_PROJECT_NAME + "}}DataFramesFactoryImpl *instance();\r\n"
            "{{" + TEMPLATE_FUNCTIONS +
            "}}"
            "private:\r\n"
            "\tstatic {{" + TEMPLATE_PROJECT_NAME + "}}DataFramesFactoryImpl *_instance;\r\n"
            "};\r\n"
            "\r\n{{" + TEMPLATE_FRAME_NAMESPACE_CLOSE + "}}\r\n"
            );

const QString TEMPLATE_FRAME_CONSTANT_IMPLEMENTATION =
        QString("const char FRAME_NAME_{{" + FRAME_NAME_UPPER +
                "}}[] = \"{{" + FRAME_NAME_UPPER +"}}\";\r\n");

const QString TEMPLATE_ATOM_CONSTANT_IMPLEMENTATION =
        QString("const char ATOM_NAME_{{" + TEMPLATE_ATOM_NAME_UPPER +
                "}}[] = \"{{ " + TEMPLATE_ATOM_NAME +
                "}}\";\r\n");

const QString TEMPLATE_ASSIGN_ATOM = QString(
            "\tsetAtomValue<" + CBS_TEMPLATE_ATOM_TYPE + ">(ATOM_NAME_{{" + TEMPLATE_ATOM_NAME_UPPER + "}}, frame, {{" + TEMPLATE_ATOM_NAME + "}});\r\n");

const QString TEMPLATE_FUNCTION_IMPLEMENTATION = QString(
            "DataFrame::Ptr {{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactoryImpl::build{{" + FRAME_NAME + "}}Frame({{" + TEMPLATE_ARGUMENTS + "}})\r\n{\r\n"+
            "\tDataFrame::Ptr frame(buildFrame(FRAME_NAME_{{" + FRAME_NAME_UPPER + "}}));\r\n" +
            "\tif (!frame)\r\n\t\treturn frame;\r\n" +
            "{{" + TEMPLATE_ASSIGN_ATOMS + "}}" +
            "\treturn frame;\r\n" +
            "}\r\n\r\n");

const QString TEMPLATE_IMPLEMENTATION_FILE = QString(
            "// Этот файл создан автоматически.\r\n\r\n"
            "#include \"{{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactory.h\"\r\n\r\n"
            "namespace " + "{{" + TEMPLATE_PROJECT_NAME +  "}}DataFramesFactory" +
            "{\r\n"
            "// Константы имен фреймов {{" + TEMPLATE_PROJECT_NAME + "}}\r\n"
            "{{" + TEMPLATE_FRAMES_NAMES_CONSTANTS +
            "}}\r\n"
            "// Константы имен атомов {{" + TEMPLATE_PROJECT_NAME + "}}\r\n"
            "{{" + TEMPLATE_ATOMS_NAMES_CONSTANTS +
            "}}\r\n"
            "}\r\n"
             "{{" + TEMPLATE_FRAME_NAMESPACE_OPEN + "}}\r\n"
            "{{" + TEMPLATE_PROJECT_NAME + "}}DataFramesFactoryImpl *{{" + TEMPLATE_PROJECT_NAME + "}}DataFramesFactoryImpl::_instance = nullptr;\r\n\r\n"
            "{{" + TEMPLATE_PROJECT_NAME + "}}DataFramesFactoryImpl::{{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactoryImpl(DataFramesDefinitions *definitions)\r\n    : DataFramesFactory(definitions)\r\n{\r\n\t_instance = this;\r\n}\r\n\r\n"

            "{{" + TEMPLATE_PROJECT_NAME + "}}DataFramesFactoryImpl::~{{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactoryImpl()\r\n{\r\n\t_instance = nullptr;\r\n}\r\n\r\n"

            "{{" + TEMPLATE_PROJECT_NAME + "}}DataFramesFactoryImpl *{{" + TEMPLATE_PROJECT_NAME +
            "}}DataFramesFactoryImpl::instance()\r\n"
            "{\r\n\treturn _instance;\r\n}\r\n\r\n"
            "{{" + TEMPLATE_FUNCTIONS + "}}\r\n"
            "\r\n{{" + TEMPLATE_FRAME_NAMESPACE_CLOSE + "}}\r\n"
            );

const QString DataFramesUsing = "using namespace Threader::Frames;\r\n";
const QString NS_Format = "namespace %1 {\r\n\r\n";
const QString ConstNSUsing = "using namespace ";
}
