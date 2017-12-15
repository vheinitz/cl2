#ifndef LIS_PROXY_HG_
#define LIS_PROXY_HG_

#include <log/clog.h>
#include <cldef.h>
#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QList>
#include <QTimer>
#include <QSet>
#include <QFileSystemWatcher>

enum TLisMode{ LisMode_Tcp=0, LisMode_File, LisMode_None=4711 };

class LisConfiguration
{
public:
	QString _orderFileNameFormat;
	QString _requestFileNameFormat;
	QString _resultFileNameFormat;
	bool _removeAfterRead;
	LisConfiguration():_removeAfterRead(true)
	{	
	}
};

class CLIB_EXPORT LisProxy : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *_lisSocket;
    QTcpSocket *_deviceSocket;
    QTcpServer *_proxyServer;
	QFileSystemWatcher *_deviceLisTraceWatcher;
	QFileSystemWatcher *_lisLisTraceWatcher;
	QByteArray _lisDataToSave;
	TLisMode _lisMode;


	QString _deviceSendFolder;
	QString _deviceReceiveFolder;
	QSet<QString> _deviceReceiveFolderFiles;
	QSet<QString> _deviceSendFolderFiles;

	QString _lisSendFolder;
	QString _lisReceiveFolder;
	QSet<QString> _lisReceiveFolderFiles;
	QSet<QString> _lisSendFolderFiles;

	QByteArray _lisData;
	QList<QByteArray> _deviceData;
	QByteArray _deviceLastChunk;

	QTimer *_deviceDataCollectTimer;
	QTimer *_lisDataCollectTimer;
	bool _sendingLis2Data;
	bool _beginInstrumentMessage;
	bool _terminateInstrumentMessage;
	int _recordSequence;
	bool _enableLogging;
	LisConfiguration _lisconfiguration;

private slots:
    void processLisConnected ();
    void processLisDisconnected ();
    void processLisError ( QAbstractSocket::SocketError socketError );
    void processLisData();
	void storeLisData();

    void processDeviceConnected ();
    void processDeviceDisconnected ();
    void processDeviceError ( QAbstractSocket::SocketError socketError );
    void collectDeviceData();
	void processDeviceData();
    void acceptDeviceConnection();
	void processDeviceDirectoryChange( const QString & path );
	void processDeviceDirectoryChangeTO( );

	void processLisDirectoryChange( const QString & path );
	void processLisDirectoryChangeTO( );
	QByteArray applyTriturusWorkaround( QByteArray );

public:
    LisProxy(QObject *parent = 0);

	virtual ~LisProxy();

	TLisMode lisMode() const { return _lisMode; }

	bool lisConnected() const 
	{
		return _lisSocket != 0 && _lisSocket->state() == QAbstractSocket::ConnectedState;
	}

	bool deviceConnected() const 
	{
		return _deviceSocket != 0 && _deviceSocket->state() == QAbstractSocket::ConnectedState;
	}
    
signals:
	void lisConnectionState(bool);
	void deviceConnectionState(bool);
	void sendErrorMessage(QString);
	void sendLog( QString );
    
public slots:
	void start( unsigned short proxySrvPort, QString sendFolder, QString receiveFolder, const LisConfiguration & lisConf );
    void start(QString lisHost, unsigned short lisPort, unsigned short proxySrvPort );
	void start(QString lisHost, unsigned short lisPort, QString sendFolder, QString receiveFolder );
    void stop();
    
};

#endif // LIS_PROXY_HG_
