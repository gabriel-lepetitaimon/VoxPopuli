.PHONY : deamon deamonProject gui installer help all clean
.SILENT: init initDeamon


QMAKE ?= qmake

# PLATEFORM DETECTION
PLATEFORM ?= auto

ifeq ($(PLATEFORM), auto)
	system	:= $(shell uname -s)
	ifeq ($(system), MINGW32_NT-6.1)
		PLATEFORM = windows
	else
	ifeq ($(system), MINGW32_NT-6.2)
		PLATEFORM = windows
    else
		PLATEFORM = linux
	endif
	endif
endif

ifeq ($(PLATEFORM), windows)
	PLATEFORM = win64
endif

#ifeq ($(PLATEFORM), windows)
#		WINARCH ?= $(shell ./qt-arch)
#	ifeq ($(WINARCH), x64)
#		PLATEFORM = win64
#		else
#		PLATEFORM = win32
#		endif
#endif

# PLATEFORM SPECIFIC DEFINITIONS
ifeq ($(PLATEFORM), win32)
	WINOPTS := /maxcpucount:4 /p:Configuration=Release /p:Platform=$(WINARCH)
	QSPEC ?= -spec win32-msvc2013
	BUILD_MAKE = MSBuild
else
ifeq ($(PLATEFORM), win64)
	WINOPTS := /maxcpucount:4 /p:Configuration=Release /p:Platform=$(WINARCH)
	QSPEC ?= -spec win32-msvc2013
	BUILD_MAKE = nmake
else
ifeq ($(PLATEFORM), linux)
	BUILD_MAKE = make
endif
endif
endif

# GLOBAL CONSTANT DEFINITION
BUILDF ?= $(PLATEFORM)
BUILD_PATH = $(notdir $(shell pwd))
ROOT := ..

all : deamon

help :
	@echo "This makefile build VoxPopuli softwares for Linux and Windows platforms."
	@echo "Available targets are : "
	@echo "  deamon"
	@echo "  gui"
	@echo "  installer"
	@echo "$PLATEFORM allows to force a compilation to a specific plateform:"
	@echo "  windows : build for Windows"
	@echo "  win32 : force build for win32"
	@echo "  win64 : force build for win32"
	@echo "  linux : build for Linux"
	@echo "$BUILDF defines the output folder of compilation"
	@echo "$QMAKE and $NMAKE allows to redefine these commands path and options"

#===============================================================
# building Deamon
#===============================================================
DEAMON_PATH := $(VP_PATH)/src/Deamon
B_DEAMON := $(BUILDF)/deamon
deamon: $(B_DEAMON)/VPDeamon.vcxproj
	@echo " -- Building VoxPopuli Deamon -- "
	

deamonProject : initDeamon $(B_DEAMON)/VPDeamon.vcxproj
	cd $(B_DEAMON) && $(QMAKE) $(QSPEC) ../../$(DEAMON_PATH)/VPDeamon.pro
	
initDeamon: init
	cd $(BUILDF) &&\
	[ -d deamon ] || mkdir deamon	
		 
	
$(B_DEAMON)/VPDeamon.vcxproj: $(DEAMON_PATH)/VPDeamon.pro
	make deamonProject

#===============================================================
init:
	[ -d $(BUILDF) ] || mkdir $(BUILDF)

#===============================================================
clean :
	rm -Rf $(BUILDF)

