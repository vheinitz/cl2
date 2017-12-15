#include "LisProxy.h"
#include "ASTMRecord.h"
#include <QFileInfo>
#include <QDir>

LisProxy::LisProxy(QObject *parent) :
    QObject(parent),
	_lisSocket(0),
	_deviceSocket(0),
	_proxyServer(0),
	_deviceLisTraceWatcher(0),
	_lisLisTraceWatcher(0),
	_lisMode(LisMode_None),
	_sendingLis2Data(false),
	_beginInstrumentMessage(false),
	_terminateInstrumentMessage(false),
	_recordSequence(1),
	_enableLogging(true)

{
	_deviceDataCollectTimer = new QTimer(this);
	_deviceDataCollectTimer->setSingleShot(true);
	_deviceDataCollectTimer->setInterval(100); //todo configurable

	_lisDataCollectTimer = new QTimer(this);
	_lisDataCollectTimer->setSingleShot(true);
	_lisDataCollectTimer->setInterval(100); //todo configurable
}

LisProxy::~LisProxy()
{
	stop();
}

void LisProxy::start( unsigned short proxySrvPort, QString sendFolder, QString receiveFolder, const LisConfiguration & lisConf )
{
	_lisconfiguration = lisConf;
	_lisMode=LisMode_None;
	if ( !(QFileInfo(sendFolder).exists() && QFileInfo(receiveFolder).exists()) )
	{
		//C_ERROR( "Invalid paths" )		
		return;
	}
    if ( !_proxyServer && !_lisLisTraceWatcher )
    {
		_lisMode=LisMode_File;
		emit lisConnectionState(false);
		emit deviceConnectionState(false);
        _proxyServer = new QTcpServer(this);
        _proxyServer->listen(QHostAddress::Any, proxySrvPort );
        connect(    _proxyServer,   SIGNAL(newConnection()),
                    this,           SLOT(acceptDeviceConnection())  );
		_lisLisTraceWatcher = new QFileSystemWatcher(this);
		_lisSendFolder = sendFolder;
		_lisReceiveFolder = receiveFolder;
		_lisLisTraceWatcher->addPaths( QStringList()<<_lisSendFolder<<_lisReceiveFolder );
		connect( _lisLisTraceWatcher, SIGNAL( directoryChanged ( QString ) ), this, SLOT( processLisDirectoryChange ( QString  ) ) );

 		_lisSendFolderFiles = QDir( _lisSendFolder ).entryList().toSet();
		_lisReceiveFolderFiles = QDir( _lisReceiveFolder ).entryList().toSet();
		connect (_deviceDataCollectTimer, SIGNAL(timeout()), this, SLOT( processDeviceData() ) );
    }
    else
    {
        C_ERROR( "LIS connection already started" )
    }
}

void LisProxy::start(QString lisHost, unsigned short lisPort, unsigned short proxySrvPort )
{
	_lisMode=LisMode_None;
    if ( !_proxyServer )
    {
		emit lisConnectionState(false);
		emit deviceConnectionState(false);
        _proxyServer = new QTcpServer(this);
        _proxyServer->listen(QHostAddress::Any, proxySrvPort );
        connect(    _proxyServer,   SIGNAL(newConnection()),
                    this,           SLOT(acceptDeviceConnection())  );

        _lisSocket = new QTcpSocket;
        connect(    _lisSocket, SIGNAL(readyRead()),
                    this,       SLOT(processLisData())  );
        connect(    _lisSocket, SIGNAL(connected ()),
                    this,       SLOT(processLisConnected())  );
        connect(    _lisSocket, SIGNAL(disconnected ()),
                    this,       SLOT(processLisDisconnected())  );
		connect(	_lisSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this,       SLOT(processLisError(QAbstractSocket::SocketError)));
        _lisSocket->connectToHost(lisHost,lisPort);       

		connect (_deviceDataCollectTimer, SIGNAL(timeout()), this, SLOT( processDeviceData() ) );
		_lisMode=LisMode_Tcp;

    }
    else
    {
        //_ERROR( "Already started" );

    }
}



