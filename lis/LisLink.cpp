#include "LisLink.h"

#include "ASTMRecord.h"
#include <kvs/kvstore.h>
#include <log/clog.h>
#include <datamapper.h>



LisSendjob::LisSendjob( )
:
	_currentDataIdx(0)
	,_currentSendAttempt(0)
	,_cb(0)
{}

LisSendjob::~LisSendjob( )
{

}


ConnectionManager::ConnectionManager():
_errorCnt(0),
_connection(0),
_maxReconnectNumber(5),
_maxAttemptsPerRecord(5)
{


	int waitBeforeReconnect = 10000;
	int responseTimeout = 10000;

	_timeoutTimer.setInterval( responseTimeout );
	connect(&_timeoutTimer,SIGNAL( timeout() ), this, SLOT(onTimeout() ) );

	_reconnectTimer.setInterval( waitBeforeReconnect );
	connect(&_reconnectTimer,SIGNAL( timeout() ), this, SLOT(createLisConnection() ) );
	_reconnectTimer.setSingleShot(true);
}

void ConnectionManager::onDataRead()
{	
	if ( !_connection )
	{
		C_ERROR("Internal error.");
		return;
	}

	if ( _sendJobs.isEmpty() )
	{
		C_ERROR("Unexpected response from LIS");
		return;
	}
	TPLisSendjob & currentJob = _sendJobs.head();
	_timeoutTimer.stop();
    QByteArray data = _connection->readAll();
	C_LOG("ASTM RX: [%s]", QS2CS( Helpers::dataToString( data )) );
	if ( data.isEmpty() )
	{
		C_WARNING("Data format error. Empty data" );
		return;
	}
	if (data.size() != 1)
	{
		C_ERROR("Data format error." );
		return;
	}
	if ( data.at(0) == ASTM_ACK )
	{
		currentJob->_currentSendAttempt=0;
		++currentJob->_currentDataIdx;
		QTimer::singleShot(50, this, SLOT( processDataToSend() )  );
	}
	else if ( data.at(0) == ASTM_NAK )
	{
		++currentJob->_currentSendAttempt;
		QTimer::singleShot(50, this, SLOT( processDataToSend() )  );
	}
	else
	{
		C_ERROR("Data format error. Unexpected data." );
		return;
	}
}

void ConnectionManager::onConnnected()
{
  emit updateConnectionState();
}

void ConnectionManager::addSendJob( TPLisSendjob job )
{
	_sendJobs.enqueue(job);
	if ( _sendJobs.size() == 1 )
	{
		QTimer::singleShot(10, this, SLOT( processDataToSend() )  );
	}
}

void ConnectionManager::processDataToSend()
{
	if ( _sendJobs.isEmpty() )
	{
		C_ERROR("Unexpected send-request to LIS");
		return;
	}

	if (!_connection )
	{
		C_ERROR("Internal error." );
		return;
	}

	TPLisSendjob & currentJob = _sendJobs.head();

	if ( currentJob->_data.isEmpty() )
	{
		C_ERROR( "Internal error. No data to send to LIS." )
		currentJob->_cb->error();
		return;
	}

	else if ( currentJob->_currentSendAttempt > _maxAttemptsPerRecord)
	{
		C_ERROR( "Comunication error. Max send attempts reached." );
		currentJob->_cb->error();
		//TODO clear job list
		return;
	}

	if (  currentJob->_currentDataIdx >= currentJob->_data.size() )
	{		
		currentJob->_cb->success();
		_sendJobs.dequeue();
		if ( !_sendJobs.isEmpty() )
		{
			QTimer::singleShot(10, this, SLOT( processDataToSend() )  ); //next job
		}
		return;
	}
	else if (  currentJob->_data.at(  currentJob->_currentDataIdx ).size() == 1     //No response to expect, check if next job to send
		&&  currentJob->_data.at(  currentJob->_currentDataIdx ).at(0) == ASTM_EOT )
	{

		C_LOG( "ASTM TX: [%s]", QS2CS( Helpers::dataToString(currentJob->_data.at( currentJob->_currentDataIdx ) )) );
		_connection->write( currentJob->_data.at(  currentJob->_currentDataIdx ) );
		currentJob->_cb->success();
		_sendJobs.dequeue();
		if ( !_sendJobs.isEmpty() )
		{
			QTimer::singleShot(10, this, SLOT( processDataToSend() )  ); //next job
		}
		return;
	}
	else
	{
		_timeoutTimer.start();
		C_LOG( "ASTM TX: [%s]", QS2CS( Helpers::dataToString(currentJob->_data.at(  currentJob->_currentDataIdx ) )) );
		_connection->write( currentJob->_data.at(  currentJob->_currentDataIdx ) );
	}
}

void ConnectionManager::onTimeout()
{
	C_ERROR("Timeout while waiting for LIS response" );
	stopLisConnection();
	emit updateConnectionState();
}

void ConnectionManager::onError( QAbstractSocket::SocketError err )
{
	_errorCnt++;
	emit updateConnectionState();
	stopLisConnection();
	if ( _errorCnt < _maxReconnectNumber )
		_reconnectTimer.start();		
	else
		C_ERROR("Max number of reconnects reached. LIS-link closed")
}

bool ConnectionManager::isConnected()const
{
	if (!_connection)
		return false;
	if ( _connection->state() == QAbstractSocket::ConnectedState)
		return true;

	return false;
}

void ConnectionManager::cleanupSocket()
{
	if (_connection )
	{
		_connection->deleteLater();
		_connection = 0;
	}
}

void ConnectionManager::stopLisConnection()
{
	if (_connection )
	{
		_timeoutTimer.stop();
		_sendJobs.clear();
		cleanupSocket();
		emit updateConnectionState();	
	}
}

void ConnectionManager::onDisconnected( )
{
	if (_connection )
	{
	cleanupSocket();

	emit updateConnectionState();
	if ( !_sendJobs.isEmpty() )
	{
		C_ERROR("LIS closed connection unexpectedly. Still data to send.")
	}
}
}

ConnectionManager & ConnectionManager::instance()
{
	static ConnectionManager inst;
	return inst;
}

void ConnectionManager::startLisConnection()
{
	if ( _connection )
	{
		//C_ERROR("State error connection already established");
		return;
	}
	_reconnectTimer.stop();
	_errorCnt=0;
	createLisConnection();
}


void ConnectionManager::createLisConnection()
{
	if ( _connection )
	{
		//C_ERROR("State error connection already established");
		return;
	}
	_connection = new QTcpSocket(this);


	QString host = "localhost";
	unsigned short port = 4545;
	_connection->connectToHost( host, port );
	

	connect(_connection,SIGNAL(connected()), this, SLOT(onConnnected()));
	connect(_connection,SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(_connection,SIGNAL(readyRead()), this, SLOT(onDataRead()));
    connect(_connection,SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}
