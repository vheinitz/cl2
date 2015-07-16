# Build this project to generate a shared library (*.dll or *.so).

TARGET = QtWebApp
TEMPLATE = lib
QT -= gui
DEFINES += QTWEBAPPLIB_EXPORT

#include(qtservice/qtservice.pri)
include(logging/logging.pri)
include(httpserver/httpserver.pri)
include(templateengine/templateengine.pri)
include(../../output.pri)