void LisProxy::start(QString lisHost, unsigned short lisPort, QString sendFolder, QString receiveFolder )
{
	_lisMode=LisMode_None;
	if ( !(QFileInfo(sendFolder).exists() && QFileInfo(receiveFolder).exists()) )
	{
		//C_ERROR( "Invalid paths" )
		return;
	}
    
	if ( !_deviceLisTraceWatcher )
    {
		_deviceLisTraceWatcher = new QFileSystemWatcher(this);
		_deviceSendFolder = sendFolder;
		_deviceReceiveFolder = receiveFolder;
		_deviceLisTraceWatcher->addPaths( QStringList()<<_deviceSendFolder<<_deviceReceiveFolder );
		connect( _deviceLisTraceWatcher, SIGNAL( directoryChanged ( QString ) ), this, SLOT( processDeviceDirectoryChange ( QString  ) ) );

        _lisSocket = new QTcpSocket;
        connect(    _lisSocket, SIGNAL(readyRead()),
                    this,       SLOT(processLisData())  );
        connect(    _lisSocket, SIGNAL(connected ()),
                    this,       SLOT(processLisConnected())  );
        connect(    _lisSocket, SIGNAL(disconnected ()),
                    this,       SLOT(processLisDisconnected())  );
		connect(	_lisSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this,       SLOT(processLisError(QAbstractSocket::SocketError)));
        _lisSocket->connectToHost(lisHost,lisPort);       
		QStringList dirs = _deviceLisTraceWatcher->directories();
		connect (_lisDataCollectTimer, SIGNAL(timeout()), this, SLOT( storeLisData() ) );

		

		_deviceSendFolderFiles = QDir( _deviceSendFolder ).entryList().toSet();
		_deviceReceiveFolderFiles = QDir( _deviceReceiveFolder ).entryList().toSet();
		_lisMode=LisMode_Tcp;

    }
    else
    {
        C_WARNING( "LIS communication already started" )

    }
}


void LisProxy::stop()
{
	_lisMode=LisMode_None;
    if ( _proxyServer )
    {
		if (_proxyServer->isListening())
			_proxyServer->close();		
		_proxyServer->deleteLater();
        _proxyServer=0;
		_deviceSocket = 0;
    }
	if ( _deviceSocket )
    {
		_deviceSocket->close();
		_deviceSocket->deleteLater();
		_deviceSocket = 0;
    }
	if ( _lisSocket )
	{
		_lisSocket->close();
		_lisSocket->deleteLater();
		_lisSocket = 0;
	}
	
	if (_deviceLisTraceWatcher)
	{
		_deviceLisTraceWatcher->deleteLater();
		_deviceLisTraceWatcher=0;
		_deviceDataCollectTimer->stop();
	}
	if (_lisLisTraceWatcher)
	{
		_lisLisTraceWatcher->deleteLater();
		_lisLisTraceWatcher=0;
		_lisDataCollectTimer->stop();
	}
	_sendingLis2Data=false;
}

void LisProxy::processLisConnected ()
{
	emit lisConnectionState(true);	
}

void LisProxy::processLisDisconnected ()
{
	emit lisConnectionState(false);
	stop();	
}

void LisProxy::processLisError ( QAbstractSocket::SocketError  )
{
	emit sendErrorMessage(tr("LIS connection error"));
	emit lisConnectionState(false);
}

