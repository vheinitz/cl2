#include "clog.h"
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <QApplication>
#include <fstream>
#include <QMutex>

#pragma warning( push )
#pragma warning( disable : 4996 ) // disable unsafe fopen, strcpy


QtMsgHandler CLogger::_messageHandlerInstaller = qInstallMsgHandler( CLogger::pdMessageOutput );

QString _Log_ScopeTracer_::ScopeIndent;


FileLoggChannel::FileLoggChannel (QString fileName)
{
	_chName = fileName;
	QDir().mkpath( QFileInfo( _chName ).absolutePath() );
}

void FileLoggChannel::writeLog( QString msg )
{
	std::ofstream ofs( QS2CS(_chName), std::ios_base::app);
            ofs <<QS2CS(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\t%1").arg(msg)) <<std::endl; 
}


CLogger::CLogger():_logLevel(PD_LL_ERROR)
{
}

bool CLogger::addLogChannel( LoggChannel * ch )
{ 	
    _logChannels.append(PLoggChannel(ch));
    return true; //TODO
}

bool CLogger::removeLogChannel( QString chName )
{ 
	foreach( PLoggChannel ch, _logChannels )
	{
		if ( ch->chName() == chName )
		{
			_logChannels.removeAll( ch );
			return true;
		}
	}
    return false;
}

CLogger::~CLogger()
{
	//Causes segfault, disabled
    //CLogger::instance()._logStream <<"\n\n"<< QDateTime::currentDateTime().toString("MM-dd_hh:mm:ss:zzz") 
    //        << "\t" << "PDLoger shut down \n";	
}

void CLogger::initLogging( QStringList args) 
{
	int ll = 2; //Warning
    foreach ( QString arg, args )
    {
        if ( arg.contains("--traceLevel") )
        {
            ll = arg.section("=",1).toInt();            
        }
    }
	setLogLevel(ll);
	qInstallMsgHandler( CLogger::pdMessageOutput );
}

void CLogger::stopLogging()
{
	_logChannels.clear();
}

void CLogger::setLogLevel( int ll ) {
	switch(ll)
	{
	case PD_LL_NOLOG:
		_logLevel=0; 
		break;
	case PD_LL_TRACE:
		_logLevel = PD_LL_TRACE | PD_LL_WARNING | PD_LL_ERROR;
		break;
	case PD_LL_WARNING:
		_logLevel = PD_LL_WARNING | PD_LL_ERROR;
		break;
	case PD_LL_PERF:
		_logLevel = PD_LL_PERF;
		break;
	case PD_LL_ERROR:
	default:
		_logLevel = PD_LL_ERROR;
		break;	
	}
}


/*! Singleton logger class. Initializes and opens file on first call
*/
CLogger & CLogger::instance()
{
    static CLogger inst;
    return inst;
}

void CLogger::pdMessageOutput(QtMsgType type, const char *msg)
{
    CLogger::instance().messageOutput( type, msg);
}

QMutex mx;
void CLogger::messageOutput(QtMsgType type, const char *msg)
{
	
	mx.lock();
    static int lineCnt=0;
	static QRegExp appLogRx("ERR|LOG|TRA|WARN|TODO|PERF|SCOPE");
    //ignore anything but aplication logs
    if( !QString(msg).contains( appLogRx ) )
    {
	   mx.unlock();
       return;
    }
  
    foreach ( PLoggChannel logCh, _logChannels ) //TODO stream depending on level too
    {
        if (type == QtFatalMsg)
        {
		    //LOG(FATAL) << msg;
			logCh->writeLog( msg );
            //std::ofstream ofs( QS2CS(logCh), std::ios_base::app);
            //ofs <<QS2CS(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\t"))<< msg <<std::endl; 
            abort();
        }
        else if ( type ==QtDebugMsg )
	    {
		    //LOG(INFO) << msg;
			logCh->writeLog( msg );
            //std::ofstream ofs(QS2CS(logCh),std::ios_base::app);
            //    ofs <<QS2CS(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\t"))<< msg <<std::endl; 
	    }
	    else if (type ==QtWarningMsg)
	    {
		    //LOG(WARNING) << msg;
			logCh->writeLog( msg );
            //std::ofstream ofs(QS2CS(logCh),std::ios_base::app);
            //    ofs <<QS2CS(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\t"))<< msg <<std::endl; 
	    }
	    else if (type ==QtCriticalMsg)
        {
		    //LOG(ERROR) << msg;
			logCh->writeLog( msg );
            //std::ofstream ofs(QS2CS(logCh),std::ios_base::app);
            //    ofs <<QS2CS(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\t"))<< msg <<std::endl; 
        }
    }
	 mx.unlock();
}

#pragma warning( pop )

