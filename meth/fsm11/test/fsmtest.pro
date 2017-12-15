#-------------------------------------------------
#
# Project created by QtCreator 2016-09-08T10:00:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fsmtest
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../fsm.cpp

HEADERS  += mainwindow.h \
    ../fsm.h

FORMS    += mainwindow.ui
