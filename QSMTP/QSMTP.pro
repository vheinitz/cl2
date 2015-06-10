
QT       += core gui network

TEMPLATE = vclib
TARGET = QSMTP
DEFINES += BUILDING_QSMTP_DLL


SOURCES += \
    ./emailaddress.cpp \
    ./mimeattachment.cpp \
    ./mimefile.cpp \
    ./mimehtml.cpp \
    ./mimeinlinefile.cpp \
    ./mimemessage.cpp \
    ./mimepart.cpp \
    ./mimetext.cpp \
    ./smtpclient.cpp \
    ./quotedprintable.cpp \
    ./mimemultipart.cpp \
    ./mimecontentformatter.cpp

HEADERS  += \
    ./emailaddress.h \
    ./mimeattachment.h \
    ./mimefile.h \
    ./mimehtml.h \
    ./mimeinlinefile.h \
    ./mimemessage.h \
    ./mimepart.h \
    ./mimetext.h \
    ./smtpclient.h \
    ./SmtpMime \
    ./quotedprintable.h \
    ./mimemultipart.h \
    ./mimecontentformatter.h \
	./common.h

include(../output.pri)