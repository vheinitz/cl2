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

INCLUDEPATH += . base log os net lis kvs db vision xml

DEPENDPATH += . \
              ai \
              app \
              base \
              db \
              gui \
              kvs \
              lis \
              log \
              os \
              sec \
              vision \
              xml \
              3rdparty/QSMTP \


#output directory
TARGET = cl2
TEMPLATE = vclib 

############ BASE #############
HEADERS  += base/cldef.h 

############ LOG #############
SOURCES += log/clog.cpp 
HEADERS  += log/clog.h 

############ OS ##############
SOURCES += \
	os/fstools.cpp \	
	os/persistence.cpp \
	os/filewatcher.cpp \

HEADERS  += \
	os/fstools.h \
	os/persistence.h \
	os/filewatcher.h \

	
############ VISION ##############
SOURCES += \
	vision/imgproc.cpp \

HEADERS  += \
	vision/imgproc.h \
	
	
############ KVS ##############
SOURCES += kvs/kvstore.cpp 
HEADERS  += kvs/kvstore.h 
	
	
############ XML ##############
SOURCES += xml/xpath.cpp 
HEADERS  += xml/xpath.h 

	
############ DB ##############
SOURCES += db/dbmanager.cpp\
    db/imgdb.cpp \
		   
HEADERS  += db/dbmanager.h\
    db/imgdb.h \
	
	
############ LIS ##############
SOURCES += \
	lis/astmparser.cpp \
    lis/ASTMRecord.cpp \
    lis/DataMapper.cpp \
    lis/LisLink.cpp \
    lis/LisProxy.cpp \

HEADERS  += \
	lis/astmparser.h \
	lis/ASTMRecord.h \
	lis/DataMapper.h \
	lis/LisLink.h \
	lis/LisProxy.h \
	
	
############ SEC ##############
SOURCES += \
    sec/authmanager.cpp\
	sec/cryptmanager.cpp\

HEADERS  += \
    sec/authmanager.h\
	sec/cryptmanager.h\
	
############ GUI ##############
SOURCES += \
    gui/extcombobox.cpp \
    gui/extgview.cpp \
    gui/extimageviewer.cpp \
    gui/imagegroupview.cpp \
    gui/imagelistmodel.cpp \
    gui/imageview.cpp \
    gui/markeritem.cpp \
    gui/objectselector.cpp \
    gui/screenmanager.cpp \
#    gui/valuecontrols.cpp \
			
	
 HEADERS  += \    
    gui/extcombobox.h \
    gui/extgview.h \
    gui/extimageviewer.h \
    gui/imagegroupview.h \
    gui/imagelistmodel.h \
    gui/imageview.h \
    gui/markeritem.h \
    gui/objectselector.h \
    gui/screen.h \
    gui/screenmanager.h \
#    gui/valuecontrols.h \
    gui/screen.h \
    
FORMS  += \   	
    gui/imagegroupview.ui \



######### COM #########################
DEPENDPATH += \
              com/app \
              com/comport \
              com/tcp 
HEADERS += \
           com/app/fileupload.h \
           com/app/httpsrequest.h \
           com/app/httpsupload.h \
           com/app/uploadfile.h \
           com/app/webapiconnector.h \
           com/comport/comport.h \
           com/tcp/tcpserver.h \
           com/comport/comport_win.cpp \
           com/comport/comport_lin.cpp

SOURCES += \
           com/app/fileupload.cpp \
           com/app/httpsrequest.cpp \
           com/app/httpsupload.cpp \
           com/app/uploadfile.cpp \
           com/app/webapiconnector.cpp \
           com/comport/comport.cpp \
           com/comport/comport_lin.cpp \
           com/comport/comport_win.cpp \
           com/tcp/tcpserver.cpp 

####### APP  ######
DEPENDPATH += \
              app \

HEADERS +=  \
           app/languagemanager.h \
           app/qmlclient.h \
           app/siserver.h \
           app/translationmanager.h \

SOURCES +=  \
           app/languagemanager.cpp \
           app/qmlclient.cpp \
           app/siserver.cpp \
           app/translationmanager.cpp \

####### AI  ######
SOURCES += \
	
	
HEADERS  += \
