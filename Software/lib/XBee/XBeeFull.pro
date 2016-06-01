### LOCATIONS ###
XBee = $$PWD

### BUILD OPTIONS ###
DEFINES +=XBEE_DEVICE_ENABLE_ATMODE

### SOURCES ###
INCLUDEPATH += $$XBee/include
#HEADERS += $$files($$XBee/include/*.h, true)

SOURCES += $$files($$XBee/src/util/*.c)
SOURCES += $$files($$XBee/src/xbee/*.c)
SOURCES += $$files($$XBee/src/wpan/*.c)
SOURCES += $$files($$XBee/src/zigbee/*.c)


### Platform Specific ###
unix {
SOURCES += $$files($$XBee/src/posix/*.c)
DEFINES += POSIX
}

#win32 {
#SOURCES += $$files($$XBee/src/win32/*.c)
#DEFINES += WIN32
#}

