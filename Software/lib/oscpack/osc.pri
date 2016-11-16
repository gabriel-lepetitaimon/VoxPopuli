OSCLIB = $$LIB/oscpack

SOURCES += $$files($$OSCLIB/osc/*.cpp)
SOURCES += $$files($$OSCLIB/ip/*.cpp)

HEADERS += $$files($$OSCLIB/osc/*.h)
HEADERS += $$files($$OSCLIB/ip/*.h)

unix:!macx:!ios:!android {
    DEFINES += OSC_HOST_LITTLE_ENDIAN __LINUX__
    SOURCES += $$files($$OSCLIB/ip/posix/*.cpp)
}
win32{
    DEFINES += OSC_HOST_LITTLE_ENDIAN __WIN32__ WINDOWS
    SOURCES += $$files($$OSCLIB/ip/win32/*.cpp)
	LIBS += -lws2_32 -lwinmm
}

SOURCES += $$files($$OSCLIB/qt/*.cpp)
HEADERS += $$files($$OSCLIB/qt/*.h)

INCLUDEPATH += $$OSCLIB $$OSCLIB/qt
