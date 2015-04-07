QT += core network
QT -= gui

DEFINES += BUILDING_EIKJU_DLL
INCLUDEPATH += . 

DESTDIR = ../../output
TARGET = eikju
TEMPLATE = vclib

SOURCES += \
    eikjuserver.cpp \
    eikjucommand.cpp \
    ekhttprequest.cpp \
    eikjufunction.cpp
HEADERS += eikjuserver.h \
    eikjucommand.h \
    ekhttprequest.h \
    eikjufunction.h \
	common.h
