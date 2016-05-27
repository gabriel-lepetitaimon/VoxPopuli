### CONFIG ###
QT += core network
QT -= gui

CONFIG += c++11, console
CONFIG -= app_bundle
TEMPLATE = app

TARGET = VPDeamon

### LOCATIONS ###
VPD = $$PWD
SRC = $$VPD/..
ROOT = $$SRC/..
LIB = $$ROOT/lib




### SOURCES ###
SOURCES += VPDeamon.cpp \
    telnet/telnetserver.cpp \
    telnet/telnetsocket.cpp \
    xbee/xbeeinterface.cpp \
    model/model.cpp \
    model/networkmodel.cpp\
    $$SRC/lib/misc.cpp \
    xbee/xbeeremote.cpp \
    model/eventmodel.cpp \
    model/virtualnetwork.cpp \
    model/eventtrigger.cpp

HEADERS += \
    telnet/telnetserver.h \
    telnet/telnetsocket.h \
    xbee/xbeeinterface.h \
    model/model.h \
    model/networkmodel.h\
    $$SRC/lib/singleton.h\
    $$SRC/lib/misc.h \
    xbee/xbeeremote.h \
    model/eventmodel.h \
    model/virtualnetwork.h \
    model/eventtrigger.h


    
### LIBRAIRIES ###
INCLUDEPATH += $$SRC/lib
LIBS += -ltelnet -lrtmidi
include($$LIB/XBee/XBee.pri)

RESOURCES += \
    ressources.qrc
