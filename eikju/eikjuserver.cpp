#include "eikjuserver.h"
#include "eikjucommand.h"
#include "eikjufunction.h"


EiKjuServer::EiKjuServer( QObject * parent ):QObject(parent),
_wwwroot("./") 
{     
     connect(&_server, SIGNAL(newConnection()), this, SLOT(onConnnected()));
     _mimeType.insert("html","text/html; charset=utf-8");
     _mimeType.insert("jpg","image/jpeg");
     _mimeType.insert("png","image/png");
     _mimeType.insert("gif","image/gif");
     _mimeType.insert("css","text/css");
}

bool  EiKjuServer::start( unsigned short port )
{
	return _server.listen(QHostAddress::Any, port );
}

bool  EiKjuServer::stop( )
{
	_server.close();
	return true;
}

bool EiKjuServer::setRoot(QString r)
{
	_wwwroot = r;
	return QFileInfo(r).exists();
}

void EiKjuServer::onConnnected()
{
   
    QTcpSocket * _serverConnection = _server.nextPendingConnection();
    connect(_serverConnection, SIGNAL(readyRead()), this, SLOT(onDataRead()));
    connect(_serverConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    
}

void EiKjuServer::onDataRead()
{
	QTcpSocket * _serverConnection = qobject_cast<QTcpSocket *>(sender());
    EKHttpRequest request;
    while( _serverConnection->bytesAvailable() )
    {
        request.append( _serverConnection->readLine() );;
    }
    request.parse();
    //qDebug( QString("%1").arg(request.httpData().join(";")).toStdString().c_str() );
    if ( request.isValide() )
    {
        QString reqUrl;
        QString filedata;
        QString responseHeader;
        QByteArray responseData;
        if ( request.url() == "/"  )
        {
            reqUrl = _wwwroot + "index.html";			
        }
        else
        {
            reqUrl = _wwwroot + "/" + request.url();            
        }
        QString fileType = request.url().section('.',-1);
        QString mime = _mimeType[fileType];
        if (mime.isEmpty()) mime="text/html";

        //qDebug( QString("%1,%2").arg(mime, fileType).toStdString().c_str()) ;
        QFile f( reqUrl );
        if( f.exists() && f.open(QIODevice::ReadOnly) )
        {

            if ( fileType == "html" )
            {
                if ( !request.getVars().isEmpty() )
                {
                    EiKjuCommand::instance().process( request );
                }
                QString compositData;
                QString data =f.readAll();
                QStringList codeSections = data.split("<!-- [[%");
                foreach ( QString code, codeSections )
                {
                    QString kcode;
                    QString hcode;
                    if ( code.contains( "%]] -->",Qt::CaseSensitive ) )
                    {
                        kcode = code.section("%]] -->",0,0);
                        hcode = code.section("%]] -->",1,1);
                    }
                    else
                    {
                        hcode = code;
                    }

                    if (!kcode.isEmpty())
                    {
                        kcode = kcode.trimmed();
                        QString fname = kcode.section(" ",0,0);
                        QString format = kcode.section(" ",1);
                        if ( format.isEmpty() )
                        {
                            format="%1";
                        }
                        compositData += EiKjuFunction::instance().process(fname,format,request);
                    }
                    compositData += hcode;
                     //qDebug( QString("%1").arg(compositData).toStdString().c_str()) ;
                }
                responseData = compositData.toAscii();
            }
            else
            {
                responseData = f.readAll();
            }
            responseHeader = QString( "HTTP/1.1 200 OK\r\nContent-Type: %1\r\nPragma-directive: no-cache\r\nCache-directive: no-cache\r\nCache-control: no-cache\r\nPragma: no-cache\r\nExpires: 0\r\nContent-Length: %2\r\n\r\n" ).arg(mime).arg(responseData.size());
        }
        else
        {
            responseHeader = "HTTP/1.1 400 Error\r\nContent-Length: 0\r\n\r\n";
        }
        _serverConnection->write(responseHeader.toAscii());
        _serverConnection->write(responseData);
		_serverConnection->waitForBytesWritten(1000);
		_serverConnection->close();
		_serverConnection->deleteLater();
		_serverConnection = 0;
        //qDebug( QString("%1").arg(responseHeader).toStdString().c_str()) ;
    }    
}

 void  EiKjuServer::onError ( QAbstractSocket::SocketError )
 {
	 QTcpSocket * _serverConnection = qobject_cast<QTcpSocket *>(sender());
    _serverConnection->deleteLater();
    _serverConnection=0;
 }
