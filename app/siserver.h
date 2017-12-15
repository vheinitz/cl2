#ifndef SI__Server_HG
#define SI__Server_HG

#include <cldef.h>
#include <QtNetwork>
#include <QtCore>



class CLIB_EXPORT SIServer : public QObject
{
    Q_OBJECT
    QTcpServer _server;
	unsigned short _port;
    QTcpSocket *_serverConnection;

signals:
	void request( QString );
private slots:
    void onConnnected();
    void onError ( QAbstractSocket::SocketError );
    void onDataRead();
public:
    SIServer(QObject*p=0);
	bool start( unsigned short port );
    void stop( );
	void disconnectCurrentClient( );
	void forwardRequest( QString );	
	virtual ~SIServer();
};

#endif
