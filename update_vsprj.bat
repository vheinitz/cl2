@REM creates VS project file
@REM Valentin Heinitz, 2015-04-15
set PATH=C:\Qt\4.8.4\bin\


qmake -t vclib
cd QSMTP
qmake -t vclib
cd ..
cd 3rdparty\QtWebApp
qmake -t vclib
cd ..\..