void LisProxy::processLisData()
{
	if ( _lisSocket ) //TODO prevent race-condition by mx
	{
		QByteArray data = _lisSocket->readAll();
		emit sendLog(QString("<--L:  %1").arg( Helpers::dataToString(data) ));
				
		//Currently sending buffer filled with Device data to LIS. Only ACK/NAK from LIS are accepted.
		if ( _sendingLis2Data ) 
		{
			if (data.size()==1 && data.at(0) == ASTM_ACK)
			{
				QTimer::singleShot(1,this, SLOT(processDeviceData()));
			}
			else if (data.size()==1 && data.at(0) == ASTM_EOT)
			{
				//_sendingLis2Data = false;
				return;
			}
			else if (data.size()==1 && data.at(0) == ASTM_NAK)
			{
				emit sendErrorMessage(tr("Data rejected by LIS"));
				stop();
			}
			else
			{
				emit sendErrorMessage(tr("Protocol error"));
				stop();
			}
		}
		//LIS is sending data to be forwarded to Device
		else
		{
			if (data.size()==1) // Control data. Only ENQ or EOT allowed
			{
				if (data.at(0) == ASTM_ENQ)
				{
					emit sendLog(QString("-->L:  <ACK>"));
					_lisSocket->write( &ASTM_ACK,1 );
				}
				else if (data.at(0) == ASTM_EOT)
				{
					return;
				}
				else
				{
					emit sendErrorMessage(tr("Protocol error"));
					//stop();
					return;
				}
			}
			else
			{								
				QByteArray lis2 = Helpers::toLIS2_A2( data );
				emit sendLog(QString("I<--:  %1").arg( Helpers::dataToString(lis2) ));
				if (_deviceSocket)
				{
					C_LOG( "LIS->DEVICE: %s", QS2CS(QString(lis2)) )
					_deviceSocket->write( lis2 );
				}				
				else if ( _deviceLisTraceWatcher  )
				{
					_lisDataToSave += lis2;
					_lisDataCollectTimer->stop();
					_lisDataCollectTimer->start(3000);
				}
				else
				{
					emit sendLog(QString("-->L:  <NAK>"));
					emit sendErrorMessage(tr("Device not ready"));
					_lisSocket->write( &ASTM_NAK,1 );
					stop();
					return;
				}
				
				emit sendLog(QString("-->L:  <ACK>"));
				_lisSocket->write( &ASTM_ACK,1 );
			}
		}
	}
	else
	{
		emit sendErrorMessage(tr("Internal error"));
	}
}

void LisProxy::processDeviceConnected ()
{
	C_LOG("LIS Device connected");
	emit deviceConnectionState(true);
}

void LisProxy::processDeviceDisconnected ()
{
	if (_deviceSocket)
	{
		_deviceDataCollectTimer->stop();
		_deviceSocket->deleteLater();
		_deviceSocket=0;
		
		if (_sendingLis2Data)
		{
			emit sendErrorMessage(tr("Warning: Communication disconnected while sending"));
			C_WARNING("Device lost while sending data");	
		}
	}	
	C_LOG("LIS Device disconnected");
	emit deviceConnectionState(false);
}

void LisProxy::processDeviceError ( QAbstractSocket::SocketError socketError )
{

	if ( socketError == QAbstractSocket::RemoteHostClosedError && !_sendingLis2Data) //not error
	{
		emit deviceConnectionState(false);
		return;
	}
	else if ( socketError == QAbstractSocket::RemoteHostClosedError && _sendingLis2Data) //Warning. Ifa lost while sending data
	{
		emit sendErrorMessage(tr("Warning. Device disconnected while sending data"));
		C_ERROR("Device error (%d)", socketError)
	}
	else 
	{
		C_ERROR("Device error %d", socketError);
		emit sendErrorMessage(tr("Communication error (%1)").arg(socketError));
		//stop();
	}

	if (_deviceSocket)
	{
		_deviceDataCollectTimer->stop();
		_deviceSocket->deleteLater();
		_deviceSocket=0;
	}

	
	emit deviceConnectionState(false);
}

QByteArray LisProxy::applyTriturusWorkaround( QByteArray datain )
{
	QByteArray DeviceData;

	QString strdata = datain;
	static int PatientRecordNumber=1;
	static int OrderRecordNumber=1;
	static int ResultRecordNumber=1;
	static bool triturus = false;
	//Triturus bugfix
	if ( strdata.contains( QRegExp("H[|][\\\\][^][&].*Triturus") ) )
	{
		triturus = true;
		PatientRecordNumber=0;
		OrderRecordNumber=0;
		ResultRecordNumber=0;
	}

	if (triturus)
	{
		
		if ( strdata.contains( QRegExp("^P[|][0-9]+") ) )
		{
			PatientRecordNumber++;
			OrderRecordNumber=0;
			ResultRecordNumber=0;
			strdata.replace( QRegExp("^P[|][0-9]+"), QString("P|%1").arg(PatientRecordNumber) );
		}
		else if ( strdata.contains( QRegExp("^O[|][0-9]+") ) )
		{
			OrderRecordNumber++;
			ResultRecordNumber=0;
			strdata.replace( QRegExp("^O[|][0-9]+"), QString("O|%1").arg(OrderRecordNumber) );
		}
		else if ( strdata.contains( QRegExp("^R[|][0-9]+") ) )
		{
			ResultRecordNumber++;
			strdata.replace( QRegExp("^R[|][0-9]+"), QString("R|%1").arg(ResultRecordNumber) );
		}
		DeviceData = strdata.toAscii();

	}
	else
	{
		DeviceData = datain;
	}

	return DeviceData;
}


