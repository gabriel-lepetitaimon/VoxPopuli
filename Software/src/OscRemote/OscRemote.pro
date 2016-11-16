TEMPLATE = app

QT += qml quick

CONFIG += c++11

##############################
###      LOCATIONS         ###
##############################
VPD = $$PWD
SRC = $$VPD/..
ROOT = $$SRC/..
LIB = $$ROOT/lib


##############################
###        SOURCES         ###
##############################
SOURCES += main.cpp \
    oscremote.cpp
HEADERS += \
    oscremote.h


RESOURCES += \
    rsrc.qrc

##############################
###      LIBRAIRIES        ###
##############################
#OSC
include($$LIB/oscpack/osc.pri)
