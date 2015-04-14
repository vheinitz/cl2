#ifndef SERVER_SOCKET_HG_
#define SERVER_SOCKET_HG_

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QList>
#include <QTimer>
#include <QSet>


class TcpServer : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *_socket;
    QTcpServer *_server;
	QByteArray _pendingData;
	QByteArray _data;



	QTimer _dataCollectTimer;

private slots:
	void acceptConnection();
    void processDisconnected();
    void processError( QAbstractSocket::SocketError socketError );
    void collectData();
	void processData();
    


public:
    TcpServer(QObject *parent = 0);
	virtual ~TcpServer();
    
signals:
	void clientConnected();
	void clientDisonnected();
	void error();
	void newData();
    
public:
    void start(unsigned short port);
    void stop();
	QByteArray data()const{ return _data;}
	bool send( QByteArray  );
    
};

#endif // SERVER_SOCKET_HG_