void LisProxy::collectDeviceData()
{
	if ( _deviceSocket ) //TODO prevent race-condition by mx
	{
		_deviceDataCollectTimer->stop();
		_deviceDataCollectTimer->start();
		QByteArray DeviceData = applyTriturusWorkaround (_deviceSocket->readLine() );	

		C_LOG("DEVICE->LIS %s",QS2CS(QString(DeviceData)));
		
		emit sendLog(QString("I-->:  %1").arg( Helpers::dataToString(DeviceData) ));
		if ( _lisMode == LisMode_Tcp )
		{
			_deviceLastChunk.append(DeviceData);
			QList<QByteArray> recs = _deviceLastChunk.split(ASTM_CR);
			if (recs.size()>1)
			{
				for(int i= 0; i < recs.size()-1; ++i)
				{
					_deviceData += recs.at(i); //Helpers::toLIS01_A2( recs.at(i), _recordSequence ) ;
				}
				_deviceLastChunk = recs.last();
			}
		}
		else if( _lisMode == LisMode_File )
		{
			_deviceData += DeviceData;
		}
		else
		{
			C_LOG("LIS internal error");
			emit sendErrorMessage(tr("internal error"));
			stop(); 
		}
	}
	else
	{
		C_LOG("LIS internal error");
		emit sendErrorMessage(tr("internal error"));
		stop(); 
	}
}


/*!
-Starts ASTM-session
-Sends Device data 
-Closes ASTM-session
*/
void LisProxy::processDeviceData()
{
	if ( _lisMode==LisMode_Tcp && _lisSocket ) //TODO prevent race-condition by mx
	{
		if(!_sendingLis2Data) //startsesstion
		{
			emit sendLog(QString("-->L:  <ENQ>"));
			_lisSocket->write( &ASTM_ENQ, 1 );
			_sendingLis2Data=true;
		}
		else
		{
			if( _deviceData.isEmpty() )
			{
				emit sendLog(QString("-->L:  <EOT>"));
				_lisSocket->write( &ASTM_EOT,1 );
				_sendingLis2Data=false;
				_terminateInstrumentMessage=false;
				_recordSequence=1;
			}
			else if( _beginInstrumentMessage )
			{
				emit sendLog(QString("-->L:  <ENQ>"));
				_lisSocket->write( &ASTM_EOT,1 );
				_beginInstrumentMessage=false;
			}
			else if( _terminateInstrumentMessage )
			{
				emit sendLog(QString("-->L:  <EOT>"));
				_lisSocket->write( &ASTM_EOT,1 );
				_terminateInstrumentMessage=false;
				_beginInstrumentMessage=true;
				_recordSequence=1;
			}
			else
			{
				QByteArray rec = _deviceData.takeFirst();
				emit sendLog(QString("-->L:  %1").arg( Helpers::dataToString(rec) ));
				if ( !rec.isEmpty() && rec.at(0) == 'L' )
				{
					_terminateInstrumentMessage = true;
				}
				QList<QByteArray> rec_01A2 = Helpers::toLIS01_A2( rec, _recordSequence );
				foreach( QByteArray r, rec_01A2)
					_lisSocket->write( r );
			}
		}
	}
	else if ( _lisMode==LisMode_File && QFileInfo(_lisSendFolder).exists() )
	{
		static int sessionCnt=0;
		QString reqFname = _lisconfiguration._requestFileNameFormat.arg( QString("%1%2").arg(QDateTime().currentDateTime().currentMSecsSinceEpoch()).arg(QString::number(sessionCnt++)) );
		QString fn = QString("%1/%2").arg(_lisSendFolder).arg(reqFname);						

		if ( QFileInfo(fn).exists() )
		{
			C_ERROR( "LIS configuration error. File conflict (exists) for %s", QS2CS(fn) );
			return;
		}

		QFile f( fn );
		if (!f.open(QIODevice::WriteOnly))
		{
			C_ERROR("Can't send data to LIS via file: %s", QS2CS(fn) );
			return;
		}

		foreach(QByteArray ln, _deviceData)
		{
			f.write(ln);
			//f.write(&ASTM_CR,1);
		}
		_deviceData.clear();
		f.close();
	}
	else
	{
		C_LOG("LIS internal mode error");
		emit sendErrorMessage(tr("Internal error"));
		return;
	}
}

