#include "cryptmanager.h"
#include "log/clog.h"
#include <QDateTime>
#include <QCryptographicHash>
#include <QApplication>

CryptManager::CryptManager()
{
	_cryptTool = qApp->applicationDirPath()+"/CSDES.exe"; //preliminary solution - using external tool.
	_cryptKeyVersions["0.0.1"] = "1ESKU!F1";
};

bool CryptManager::encrypt ( QString in, QString out, QString key )
{

	return false;
}

bool CryptManager::decrypt ( QString in, QString out, QString key )
{

	return false;
}

QString CryptManager::getWorkListXmlPath ( QString in  )
{
	//TODO: check version, use appropriate key
	QFile fin(in);
	QString tmpPath;
	if (!fin.exists() || !fin.open(QIODevice::ReadOnly) )
	{
		C_ERROR("Can't decrypt worklist %s. Can't open file.",QS2CS( in ) );
		return "";
	}	
	QString head = fin.read(10);
	fin.close();
	if ( head.contains("xml") )
	{
		return in;
	}
	if (!QFileInfo( _cryptTool ).exists())
	{
		C_ERROR("Can't decrypt worklist %s. Crypt-tool missing.",QS2CS( in ) );
	}
	else
	{
		tmpPath = QDir::tempPath() + "/" + QString::number( QDateTime::currentMSecsSinceEpoch() ) + ".tmp";
		QProcess::execute(_cryptTool, QStringList()<<"de"<<in<<tmpPath );
	}
	return tmpPath;
}