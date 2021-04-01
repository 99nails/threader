include(../../Common.pri)
include(../../Library.pri)

include(ThirdParty/qtservice/qtservice.pri)

QT -= gui

TARGET = $$qtLibraryTarget(Threader$${LIB_SUFFIX})
TEMPLATE = lib

DEFINES += THREADER_LIBRARY

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        Frames/DataAtoms.cpp \
        Frames/DataFrameRawData.cpp \
        Frames/DataFrames.cpp \
        Frames/DataFramesCommon.cpp \
        Frames/DataFramesContractBuilder.cpp \
        Frames/DataFramesFactory.cpp \
        Frames/DataFramesMask.cpp \
        Frames/DataFramesPackets.cpp \
        Frames/MessageDataFrames.cpp \
        Frames/MessageQueue.cpp \
        Frames/QueueDataFrames.cpp \
        ThirdParty/mustache/mustache.cpp \
        Threads/HandlerBase.cpp \
        Threads/HandlerSerialPort.cpp \
        Threads/HandlerTcpSocket.cpp \
        Threads/HandlerUdpSocket.cpp \
        Threads/ListThreads.cpp \
        Threads/MessageBase.cpp \
        Threads/MessageBinary.cpp \
        Threads/MessageLog.cpp \
        Threads/MessageObject.cpp \
        Threads/MessageString.cpp \
        Threads/MessageThread.cpp \
        Threads/MessageTimer.cpp \
        Threads/MessageWriteToFile.cpp \
        Threads/PacketFactoryAsciiLines.cpp \
        Threads/PacketFactoryBase.cpp \
        Threads/PollerListenSocket.cpp \
        Threads/PollerThread.cpp \
        Threads/QueueMessages.cpp \
        Threads/SettingsBase.cpp \
        Threads/ThreadBase.cpp \
        Threads/ThreadDataFrames.cpp \
        Threads/ThreadHandler.cpp \
        Threads/ThreadListenSocket.cpp \
        Threads/ThreadLogs.cpp \
        Threads/ThreadMainDaemon.cpp \
        Threads/ThreadTimer.cpp \
        Threads/WriterLogs.cpp \
        Utils/CrcUtils.cpp \
        Utils/DataStream.cpp \
        Utils/DateUtils.cpp \
        Utils/IpMask.cpp \
        Utils/SerialUtils.cpp \
        Utils/SocketUtils.cpp \
        Utils/TrafficCounter.cpp

HEADERS += \
    Frames/DataAtoms.h \
    Frames/DataFrameRawData.h \
    Frames/DataFrames.h \
    Frames/DataFramesCommon.h \
    Frames/DataFramesContractBuilder.h \
    Frames/DataFramesFactory.h \
    Frames/DataFramesLiterals.h \
    Frames/DataFramesMask.h \
    Frames/DataFramesPackets.h \
    Frames/MessageDataFrames.h \
    Frames/MessageQueue.h \
    Frames/QueueDataFrames.h \
    ThirdParty/mustache/mustache.h \
    Threads/DaemonApplication.h \
    Threads/HandlerSerialPort.h \
    Threads/HandlerUdpSocket.h \
    Threads/MessageThread.h \
    Threads/PacketFactoryAsciiLines.h \
    Threads/SettingsBase.h \
    Threads/ThreadDataFrames.h \
    Utils/IpMask.h \
    threader_global.h \
    Threads/ThreadsCommon.h \
    Threads/HandlerBase.h \
    Threads/HandlerTcpSocket.h \
    Threads/ListThreads.h \
    Threads/LogMessagesTemplates.h \
    Threads/MessageBase.h \
    Threads/MessageBinary.h \
    Threads/MessageLog.h \
    Threads/MessageObject.h \
    Threads/MessageString.h \
    Threads/MessageTimer.h \
    Threads/MessageWriteToFile.h \
    Threads/PacketFactoryBase.h \
    Threads/PollerListenSocket.h \
    Threads/PollerThread.h \
    Threads/QueueMessages.h \
    Threads/ThreadBase.h \
    Threads/ThreadHandler.h \
    Threads/ThreadListenSocket.h \
    Threads/ThreadLogs.h \
    Threads/ThreadMainDaemon.h \
    Threads/ThreadTimer.h \
    Threads/WriterLogs.h \
    Utils/CrcUtils.h \
    Utils/DataStream.h \
    Utils/DateUtils.h \
    Utils/SerialUtils.h \
    Utils/SocketUtils.h \
    Utils/TrafficCounter.h

unix:SOURCES += Threads/PollingsLinux.cpp Utils/PosixHandler.cpp
win32:SOURCES += Threads/PollingsWindows.cpp
unix:HEADERS += Threads/PollingsLinux.h Utils/PosixHandler.h
win32:HEADERS += Threads/PollingsWindows.h

win32: LIBS += -lws2_32 -lpsapi
