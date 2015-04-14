#include "tcpserver.h"
#include <QFileInfo>
#include <QDir>

TcpServer::TcpServer(QObject *parent) :
    QObject(parent),
	_socket(0),
	_server(0)

{
	_dataCollectTimer.setSingleShot(true);
	_dataCollectTimer.setInterval(100);
	connect(    &_dataCollectTimer,   SIGNAL(timeout()),
                    this,           SLOT(processData())  );
}

TcpServer::~TcpServer()
{
	stop();
}



void TcpServer::start(unsigned short port )
{

    if ( !_server )
    {

        _server = new QTcpServer(this);
        _server->listen(QHostAddress::Any, port );
        connect(    _server,   SIGNAL(newConnection()),
                    this,           SLOT(acceptConnection())  );
    }
    else
    {
        //_ERROR( "Already started" );

    }
}

void TcpServer::stop()
{
    if ( _server )
    {
		if (_server->isListening())
			_server->close();		
		_server->deleteLater();
        _server=0;
		_socket = 0;
    }
	if ( _socket )
	{
		_socket->close();
		_socket->deleteLater();
		_socket = 0;
	}
}

bool TcpServer::send( QByteArray d  )
{
	if ( !_socket )
		return false;

	_socket->write(d);
	return true;
}

void TcpServer::processDisconnected ()
{
	emit clientDisonnected();
	if (_socket)
	{
		_dataCollectTimer.stop();
		_socket->deleteLater();
		_socket=0;				
	}	
}

void TcpServer::processError ( QAbstractSocket::SocketError socketError )
{
	emit error();
}

void TcpServer::collectData()
{
	if ( _socket ) 
	{
		_pendingData += _socket->readAll();
		_dataCollectTimer.start();
	}
}

void TcpServer::processData()
{
	_data = _pendingData;
	_pendingData.clear();
	emit newData();

}

void TcpServer::acceptConnection()
{
	 
	if( _socket == 0)
    {
		_data.clear();
		_pendingData.clear();
        _socket = _server->nextPendingConnection();
        connect(_socket, SIGNAL(readyRead()),
                this, SLOT(collectData()));
        connect(    _socket, SIGNAL(disconnected ()),
                    this,       SLOT(processDisconnected())  );
		connect(	_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this,       SLOT(processError(QAbstractSocket::SocketError)));
		emit clientConnected();
    }
	else
	{
		emit error();
	}
}
