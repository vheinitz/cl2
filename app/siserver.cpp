#include "siserver.h"
#include <log/clog.h>

SIServer::SIServer(QObject*p):QObject(p),_serverConnection(0),_port(0)
{
     
}

void SIServer::onConnnected()
{
   if( _serverConnection == 0)
    {
        _serverConnection = _server.nextPendingConnection();
        connect(_serverConnection, SIGNAL(readyRead()), this, SLOT(onDataRead()));
        connect(_serverConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    }
}

void SIServer::onDataRead()
{
    if ( _serverConnection )
    {
	    QStringList data = QString(_serverConnection->readAll()).split(QRegExp("[\n\r]"));
		foreach ( QString cmd, data)
			emit request( cmd );
		void disconnectCurrentClient( );
    }
    else
    {
        C_ERROR ("Internal error in socket connection");
    }
}

bool SIServer::start( unsigned short port )
{
	_port = port;
	if (!_server.listen(QHostAddress::Any, port ) )
	{
		return false;
	}
		
	connect(&_server, SIGNAL(newConnection()), this, SLOT(onConnnected()));
	return true;
}

void SIServer::stop( )
{
    if( _server.isListening() )
    {
	    _server.close();
        if( _serverConnection )
        {
            _serverConnection->abort();
            delete _serverConnection;
            _serverConnection=0;
        }
    }
}

SIServer::~SIServer()
{
    stop();
}

void  SIServer::onError ( QAbstractSocket::SocketError )
{
    if(_serverConnection)
    {
	    _serverConnection->deleteLater();
	    _serverConnection=0;
    }
}

void SIServer::disconnectCurrentClient( )
{
	if( _serverConnection )
    {
        _serverConnection->close();
        delete _serverConnection;
        _serverConnection=0;
    }
}

void  SIServer::forwardRequest( QString request )
{
    if( _serverConnection )
    {
        _serverConnection->abort();
        delete _serverConnection;
        _serverConnection=0;
    }
	_serverConnection = new QTcpSocket(this);
	_serverConnection->connectToHost("localhost", _port, QIODevice::WriteOnly);
	QTextStream ts( _serverConnection );
	ts << request;
}
