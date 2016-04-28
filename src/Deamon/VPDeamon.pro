QT += core network
QT -= gui

CONFIG += c++11

TARGET = VPDeamon
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += VPDeamon.cpp \
    telnet/telnetserver.cpp \
    telnet/telnetsocket.cpp

HEADERS += \
    telnet/telnetserver.h \
    telnet/telnetsocket.h

LIBS += -ltelnet
