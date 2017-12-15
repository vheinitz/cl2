#ifndef CL_QMLCLIENT_H
#define CL_QMLCLIENT_H

#if 0

#include "pddatapool.h"
#include <QWidget>
#include <QDeclarativeView> //todo: forward!
#include <QDeclarativePropertyMap> //todo: forward!
#include <QAbstractSocket> //todo: forward!
#include <QSslSocket> //todo: forward!
#include <QStringList>
#include <QSet>
#include <QTimer>
#include <QDateTime>
#include <QShortcut>


class CLIB_EXPORT QmlClient : public QWidget
{
    Q_OBJECT

public:
    QmlClient(QWidget *parent = 0);
    ~QmlClient();

    void setBaseUrl( const QString & url ){ _baseUrl = url; }

    Q_INVOKABLE bool processCommand(QString command, QString value, QString param=QString::null);

signals:
    void forwardCommand(QString command, QString value, QString param=QString::null);

/////Data-Pool interface
private: 
    QString dummy(  ){ return QString::null;}
    void processValueChange(QString);
public:
    Q_PROPERTY( QString valueChanged READ dummy WRITE processValueChange USER true)
/////END Data-Pool interface

private:
    QDeclarativeView *_view;
    QDeclarativePropertyMap _dataBinding;
    QSslSocket *_socket;
    QMap<QString, QString> _subscription;
    QMap<QString, QString> _varToDP;
    static QString _baseUrl;
	QString _currentUI;
    void subscribe( QString qml );
	QShortcut *_reloadSK;


    struct DeviceLookUpData
    {
        DeviceLookUpData( 
            QDateTime lastUpdate = QDateTime::currentDateTime(),
            QString host = QString::null,
            quint16 port = 0 ):
            _lastUpdate(lastUpdate), _host(host), _port(port) {}
        QDateTime _lastUpdate;
        QString _host;
        quint16 _port;
    };
    QMap<QString, DeviceLookUpData> _foundDevices;
    QTimer _devicePresenceChecker;

private slots:
    void updateDeviceList();
	void reloadUI();

private:
    void connectToServer( QString host, quint16 port = 9991 );
    void disconnectFromServer();

public slots:
    void deviceFound( QString );
    //From outside
    void processDataChanged ( const QString & key, const QVariant & value );

private slots:

    //socket interface    
    void processStateChanged(QAbstractSocket::SocketState state);
    void processEncrypted();
    void processReadyRead();
    void processSslErrors(const QList<QSslError> &errors);

    //Qml interface
    void processValueChanged ( const QString & key, const QVariant & value );
};

#endif // CL_QMLCLIENT_H


#endif
