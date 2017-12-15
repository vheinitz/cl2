#include "qmlclient.h"


#if 0

#include <QDeclarativeContext>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QMap>

QString QmlClient::_baseUrl;

QmlClient::QmlClient(QWidget *parent) :
    QWidget(parent),
    _view(0),
    _socket(0)
{
    _view = new QDeclarativeView(this);
    setLayout( new QVBoxLayout );
    layout()->addWidget( _view );


    //Qml Interface
    connect( &_dataBinding, SIGNAL(valueChanged(QString,QVariant)), this, SLOT(processValueChanged(QString,QVariant)) );
    connect( &_devicePresenceChecker, SIGNAL(timeout( )), this, SLOT(updateDeviceList( )) );

    _devicePresenceChecker.start(3000);
    
	_reloadSK = new QShortcut(QKeySequence(tr("F5", "Reload")), this);
	connect( _reloadSK, SIGNAL(activated()), this, SLOT(reloadUI()));
    
    resize(800,600);
}

QmlClient::~QmlClient()
{
    
}

void QmlClient::reloadUI()
{
	processCommand("openUI", _currentUI);
}

void QmlClient::deviceFound( QString msg )
{

    QStringList entries = msg.split(QRegExp("[;\\n]+"));
    QMap<QString, QString> entryMap;
    foreach ( QString e, entries){
        entryMap[ e.section(":",0,0).trimmed() ] = e.section(":",1).trimmed();
    }    

    QString deviceId = entryMap["id"];
    if ( deviceId.isEmpty() )
    {
        deviceId = QString("%1:%2").arg( entryMap["__address"], entryMap["port"] );
    }
    
    _foundDevices[deviceId]  = DeviceLookUpData( QDateTime::currentDateTime(), entryMap["__address"], entryMap["port"].toUInt() );
    updateDeviceList();
}

void QmlClient::updateDeviceList()
{
    QDateTime now = QDateTime::currentDateTime();
    QMap<QString, DeviceLookUpData>::iterator it = _foundDevices.begin();
    while( it != _foundDevices.end() )
    {
        //if no update since 5 secs -> delete entry
        if ( now.toMSecsSinceEpoch() - it.value()._lastUpdate.toMSecsSinceEpoch() > 3000  )
        {
            _foundDevices.remove(it.key());
            it = _foundDevices.begin();
            continue;
        }
        ++it;
    }
    QDeclarativeContext *context = _view->rootContext();
    QStringList deviceList = _foundDevices.keys();
    context->setContextProperty("deviceList", QVariant::fromValue( deviceList ));    

}