void LisProxy::storeLisData()
{
	QString p = _deviceReceiveFolder +"/"+QDateTime( ).currentDateTime().toString("yyyymmddhhMMss0000")+".dat";
	QFile f( p );
	f.open(QIODevice::WriteOnly);
	f.write(_lisDataToSave);
	f.close();
	_lisDataToSave.clear();
}

void LisProxy::processDeviceDirectoryChange ( const QString &  )
{
	QTimer::singleShot(5000,this,SLOT( processDeviceDirectoryChangeTO( ) ));
}

void LisProxy::processDeviceDirectoryChangeTO( )
{
	QSet<QString> actualSendFolderFiles = QDir( _deviceSendFolder ).entryList().toSet();

	foreach( QString nf, actualSendFolderFiles.subtract( _deviceSendFolderFiles ).toList() )
	{
		QString p = _deviceSendFolder +"/"+nf;
		QFile f( p );
		f.open(QIODevice::ReadOnly);
		QTextStream ts( &f );
		while (!ts.atEnd() )
		{
			QByteArray dline = ts.readLine().toAscii();
			QByteArray tritCleanedData = applyTriturusWorkaround ( dline );
			_deviceData += Helpers::toLIS01_A2( tritCleanedData, _recordSequence ) ;
		}
		processDeviceData();
		f.close();
		emit sendLog(_deviceSendFolder);
	}
	_deviceSendFolderFiles = QDir( _deviceSendFolder ).entryList().toSet();

}


void LisProxy::processLisDirectoryChange ( const QString &  )
{
	QTimer::singleShot(2000,this,SLOT( processLisDirectoryChangeTO( ) ));
}

void LisProxy::processLisDirectoryChangeTO( )
{
	//QSet<QString> actualSendFolderFiles = QDir( _lisSendFolder ).entryList().toSet();
	QSet<QString> actualReceiveFolderFiles = QDir( _lisReceiveFolder ).entryList().toSet();

	QRegExp orderRx( _lisconfiguration._orderFileNameFormat );

	foreach( QString nf, actualReceiveFolderFiles.subtract( _lisReceiveFolderFiles ).toList() )
	{
		if ( orderRx.exactMatch( nf ) )
		{
			QString p = _lisReceiveFolder +"/"+nf;
			QFile f( p );
			f.open(QIODevice::ReadOnly);
			QByteArray data;
			QTextStream ts(&f);
			while ( !ts.atEnd() )
			{
				QString ln = ts.readLine();
				if (ln.isEmpty())
					continue;
				data.append(ln.toAscii());
				data.append(ASTM_CR);
			}
			f.close();
			if( _deviceSocket )
			{
				C_LOG( "LIS_FILE->DEVICE: %s", QS2CS(QString(data)) )
				_deviceSocket->write( data  );
			}
			if ( _lisconfiguration._removeAfterRead )
			{
				QFile::remove( p );
			}
		}
		
		//emit sendLog(_lisReceiveFolder);
	}
	_lisReceiveFolderFiles = QDir( _lisReceiveFolder ).entryList().toSet();		
}


void LisProxy::acceptDeviceConnection()
{
	 
	if( _deviceSocket == 0)
    {
		_lisData.clear();
		_deviceData.clear();
		_deviceLastChunk.clear();
        _deviceSocket = _proxyServer->nextPendingConnection();
        connect(_deviceSocket, SIGNAL(readyRead()),
                this, SLOT(collectDeviceData()));
		connect(    _deviceSocket, SIGNAL(connected ()),
                    this,       SLOT(processDeviceConnected())  );
        connect(    _deviceSocket, SIGNAL(disconnected ()),
                    this,       SLOT(processDeviceDisconnected())  );
		connect(	_deviceSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this,       SLOT(processDeviceError(QAbstractSocket::SocketError)));
		emit deviceConnectionState(true);

		//ui->statusBar->showMessage( "Accepted Device connection" );
    }
	else
	{
		emit sendErrorMessage(tr("2nd Device connection rejected"));
	}
}
