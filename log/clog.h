#ifndef LOGSUPPORT_H
#define LOGSUPPORT_H

#include <QObject>
#include <QFile>
#include <QStringList>
#include <QSharedPointer>
#include <QString>
#include <QDebug>
#include <QtGlobal>
#include <stdio.h>
#include <stdlib.h>

#include <QtGlobal>
#include <QtMessageHandler>

#ifndef WINDOWS_LEAN_AND_MEAN
  #define WINDOWS_LEAN_AND_MEAN
  #include <windows.h>
#endif
/*! CoreLib2 Logger class

Currently collects all output from Qt-Log mechanisms (qDebug, etc) and saves it in a file
prepending additional information.
In order to be feature-compatible, log-macros should be used.
Ideas for Future:
  -Log-Rotation
  -Logging over network
  -Stable logging on crashes
  -multithreading
*/

#define C_LL_NOLOG 0 
#define C_LL_TRACE 1
#define C_LL_WARNING 2
#define C_LL_ERROR 4
#define C_LL_PERF  8


#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

class CLIB_EXPORT LoggChannel
{
protected:
	QString _chName;
public:
	virtual void writeLog( QString msg )=0;
	QString chName() const { return _chName ; }
	virtual ~LoggChannel(){}
};

typedef QSharedPointer<LoggChannel> PLoggChannel;

class CLIB_EXPORT FileLoggChannel: public LoggChannel
{	
public:
	FileLoggChannel (QString fileName);
	void writeLog( QString msg );
	virtual ~FileLoggChannel(){}
};

class CLIB_EXPORT CLogger
{
    static QtMessageHandler _messageHandlerInstaller;
    int _logLevel;

    CLogger();
    ~CLogger();
    QList<PLoggChannel> _logChannels;
    
public:
    void messageOutput(QtMsgType type, const char *msg);
    /*! Singleton logger class. Initializes and opens file on first call
    */
    static CLogger & instance();

    void initLogging( QStringList args );
	void stopLogging();

    /*! Enable/disable tracing at run-time. Traces are usually developer messages.
        This is rather a developer function. 
        Enabling traces at compile time still requires performance, even if writing traces to
        file is disabled.
    */
	void setLogLevel( int ll );
	int logLevel()const{return _logLevel;}

    bool addLogChannel( LoggChannel * ch );
    bool removeLogChannel( QString ch );

    /*! Catches all messages sent using Qt-Mechanisms as c-string        
    */
    static void pdMessageOutput(QtMsgType type, const char *msg);
};

class CLIB_EXPORT _Log_ScopeTracer_
{	
    QString _msg;
	static QString ScopeIndent;
public:
    _Log_ScopeTracer_(QString msg):_msg(msg){
		if( CLogger::instance().logLevel() & C_LL_TRACE )
		{
			ScopeIndent += "   ";
			qDebug() <<ScopeIndent<<" SCOPE>>>"<< _msg;			
		}
    }
    ~_Log_ScopeTracer_(){
		if( CLogger::instance().logLevel() & C_LL_TRACE )
		{
			qDebug() << ScopeIndent<<" SCOPE<<<"<< _msg;
			ScopeIndent.remove(0,3);
		}
    }
};

#define C_DISABLE_LOGGING
#ifndef C_DISABLE_LOGGING // disabling in pre-processor. mainly for checking, if logging is a really performance issue
    #define C_LOG( format, ... )     if( CLogger::instance().logLevel() & C_LL_ERROR ){qDebug(     "LOG:    \t%s:%d\t%s\t"format,__FILE__,__LINE__,__FUNCTION__, __VA_ARGS__ );}
    #define C_TRACE( format, ... )   if( CLogger::instance().logLevel() & C_LL_TRACE ){qDebug(     "TRACE:  \t%s:%d\t%s\t"format,__FILE__,__LINE__,__FUNCTION__, __VA_ARGS__ );}
    #define C_SCOPE( msg )           _Log_ScopeTracer_ scopeTracer##__LINE__ (QString("SCOPE: %1:%2:%3:\t%4").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__).arg(msg) );
    #define C_TODO( msg )            if( CLogger::instance().logLevel() & C_LL_WARNING ){qWarning( "TODO:   \t%s:%d\t%s\t",__FILE__, __LINE__, __FUNCTION__, msg );}
    #define C_WARNING( format, ... ) if( CLogger::instance().logLevel() & C_LL_WARNING ){qWarning( "WARNING:\t%s:%d\t"format,__FILE__,__LINE__, __VA_ARGS__ );}
    #define C_ERROR( format, ... )   if( CLogger::instance().logLevel() & C_LL_ERROR ){qCritical(  "ERROR:  \t%s:%d\t%s\t"format,__FILE__,__LINE__,__FUNCTION__, __VA_ARGS__ );}
    #define C_PERF( tag, diff )      if( CLogger::instance().logLevel() & C_LL_PERF ){qDebug(      "C_PERF:\t%s\t%d",tag, diff );}
#else
    #define C_LOG( format, ... ) ;
    #define C_TRACE( format, ... ) ;
	#define C_SCOPE( msg )         ;
    #define C_TODO ( msg )         ;
    #define C_WARNING( format, ... ) ;
    #define C_ERROR( format, ... ) ;
    #define C_PERF( tag, diff )
#endif


#define QS2CS( qstr ) qstr.toLatin1().constData()

/*!
	Class for CPU-time measurement based on WINAPI QueryPerformanceCounter 
	function and auto c-tor/d-tor calls in C++
*/
class CPerformanceMeasurement
{
	const char *_tag;
	LARGE_INTEGER _counterStart;
public:
	CPerformanceMeasurement(const char * tag):_tag(tag)
	{
		QueryPerformanceCounter( &_counterStart );
	}

	~CPerformanceMeasurement()
	{
		LARGE_INTEGER _counterEnd;
		QueryPerformanceCounter( &_counterEnd );
		unsigned long diff=_counterEnd.HighPart - _counterStart.HighPart;
		diff *=0xFFFF;
		diff += _counterEnd.LowPart - _counterStart.LowPart;
		C_PERF( _tag, diff);
	}
};

#define C_PERF_MSMT CPerformanceMeasurement _scopePerfMeasurementObj(__FUNCTION__);


#endif // LOGSUPPORT_H