void QmlClient::connectToServer( QString host, quint16 port )
{
    if (_socket)
    {
        _socket->abort();
        _socket->deleteLater();
        _socket = 0;
    }
    _socket = new QSslSocket(this);
    _socket->setPeerVerifyMode(QSslSocket::VerifyNone);
    connect(_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
    connect(_socket, SIGNAL(encrypted()),
            this, SLOT(processEncrypted()));
    connect(_socket, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(processSslErrors(QList<QSslError>)));
    connect(_socket, SIGNAL(readyRead()),
            this, SLOT(processReadyRead()));
    _socket->connectToHostEncrypted( host, port);
}

void QmlClient::disconnectFromServer()
{
    if (_socket)
    {
        _socket->abort();
        _socket->deleteLater();
        _socket = 0;
    }
    else
    {
        //todo wrn msg
    }
}

bool QmlClient::processCommand(QString command, QString value, QString param)
{
    if ( command == "openUI" )
    {
        //todo:
        // -unsubscribe old vars
        // -parse qml for data-connections and subscribe

        QDeclarativeView *old = _view;
        _view = new QDeclarativeView(this);
        _view->setGeometry(old->geometry() );
        QDeclarativeContext *context = _view->rootContext();
        context->setBaseUrl( QUrl::fromLocalFile( _baseUrl ) );
        context->setContextProperty("datapool", &_dataBinding );
        context->setContextProperty("functions", this );        
        updateDeviceList();

		_currentUI = value;
        QString url = context->baseUrl().toLocalFile() + _currentUI;
        subscribe( url );
        _view->setSource(  url );		
        _view->show();
        old->deleteLater();

    }
    else if ( command == "connect" )
    {
        //QString host =  value.section(":",0,0);
        //quint16 port =  value.section(":",1).toUInt();
        QMap<QString, DeviceLookUpData>::const_iterator it = _foundDevices.find(value);
        if ( it == _foundDevices.end() )
        {
            //todo errmsg, e.g. C_ERROR("Requested device not found");
        }
        else
        {
            connectToServer( it.value()._host, it.value()._port );
        }        
    }
    else if ( command == "startProcess" )
    {
        if (_socket)
        {
            QDataStream ds(_socket);
            ds.setVersion(QDataStream::Qt_4_8);
            ds << QString("START_PROCESS") << value;
        }
        else
        {
            Processing::instance().startProcess( value );
        }
        
    }
    else
    {
        emit forwardCommand( command,  value,  param );
    }
    return true;
}



void QmlClient::processValueChanged ( const QString & var , const QVariant &val  )
{
    //qDebug() << "send "<<key<<"="<<value<<" to the server";
    KVStore::instance().set(_varToDP[var], val, this);
    
}

void QmlClient::processStateChanged(QAbstractSocket::SocketState state)
{
    if (!_socket)
    {
        //todo errmsg
        return;
    }

    if (state == QAbstractSocket::UnconnectedState) {
        _socket->deleteLater();
        _socket = 0;
        processCommand("openUI", "./connect.qml");
    }
    else if (state == QAbstractSocket::ConnectedState ) {
        processCommand("openUI", "./main.qml");
    }
}

void QmlClient::processEncrypted()
{
    if (!_socket)
    {
        //todo errmsg
        return;
    }
//    QSslCipher ciph = socket->sessionCipher();
//    QString cipher = QString("%1, %2 (%3/%4)").arg(ciph.authenticationMethod())
//                     .arg(ciph.name()).arg(ciph.usedBits()).arg(ciph.supportedBits());
}

void QmlClient::processReadyRead()
{
    if (!_socket)
    {
        //todo errmsg
        return;
    }
    QDataStream ds(_socket);
    ds.setVersion(QDataStream::Qt_4_8);
    while( !ds.atEnd() )
    {
        QString cmd, var;
        QVariant val;
        ds >> cmd >> var >> val;
        processDataChanged( var, val );
        //qDebug() << cmd << var <<val; 
        
    }        
}

void QmlClient::processValueChange( QString var )
{
    QVariant val = KVStore::instance().get(var);
    processDataChanged( var, val );
}

void QmlClient::processDataChanged ( const QString & var, const QVariant & val )
{
    _dataBinding.insert( _subscription[var], val );
}

void QmlClient::subscribe( QString qml )
{
  
    QByteArray buf;
    QDataStream ds(&buf,QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_4_8);

    QString unsubscrMsg("UNSUBSCRIBE ");
    ds << QString("UNSUBSCRIBE") <<  _subscription.keys();
    QStringList vars = _subscription.keys();
    unsubscrMsg += vars.join(",");
    _subscription.clear();
    _varToDP.clear();

    QFile f(qml);
    f.open(QIODevice::ReadOnly);
    QTextStream ts(&f);
    {
        while (!ts.atEnd()){
            QString line = ts.readLine();
            if ( line.indexOf("//VAR ") == 0 ) // format: //VAR alias=Pv.Var
            {
                QStringList varSet = line.section( "//VAR ",1).simplified().split("=");
                _subscription[ varSet[1] ] = varSet[0];
                _varToDP[varSet[0]] = varSet[1];
            }
        }
    }

    //QDataStream ds1(&buf,QIODevice::WriteOnly);
    ds << QString("SUBSCRIBE") <<  _subscription.keys();
    if (_socket)
    {
        _socket->write( buf );
    }
    else
    {
        foreach( QString var, _subscription.keys() )
        {
            KVStore::instance().subscribe( var, this, "valueChanged", true );
        }
    }
    
}


void QmlClient::processSslErrors(const QList<QSslError> &errors)
{
    if (!_socket)
    {
        //todo errmsg
        return;
    }
    foreach (const QSslError & error, errors)
    {
        qDebug() << "SSL Error: "<< error;
    }
     
    _socket->ignoreSslErrors();

    if (_socket->state() != QAbstractSocket::ConnectedState) //???
        processStateChanged(_socket->state());
}
#endif
