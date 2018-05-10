QT += core
QT -= gui

CONFIG += c++11
include (libuv.pri)
TARGET = tcpserver1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app
win32{
FLATDIR=D:\masteraplikasi\transferh11nov\flatbuffergit\msvcinstall
INCLUDEPATH += $$FLATDIR/debug/include
LIBS += $$FLATDIR/debug/lib/flatbuffers.lib  
}
linux{
FLATDIR=/home/hv/cpp/magickinstall
INCLUDEPATH += $$FLATDIR/include 
LIBS += $$FLATDIR/lib/libflatbuffers.a

QMAKE_CXXFLAGS += -g -fsanitize=undefined
QMAKE_CFLAGS += -g -fsanitize=undefined
QMAKE_LFLAGS += -fsanitize=undefined
}

SOURCES += main.cpp \ 
    processor.cpp

HEADERS += \
    processor.h
