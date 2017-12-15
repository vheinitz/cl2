#ifndef _LIS_LINK__H
#define _LIS_LINK__H

#include <log/clog.h>
#include <cldef.h>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QByteArray>
#include <QList>
#include <QQueue>
#include <QTimer>

#include <QMutex>

class  ASTMSendCallback;


/*TODO remove class CLIB_EXPORT ConnectionUsers
{
public:
	ConnectionUsers( QObject *_firstUser=0):_connection(0),_currentUser(0)
	{
		if (_firstUser)
		{
			_currentUser = _firstUser;
			_users.append( _currentUser );
		}
	}
	QList<QObject*> _users;
	QTcpSocket * _connection;
	QObject *_currentUser;
};*/



class CLIB_EXPORT LisSendjob : public QObject
{
	Q_OBJECT

public:
	LisSendjob( );
	QList<QByteArray> _data;
	int _currentDataIdx;
	int _currentSendAttempt;
	ASTMSendCallback * _cb;
	virtual ~LisSendjob();
};

typedef QSharedPointer<LisSendjob> TPLisSendjob;

class CLIB_EXPORT ConnectionManager : public QObject
{
Q_OBJECT
	ConnectionManager();
	QMutex _mx;
	QQueue<TPLisSendjob> _sendJobs;
	QTcpSocket * _connection;
	QTimer _timeoutTimer;
	QTimer _reconnectTimer;
	int _errorCnt;

private slots:
	void onConnnected();
	void onDisconnected();
	void onDataRead();
	void onTimeout();
	void onError( QAbstractSocket::SocketError );
	void processDataToSend();
	void cleanupSocket();

public slots:
	void startLisConnection();
	void createLisConnection();
	void stopLisConnection();
	bool isConnected()const;
signals:
	void updateConnectionState();

public:
	static ConnectionManager & instance();
	void addSendJob( TPLisSendjob job );

	int _maxReconnectNumber;
	int _maxAttemptsPerRecord;

};



#endif