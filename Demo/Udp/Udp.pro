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
    MainUdp.cpp \
    ThreadMainUdp.cpp \
    ThreadUdpSocket.cpp

HEADERS += \
    ThreadMainUdp.h \
    ThreadUdpSocket.h

CONFIG(debug, debug|release) {
    win32: LIBS += -lThreaderd1
    linux-g++: LIBS += -lThreader
} else {
    win32: LIBS += -lThreader1
    linux-g++: LIBS += -lThreader
}
