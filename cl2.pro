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
#QT       += script
#QT       += declarative
#QT       += network 
#QT       += xml
#QT       += sql

DEFINES += BUILDING_CLIB_DLL

INCLUDEPATH += log 



#output directory
#DESTDIR = bin
TARGET = cl2
TEMPLATE = lib 


SOURCES += \
    log/clog.cpp \

HEADERS  += \
    log/clog.h \

