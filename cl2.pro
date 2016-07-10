# * ===========================================================================
# * Copyright 2015: Valentin Heinitz, vheinitz@googlemail.com
# * CL2 - core lib version 2
# * Author: Valentin Heinitz, 2015-04-28
# * License: MIT License
# *
# * D E S C R I P T I O N
# * CL2 is the collection of classes
# ========================================================================== 
 
QT       += core
#QT       += widgets
#QT       += script
#QT       += declarative
QT       += network 
QT       += xml
QT       += gui
QT       += sql

DEFINES += BUILDING_CLIB_DLL

INCLUDEPATH += . log os net lis kvs db vision xml



#output directory
TARGET = cl2
TEMPLATE = lib 

############ LOG #############
SOURCES += log/clog.cpp 
HEADERS  += log/clog.h 

############ OS ##############
SOURCES += \
	os/siserver.cpp \
	os/fstools.cpp \	
	os/persistence.cpp \
	os/filewatcher.cpp \
	xml/xpath.cpp \
	net/tcpserver.cpp \
	lis/astmparser.cpp \
	kvs/kvstore.cpp \
	vision/imgproc.cpp \
	vision/imgdb.cpp \
	db/dbmanager.cpp\

HEADERS  += \
	os/siserver.h \
	os/fstools.h \
	os/persistence.h \
	os/filewatcher.h \
	xml/xpath.h \
	net/tcpserver.h \
	lis/astmparser.h \
	kvs/kvstore.h \
	vision/imgproc.h \
	vision/imgdb.h \
	db/dbmanager.h\

include(output.pri)
