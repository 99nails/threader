QT -= gui

CONFIG += console
CONFIG -= app_bundle

include(../../../../Common.pri)
include(../../../../Application.pri)

INCLUDEPATH += ../../
INCLUDEPATH += ../../Frames/
INCLUDEPATH += ../../Threads/
INCLUDEPATH += ../../Utils/

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

CONFIG(debug, debug|release) {
    win32: LIBS += -lThreaderd1
    linux-g++: LIBS += -lThreader
} else {
    win32: LIBS += -lThreader1
    linux-g++: LIBS += -lThreader
}
