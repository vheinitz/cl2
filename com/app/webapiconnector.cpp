#include "webapiconnector.h"
#include <QCryptographicHash>


WebApiConnector::WebApiConnector() 
{
	_manager = new QNetworkAccessManager(this);
	connect(_manager, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(replyFinished(QNetworkReply*)));
}

WebApiConnector::~WebApiConnector()
{
}

void WebApiConnector::sendSmsNotification(QString text)
{	
	QString url = QString(_params["a"])
		.arg( QString(QCryptographicHash::hash( _params["k"].toLatin1() , QCryptographicHash::Md5 ).toHex()) )
		.arg( QString(_params["d"].toLatin1().toBase64()))
		.arg( QString( QString("Device %1: %2").arg(_params["instrumentId"]).arg(text).toLatin1().toBase64()));
	_manager->get( QNetworkRequest( QUrl( url ) ) );
}

void WebApiConnector::sendMailNotification(QString text)
{	

	QString url = QString(_params["ma"])
		.arg( QString(QCryptographicHash::hash( _params["k"].toLatin1() , QCryptographicHash::Md5 ).toHex()) )
		.arg( QString(_params["md"].toLatin1().toBase64()))
		.arg( QString( QString("Device %1: %2").arg(_params["instrumentId"]).arg(text).toLatin1().toBase64()));
	_manager->get( QNetworkRequest( QUrl( url ) ) );
}

void WebApiConnector::replyFinished(QNetworkReply* r)
{
	if(r)
		r->deleteLater();
}

void WebApiConnector::sendSmsNotificationFinished(const QString & msg)
{
	if ( _params["smsSendOnFinish"].toInt() )
	{
		if ( !msg.isNull() )
			sendSmsNotification( msg );
		else
			sendSmsNotification( _params["textOnFinished"] );
	}
}

void WebApiConnector::sendMailNotificationFinished(const QString & msg)
{
	if ( _params["mailSendOnFinish"].toInt() )
	{
		if ( !msg.isNull() )
			sendMailNotification( msg );
		else
			sendMailNotification( _params["textOnFinished"] );
	}
}

void WebApiConnector::sendSmsNotificationError(const QString & msg)
{
	if ( _params["smsSendOnError"].toInt() )
	{
		if ( !msg.isNull() )
			sendSmsNotification( msg );
		else
			sendSmsNotification( _params["textOnError"] );
	}
}

void WebApiConnector::sendMailNotificationError(const QString & msg)
{
	if ( _params["mailSendOnError"].toInt() )
	{
		if ( !msg.isNull() )
			sendMailNotification( msg );
		else
			sendMailNotification( _params["textOnError"] );
	}
}

void WebApiConnector::sendNotificationFinished()
{
	sendSmsNotificationFinished();
	sendMailNotificationFinished();
}

void WebApiConnector::sendNotificationError()
{
	sendSmsNotificationError();
	sendMailNotificationError();
}

void WebApiConnector::sendNotificationError(QString t )
{
	if ( _params["smsSendOnError"].toInt() )
	{
		sendSmsNotification(t);
	}

	if ( _params["mailSendOnError"].toInt() )
	{
		sendMailNotification(t);
	}	
}