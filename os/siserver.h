#ifndef SI__Server_HG
#define SI__Server_HG


#include <QtNetwork>
#include <QtCore>

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

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
	void forwardReqiest( QString );	
	virtual ~SIServer();
};

#endif
