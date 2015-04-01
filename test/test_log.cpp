#include <QtTest/QtTest>
#include <QFileInfo>
#include <QDateTime>
#include <../log/clog.h>

class test_log: public QObject
{
    Q_OBJECT
	QString _logDir;
	QString _logFile1;
private slots:
	void initTestCase();
	void create_log();
	void log_error();
	void log_warning();
	void log_log();

};
 
void test_log::initTestCase()
{
	_logDir = QString("c:/temp/cl2/test/log/") + QDateTime::currentDateTime().toString("yyyymmddThhMMss");
	_logFile1 = _logDir+"/test1.log";
}

void test_log::create_log()
{
	QVERIFY( QFileInfo(_logDir).exists() == false);
	CLogger::instance().initLogging(QStringList());
	
	CLogger::instance().addLogChannel( new FileLoggChannel(_logFile1) );

	//Only logdir path created. no file yet
	QVERIFY( QFileInfo(QFileInfo( _logFile1 ).absolutePath()).exists() == true);
}

void test_log::log_error()
{
    C_ERROR( "message" )
	QVERIFY( QFileInfo( _logFile1 ).exists() == true);
}

void test_log::log_warning()
{
	C_WARNING( "message" )
	QVERIFY( QFileInfo( _logFile1 ).exists() == true);
}

void test_log::log_log()
{
	C_LOG( "message" )
	QVERIFY( QFileInfo( _logFile1 ).exists() == true);
}


QTEST_MAIN(test_log)
#include "test_log.moc"