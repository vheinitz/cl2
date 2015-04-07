#ifndef EIKJUSERVER_H
#define EIKJUSERVER_H

#include "common.h"

#include <QtNetwork>
#include <QtCore>
#include "ekhttprequest.h"


class EIKJU_EXPORT EiKjuServer : public QObject
{
    Q_OBJECT
    QTcpServer _server;
    QString _wwwroot;
    QMap<QString,QString> _mimeType;

private slots:
    void onConnnected();
    void onError ( QAbstractSocket::SocketError );
    void onDataRead();
public:
    EiKjuServer(QObject * parent);
	bool start( unsigned short );
	bool stop( );
	bool setRoot(QString);
    virtual ~ EiKjuServer(){}
};

#endif // EIKJUSERVER_H
