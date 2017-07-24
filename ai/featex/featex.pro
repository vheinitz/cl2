#-------------------------------------------------
#
# Feature extractor
# VH, 2013-10-16
#-------------------------------------------------

QT       += core gui sql
QT       += script

TEMPLATE = vclib
DESTDIR = ../../output
TARGET = featex
DEFINES += BUILDING_FEATEX_DLL

DEPENDPATH += .
INCLUDEPATH += .\
  ./features \
  ../corelib \
  ../cells \
  ../ipscript
		
LIBS +=	"../../output/corelib.lib" \
		"../../output/cells.lib"	\
		"../../output/ipscript.lib"	

SOURCES += \
	./featex.cpp\
	./features/feature.cpp\
	./features/testing.cpp\
	./features/eccentricity.cpp\
	./features/circularity.cpp\
	./features/convexity.cpp\
	./features/scriptfeature.cpp\
	
	
HEADERS  += \
	./featex.h\
	./features/feature.h\
	./features/testing.h\
	./features/eccentricity.h\
	./features/circularity.h\
	./features/convexity.h\
	./features/scriptfeature.h\

RESOURCES += \
    res.qrc

INCLUDEPATH += . \
		"../../Libraries/3rdparty/opencv/include/opencv" \
		"../../Libraries/3rdparty/opencv/include/" \

win32 {
	CONFIG(debug, debug|release) {
		LIBS += \
		../../Libraries/3rdparty/opencv/lib/opencv_contrib240d.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_core240d.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_features2d240d.lib \
#		../../Libraries/3rdparty/opencv/lib/opencv_flann240d.lib \
#		../../Libraries/3rdparty/opencv/lib/opencv_nonfree240d.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_highgui240d.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_imgproc240d.lib \
#		../../Libraries/3rdparty/opencv/lib/opencv_legacy240d.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_ml240d.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_objdetect240d.lib 
	} else {
		LIBS += \
		../../Libraries/3rdparty/opencv/lib/opencv_contrib240.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_core240.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_features2d240.lib \
#		../../Libraries/3rdparty/opencv/lib/opencv_flann240.lib \
#		../../Libraries/3rdparty/opencv/lib/opencv_nonfree240.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_highgui240.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_imgproc240.lib \
#		../../Libraries/3rdparty/opencv/lib/opencv_legacy240.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_ml240.lib \
		../../Libraries/3rdparty/opencv/lib/opencv_objdetect240.lib 
	}
}

