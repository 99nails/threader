QT -= gui
QT += core network

CONFIG += console
CONFIG -= app_bundle

include(../../../../Common.pri)
include(../../../../Application.pri)
include($$PWD/../../ThirdParty/qtservice/qtservice.pri)

INCLUDEPATH += ../../
INCLUDEPATH += ../../Threads/
INCLUDEPATH += ../../Utils/
INCLUDEPATH += ../../ThirdParty/qtservice/

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        ../Common/PacketFactories/PacketFactorySimple.cpp \
        ThreadMainSimpleServer.cpp \
        ThreadSimpleServerListenSocket.cpp \
        ThreadSimpleServerSocket.cpp \
        main.cpp

HEADERS += \
    ../Common/Logs/LogMessagesTemplatesDemo.h \
    ../Common/PacketFactories/PacketFactorySimple.h \
    ThreadMainSimpleServer.h \
    ThreadSimpleServerListenSocket.h \
    ThreadSimpleServerSocket.h

CONFIG(debug, debug|release) {
    win32: LIBS += -lThreaderd1
    linux-g++: LIBS += -lThreader
} else {
    win32: LIBS += -lThreader1
    linux-g++: LIBS += -lThreader
}
